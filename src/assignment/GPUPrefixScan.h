#pragma once

#include "gpu/GLContext.hpp"
#include <string>

namespace FW {

	class GPUPrefixScan {

	public:

		static void loadProgram(GLContext * gl, const std::string & scanSource, const std::string & addSource);
		static void scan(GLContext * gl, GLuint buffer, GLuint blockBuffer, int size);

		static void scanWorkEfficient(GLContext * gl, GLuint inBuffer, GLuint outBuffer, GLuint inBlockBuffer, GLuint outBlockBuffer, int size);

		static int getSum(GLContext * gl, GLuint blockBuffer);
	private:

		static GLContext::Program * m_scan_prog;
		static GLContext::Program * m_block_add;

	};

};