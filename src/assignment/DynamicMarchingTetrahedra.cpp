#include "DynamicMarchingTetrahedra.h"
#include "ShaderSetup.h"
#include "GPUPrefixScan.h"
#include <fstream>
namespace FW {

	DynamicMarchingTetrahedra::DynamicMarchingTetrahedra(
		GLContext * gl,
		const Vec4f & cubeInfo,
		const std::string & sceneShader) :
		m_cubeInfo(cubeInfo),
		m_tetraCSProgram("shaders/tetrahedra_march/march.glsl"),
		m_sceneShader(sceneShader),
		m_renderProgram("dmt_render"),
		m_numTriangles(0)
	{
		loadShaders(gl);
		
		std::vector<int> zeros(1024*1024);

		glGenBuffers(DMT_Buffer_Types::BUFFERS_COUNT, m_buffers);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffers[DMT_Buffer_Types::INDEX_BUFFER]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1024 * 1024, zeros.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffers[DMT_Buffer_Types::BLOCK_BUFFER]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1025, zeros.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffers[DMT_Buffer_Types::MESH_BUFFER]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DMT_TriangleUnit) * 64 * 64 * 64 * 6, NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GPUPrefixScan::loadProgram(gl, "shaders/compute/prefix_scan.glsl", "shaders/compute/prefix_block_add.glsl");
	}

	void DynamicMarchingTetrahedra::update(GLContext * gl) {

		GLContext::Program * prog = gl->getProgram(m_sceneShader.c_str());
		prog->use();

		Vec3i numBlocks(64);
		Vec3i threadBlockSize(4);
		Vec3i gridSize = (numBlocks + threadBlockSize - 1) / threadBlockSize;

		gl->setUniform(prog->getUniformLoc("cubeInfo"), m_cubeInfo);
		gl->setUniform(prog->getUniformLoc("isPrefixSumPass"), true);

		gl->setUniform(prog->getUniformLoc("numCubes"), numBlocks);

		gl->setUniform(prog->getUniformLoc("sync1"), m_disp1);
		gl->setUniform(prog->getUniformLoc("sync2"), m_disp2);
		gl->setUniform(prog->getUniformLoc("sync3"), m_disp3);
		gl->setUniform(prog->getUniformLoc("sync4"), m_disp4);
		gl->setUniform(prog->getUniformLoc("maxTetrahedras"), 6);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_buffers[DMT_Buffer_Types::INDEX_BUFFER]);
		glDispatchCompute(gridSize.x, gridSize.y, gridSize.z);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		GPUPrefixScan::scan(gl, m_buffers[DMT_Buffer_Types::INDEX_BUFFER], m_buffers[DMT_Buffer_Types::BLOCK_BUFFER], 100*100*100);

		prog->use();
		gl->setUniform(prog->getUniformLoc("isPrefixSumPass"), false);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffers[DMT_Buffer_Types::MESH_BUFFER]);
		
		glDispatchCompute(gridSize.x, gridSize.y, gridSize.z);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		m_numTriangles = GPUPrefixScan::getSum(gl, m_buffers[DMT_Buffer_Types::BLOCK_BUFFER]);
	}

	void DynamicMarchingTetrahedra::renderMesh(GLContext * gl, GLContext::Program * prog) {
		prog->use();

		glVertexAttrib3f(prog->getAttribLoc("normalAttrib"), 0.0f, 0.0f, 0.0f);
		//glVertexAttrib4f(prog->getAttribLoc("vcolorAttrib"), 1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib2f(prog->getAttribLoc("texCoordAttrib"), 0.0f, 0.0f);
		gl->setUniform(prog->getUniformLoc("diffuseUniform"), Vec4f(0.6, 0.7, 0.9));
		gl->setUniform(prog->getUniformLoc("specularUniform"), Vec3f(0.5f, 0.3, 0.6));
		gl->setUniform(prog->getUniformLoc("glossiness"), 10.0f);

		gl->setUniform(prog->getUniformLoc("useDiffuseTexture"), false);

		gl->setUniform(prog->getUniformLoc("useNormalMap"), false);
		

		gl->setUniform(prog->getUniformLoc("useSpecularMap"), false);
		gl->setUniform(prog->getUniformLoc("ssaoMask"), 0.0f);
		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[DMT_Buffer_Types::MESH_BUFFER]);

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), 0);

		glEnableVertexAttribArray(1); // position
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), (GLvoid*)((char*)NULL + sizeof(float) * 8));
		//::printf("%d\n", m_numTriangles);
		if (0<m_numTriangles)glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void DynamicMarchingTetrahedra::renderMeshCheap(GLContext * gl, GLContext::Program * prog) {
		prog->use();

		glVertexAttrib3f(prog->getAttribLoc("normalAttrib"), 0.0f, 0.0f, 0.0f);
		//glVertexAttrib4f(prog->getAttribLoc("vcolorAttrib"), 1.0f, 1.0f, 1.0f, 1.0f);
		glVertexAttrib2f(prog->getAttribLoc("texCoordAttrib"), 0.0f, 0.0f);


		glBindBuffer(GL_ARRAY_BUFFER, m_buffers[DMT_Buffer_Types::MESH_BUFFER]);

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), 0);

		glEnableVertexAttribArray(1); // position
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), (GLvoid*)((char*)NULL + sizeof(float) * 8));

		if (0<m_numTriangles)glDrawArrays(GL_TRIANGLES, 0, m_numTriangles);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void DynamicMarchingTetrahedra::loadShaders(GLContext * gl) {

		const char declShader[] = "shaders/raymarch/decl.glsl"; // header stuff, declerations
		const char hgSdfShader[] = "shaders/raymarch/hg_sdf.glsl"; // mercury's sdf library
		const char noiseShader[] = "shaders/raymarch/noise.glsl"; // noise utilities
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms
		//printf("start\n");
		loadShader(gl,
		{
			declShader,
			hgSdfShader,
			noiseShader,
			m_sceneShader,
			m_tetraCSProgram
		}, m_sceneShader, true);

		//printf("done\n");

		const char vertexShader[] = "shaders/tetrahedra_march/dynamic_vertex_transform.glsl";
		const char fragmentShader[] = "shaders/shading_comparison/forward_fragment.glsl";


		loadShader(gl,

		{ declShader, hgSdfShader,
			noiseShader, vertexShader },
		{ declShader, hgSdfShader,
			noiseShader, fragmentShader }, m_renderProgram);

		/*std::vector<char> output(1000000);
		GLContext::Program * prog = gl->getProgram(m_sceneShader.c_str());
		GLenum format;
		GLint length;
		glGetProgramBinary(prog->getHandle(), sizeof(char) * 1000000, &length, &format, output.data());
		std::ofstream out("out.txt");
		out.write(output.data(), length);*/

		gl->checkErrors();

	}



};