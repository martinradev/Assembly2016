#include "SDFGeometryFactory.h"
#include "GPUPrefixScan.h"
#include "ShaderSetup.h"
#include <vector>

namespace FW {

	SDFGeometryFactory::SDFGeometryFactory(GLContext * gl) {
		std::vector<int> zeros(1024 * 1024);

		glGenBuffers(SDFGeometryFactoryBuffers::SDF_GEOM_BUFFERS_COUNT, mBuffers);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBuffers[SDFGeometryFactoryBuffers::SDF_GEOM_INDEX_BUFFER]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1024 * 1024, zeros.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBuffers[SDFGeometryFactoryBuffers::SDF_GEOM_BLOCK_BUFFER]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1025, zeros.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GPUPrefixScan::loadProgram(gl, "shaders/compute/prefix_scan.glsl", "shaders/compute/prefix_block_add.glsl");

		loadCommonShaders(gl);
	}

	void SDFGeometryFactory::resetBuffers() {
		std::vector<int> data(1024*1024, 0);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBuffers[SDFGeometryFactoryBuffers::SDF_GEOM_INDEX_BUFFER]);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * 1024 * 1024, data.data());
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void SDFGeometryFactory::produceGeometry(GLContext * gl, const SDFGeometryDescription & description, SDFGeometryOutput * geometryResult) {

		resetBuffers();

		Vec3i numBlocks(description.mNumSteps);
		Vec3i threadBlockSize(4);
		Vec3i gridSize = (numBlocks + threadBlockSize - 1) / threadBlockSize;

		GLuint prog = getProgram(description.mSDFSource);

		glUseProgram(prog);

		gl->setUniform(glGetUniformLocation(prog, "cubeInfo"), Vec4f(description.mStart, description.mCubeStep));
		gl->setUniform(glGetUniformLocation(prog, "isPrefixSumPass"), true);
		gl->setUniform(glGetUniformLocation(prog, "numCubes"), numBlocks);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mBuffers[SDFGeometryFactoryBuffers::SDF_GEOM_INDEX_BUFFER]);

		glDispatchCompute(gridSize.x, gridSize.y, gridSize.z);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		GPUPrefixScan::scan(gl, mBuffers[SDFGeometryFactoryBuffers::SDF_GEOM_INDEX_BUFFER], mBuffers[SDFGeometryFactoryBuffers::SDF_GEOM_BLOCK_BUFFER], 100 * 100 * 100);

		geometryResult->numTriangles = GPUPrefixScan::getSum(gl, mBuffers[SDFGeometryFactoryBuffers::SDF_GEOM_BLOCK_BUFFER]);

		if (geometryResult->numTriangles <= 0) {
			::printf("error\n");
			exit(1);
			return;
		}

		// reserve memory

		glGenVertexArrays(1, &geometryResult->mVao);
		glBindVertexArray(geometryResult->mVao);

		GLsizei sizeOfTriangle = sizeof(Vec4f) * 2;

		glGenBuffers(1, &geometryResult->mVbo);
		glBindBuffer(GL_ARRAY_BUFFER, geometryResult->mVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeOfTriangle * geometryResult->numTriangles, NULL, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeOfTriangle, 0);

		glEnableVertexAttribArray(1); // normal
		char * offset = (char*)NULL + sizeof(Vec4f);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeOfTriangle, offset);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glUseProgram(prog);

		gl->setUniform(glGetUniformLocation(prog, "isPrefixSumPass"), false);

		// output mesh

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, geometryResult->mVbo);

		glDispatchCompute(gridSize.x, gridSize.y, gridSize.z);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glUseProgram(0);

		geometryResult->toWorld = description.toWorld;
		geometryResult->toWorldNormal = description.toWorldNormal;
	}

	void SDFGeometryFactory::loadCommonShaders(GLContext * gl) {

		const char declarations[] = "shaders/geometry_generation/decl.glsl";
		const char noiseFunctions[] = "shaders/geometry_generation/noise.glsl";
		const char hgSdfFunctions[] = "shaders/geometry_generation/hg_sdf.glsl";
		const char march[] = "shaders/geometry_generation/march.glsl";
		try {
			mDeclarationShader = GLContext::Program::createGLShader(GL_COMPUTE_SHADER, "GL_COMPUTE_SHADER", readShaderSource(declarations), true);
			mNoiseShader = GLContext::Program::createGLShader(GL_COMPUTE_SHADER, "GL_COMPUTE_SHADER", readShaderSource(noiseFunctions), true);
			mHGSDFShader = GLContext::Program::createGLShader(GL_COMPUTE_SHADER, "GL_COMPUTE_SHADER", readShaderSource(hgSdfFunctions), true);
			mTetrahedraMarchComputeShader = GLContext::Program::createGLShader(GL_COMPUTE_SHADER, "GL_COMPUTE_SHADER", readShaderSource(march), true);
		}
		catch (GLContext::Program::ShaderCompilationException& e)
		{
			::printf("Could not compile shader %s:\n%s\n", "", e.msg_.getPtr());

			fail("Fatal, need a shader to draw!");
		}
	}

	GLuint SDFGeometryFactory::getProgram(const std::string & sdfShader) {
		
		GLuint prog = glCreateProgram();

		GLuint cShader;

		try {
			cShader = GLContext::Program::createGLShader(GL_COMPUTE_SHADER, "GL_COMPUTE_SHADER", readShaderSource(sdfShader), true);
		} 
		catch (GLContext::Program::ShaderCompilationException& e)
		{
			::printf("Could not compile shader %s:\n%s\n", sdfShader.c_str(), e.msg_.getPtr());

			fail("Fatal, need a shader to draw!");
		}

		glAttachShader(prog, mDeclarationShader);
		glAttachShader(prog, mNoiseShader);
		glAttachShader(prog, mHGSDFShader);
		glAttachShader(prog, mTetrahedraMarchComputeShader);
		glAttachShader(prog, cShader);

		try {
			GLContext::Program::linkGLProgram(prog, true);
		}
		catch (GLContext::Program::ShaderCompilationException& e)
		{
			::printf("Could not link shader %s:\n%s\n", sdfShader.c_str(), e.msg_.getPtr());

			fail("Fatal, need a shader to draw!");
		}
		

		glDeleteShader(cShader);
		::printf("linked\n");

		return prog;
	}

};