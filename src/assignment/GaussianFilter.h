#pragma once

#include "gpu/GLContext.hpp"
#include "base/Math.hpp"
#include <string>

namespace FW {

	class GaussianFilter {

	public:

		GaussianFilter() {};
		GaussianFilter(GLContext * gl, Vec2i windowSize);

		void process(GLContext * gl, GLuint inputTexture, GLuint outputTexture, int numPasses);

	private:

		std::string m_gaussianProgram;
		Vec2i m_size;
		GLuint m_fbo;
		GLuint m_helpTexture;

	};
	
};