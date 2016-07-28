#include "NewBloomFilter.h"
#include "Globals.h"
#include "ShaderSetup.h"

namespace FW {

	NewBloomFilter::NewBloomFilter(GLContext * gl, GLuint resultTexture, int width, int height):
		mResultTexture(resultTexture),
		mWidth(width),
		mHeight(height)
	{


		for (size_t i = 0; i < 5; ++i)
		{
			mHelpTextures[i] = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;
		}

		GLuint fboDepthTex = TEXTURE_POOL->request(TextureDescriptor(GL_DEPTH_COMPONENT32F, width, height, GL_DEPTH_COMPONENT, GL_FLOAT))->m_texture;

		mFbo.reset(new FBO(fboDepthTex));
		mFbo->attachTexture(0, resultTexture);

		const char vertShader[] = "shaders/common/display_vertex.glsl";


		mBrightPassProgram = loadShader(gl, vertShader, "shaders/postprocess/brightpass.glsl", "new_bloom_brightpass");

		mCombineProgram =  loadShader(gl, vertShader, "shaders/postprocess/bloom_combine.glsl", "new_bloom_combine");

		mGaussianFilter.reset(new GaussianFilter(gl, Vec2i(width, height)));

		const static F32 posAttrib[] =
		{
			-1, -1, 0, 1,
			1, -1, 0, 1,
			-1, 1, 0, 1,
			1, 1, 0, 1
		};

		glGenVertexArrays(1, &mQuadVAO);
		glGenBuffers(1, &mQuadVBO);
		glBindVertexArray(mQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, posAttrib, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void NewBloomFilter::bloom(GLContext * gl, GLuint inputTexture, int level, float luminance, float threshold, float offset) {

		if (level > 5) level = 5;

		glDisable(GL_DEPTH_TEST);

		mFbo->bind();

		glClear(GL_COLOR_BUFFER_BIT);

		mBrightPassProgram->use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, inputTexture);

		gl->setUniform(mBrightPassProgram->getUniformLoc("inImage"), 0);
		gl->setUniform(mBrightPassProgram->getUniformLoc("LUMINANCE"), luminance);
		gl->setUniform(mBrightPassProgram->getUniformLoc("THRESHOLD"), threshold);
		gl->setUniform(mBrightPassProgram->getUniformLoc("OFFSET"), offset);

		glBindVertexArray(mQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		mFbo->unbind();


		for (int i = 0; i < level; ++i) {

			GLuint inTexture = i == 0 ? mFbo->getTexture(0) : mHelpTextures[i - 1];

			mGaussianFilter->process(gl, inTexture, mHelpTextures[i], 1);
		}
		
		mFbo->bind();
		glClear(GL_COLOR_BUFFER_BIT);

		mCombineProgram->use();

		gl->setUniform(mCombineProgram->getUniformLoc("numLevels"), level);
		for (int i = 0; i < level; ++i) {
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D, mHelpTextures[i]);
			std::string uni = "blurLevels[" + std::to_string(i) + "]";
			gl->setUniform(mCombineProgram->getUniformLoc(uni.c_str()), 2);

		}

		glBindVertexArray(mQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		mFbo->unbind();

		glEnable(GL_DEPTH_TEST);

	}

};