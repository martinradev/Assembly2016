#pragma once


#include "base/Main.hpp"
#include "gpu/GLContext.hpp"
#include "GaussianFilter.h"
#include "FBO.h"
#include <memory>

namespace FW {

	class NewBloomFilter {

	public:

		NewBloomFilter(GLContext * gl, GLuint resultTexture, int width, int height);

		void bloom(GLContext * gl, GLuint inputTexture, int level, float luminance, float threshold, float offset);

	private:

		GLContext::Program * mCombineProgram;
		GLContext::Program * mBrightPassProgram;
		GLuint mResultTexture;
		int mWidth;
		int mHeight;

		GLuint mQuadVAO;
		GLuint mQuadVBO;

		GLuint mHelpTextures[5];
		std::unique_ptr<GaussianFilter> mGaussianFilter;
		std::unique_ptr<FBO> mFbo;
	};
}