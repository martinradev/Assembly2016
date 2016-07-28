#include "Mesher.h"
#include "ShaderSetup.h"
#include "GPUPrefixScan.h"
#include <fstream>
namespace FW {

	Mesher::Mesher(
		GLContext * gl,
		const Vec4f & cubeInfo,
		const std::string & sceneShader) :
		m_tetraCSProgram("shaders/tetrahedra_march/march.glsl"),
		m_sceneShader(sceneShader),
		m_cubeInfo(cubeInfo),
		m_numTriangles(0)
	{
		loadShaders(gl);

		std::vector<int> zeros(1024 * 1024);

		glGenBuffers(DMT_Buffer_Types::BUFFERS_COUNT, m_buffers);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffers[DMT_Buffer_Types::INDEX_BUFFER]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1024 * 1024, zeros.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffers[DMT_Buffer_Types::BLOCK_BUFFER]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 1025, zeros.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_buffers[DMT_Buffer_Types::MESH_BUFFER]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DMT_TriangleUnit) * 64 * 64 * 64 * 18, NULL, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		GPUPrefixScan::loadProgram(gl, "shaders/compute/prefix_scan.glsl", "shaders/compute/prefix_block_add.glsl");
	}

	void Mesher::update(GLContext * gl) {

		GLContext::Program * prog = gl->getProgram(m_sceneShader.c_str());
		prog->use();

		Vec3i numBlocks(64);
		Vec3i threadBlockSize(8);
		Vec3i gridSize = (numBlocks + threadBlockSize - 1) / threadBlockSize;

		gl->setUniform(prog->getUniformLoc("cubeInfo"), m_cubeInfo);
		gl->setUniform(prog->getUniformLoc("isPrefixSumPass"), true);

		gl->setUniform(prog->getUniformLoc("numCubes"), numBlocks);

		gl->setUniform(prog->getUniformLoc("maxTetrahedras"), 6);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_buffers[DMT_Buffer_Types::INDEX_BUFFER]);
		glDispatchCompute(gridSize.x, gridSize.y, gridSize.z);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		GPUPrefixScan::scan(gl, m_buffers[DMT_Buffer_Types::INDEX_BUFFER], m_buffers[DMT_Buffer_Types::BLOCK_BUFFER], 64 * 64 * 64);

		prog->use();
		gl->setUniform(prog->getUniformLoc("isPrefixSumPass"), false);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_buffers[DMT_Buffer_Types::MESH_BUFFER]);

		glDispatchCompute(gridSize.x, gridSize.y, gridSize.z);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		m_numTriangles = GPUPrefixScan::getSum(gl, m_buffers[DMT_Buffer_Types::BLOCK_BUFFER]);
	}

	void Mesher::loadShaders(GLContext * gl) {

		const char declShader[] = "shaders/raymarch/decl.glsl"; // header stuff, declerations
		const char hgSdfShader[] = "shaders/raymarch/hg_sdf.glsl"; // mercury's sdf library
		const char noiseShader[] = "shaders/raymarch/noise.glsl"; // noise utilities

		mTetraProgram = loadShader(gl,
		{
			declShader,
			hgSdfShader,
			noiseShader,
			m_sceneShader,
			m_tetraCSProgram
		}, m_sceneShader, true);

		gl->checkErrors();

	}



};