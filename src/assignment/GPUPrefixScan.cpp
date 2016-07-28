#include "GPUPrefixScan.h"

#include "ShaderSetup.h"

namespace FW {

	GLContext::Program * GPUPrefixScan::m_scan_prog = nullptr;
	GLContext::Program * GPUPrefixScan::m_block_add = nullptr;

	void GPUPrefixScan::loadProgram(GLContext * gl, const std::string & scanSource, const std::string & addSource) {

		loadShader(gl, scanSource, scanSource);
		m_scan_prog = gl->getProgram(scanSource.c_str());

		loadShader(gl, addSource, addSource);
		m_block_add = gl->getProgram(addSource.c_str());
	}

	void GPUPrefixScan::scanWorkEfficient(GLContext * gl, GLuint inBuffer, GLuint outBuffer, GLuint inBlockBuffer, GLuint outBlockBuffer, int size) {
		m_scan_prog->use();

		gl->setUniform(m_scan_prog->getUniformLoc("numValues"), size);
		gl->setUniform(m_scan_prog->getUniformLoc("storeLastValues"), true);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, inBlockBuffer);

		int localGroupSize = 1024;
		int numGroups = (size + localGroupSize - 1) / localGroupSize;
		glDispatchCompute(numGroups, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		int numBlocks = 1024;
		gl->setUniform(m_scan_prog->getUniformLoc("numValues"), numBlocks);
		gl->setUniform(m_scan_prog->getUniformLoc("storeLastValues"), false);

		numGroups = (numBlocks + localGroupSize - 1) / localGroupSize;

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inBlockBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outBlockBuffer);
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// add data
		m_block_add->use();
		gl->setUniform(m_block_add->getUniformLoc("numBlocks"), numBlocks);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, outBuffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, outBlockBuffer);
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	}

	void GPUPrefixScan::scan(GLContext * gl, GLuint buffer, GLuint blockBuffer, int size) {

		// scan each block
		m_scan_prog->use();

		gl->setUniform(m_scan_prog->getUniformLoc("numValues"), size);
		gl->setUniform(m_scan_prog->getUniformLoc("storeLastValues"), true);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, blockBuffer);

		int localGroupSize = 1024;
		int numGroups = (size + localGroupSize - 1) / localGroupSize;

		glDispatchCompute(numGroups, 1, 1);
		
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		
		// scan combined block
		 int numBlocks = 1024;
		gl->setUniform(m_scan_prog->getUniformLoc("numValues"), numBlocks);
		gl->setUniform(m_scan_prog->getUniformLoc("storeLastValues"), false);

		numGroups = (numBlocks + localGroupSize - 1) / localGroupSize;


		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, blockBuffer);
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		
		// add data
		m_block_add->use();
		gl->setUniform(m_block_add->getUniformLoc("numBlocks"), numBlocks);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer);
		glDispatchCompute(numGroups, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	}

	int GPUPrefixScan::getSum(GLContext * gl, GLuint blockBuffer) {
		int value;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockBuffer);
		value = *(int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 1024, sizeof(int), GL_MAP_READ_BIT);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		return value;
	}

}