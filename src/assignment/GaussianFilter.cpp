#include "GaussianFilter.h"
#include "TexturePool.h"
#include "Globals.h"
#include "ShaderSetup.h"
namespace FW {

	GaussianFilter::GaussianFilter(GLContext * gl, Vec2i size) :
		m_gaussianProgram("gaus_blur_1_pass"),
		m_size(size)
	{
		m_helpTexture = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, size.x, size.y, GL_RGBA, GL_FLOAT))->m_texture;

		const char passVertex[] = "shaders/raymarch/quad.vertex";
		const char blurFragment[] = "shaders/postprocess/fast_gaussian.glsl";
		loadShader(gl, passVertex, blurFragment, m_gaussianProgram);

		glGenFramebuffers(1, &m_fbo);
		
		
	}
	
	void GaussianFilter::process(GLContext * gl, GLuint inputTexture, GLuint outputTexture, int numPasses) {

		// change how texture looksup behave
		glBindTexture(GL_TEXTURE_2D, inputTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glBindTexture(GL_TEXTURE_2D, m_helpTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glBindTexture(GL_TEXTURE_2D, outputTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		const static F32 posAttrib[] =
		{
			-1, -1, 0, 1,
			1, -1, 0, 1,
			-1, 1, 0, 1,
			1, 1, 0, 1
		};


		const static F32 texAttrib[] =
		{
			0, 1,
			1, 1,
			0, 0,
			1, 0
		};

		const static F32 texAttribINV[] =
		{
			0, 0,
			1, 0,
			0, 1,
			1, 1
		};

		GLContext::Program * gausFilter = gl->getProgram(m_gaussianProgram.c_str());

		gausFilter->use();

		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
		glViewport(0, 0, m_size.x, m_size.y);
		GLuint tex1 = m_helpTexture;
		GLuint tex2 = outputTexture;

		gl->setAttrib(gausFilter->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		gl->setAttrib(gausFilter->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttribINV);
		glActiveTexture(GL_TEXTURE0);
		gl->setUniform(gausFilter->getUniformLoc("inImage"), 0);

		for (int i = 0; i < numPasses; ++i) {
			
			// pass x
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex1, 0);

			
			glBindTexture(GL_TEXTURE_2D, inputTexture);

			gl->setUniform(gausFilter->getUniformLoc("dir"), Vec2f(1.0, 0));
			gl->setUniform(gausFilter->getUniformLoc("step"), 1.0f / m_size.x);
			
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			
			// data is stored in tex1, we have to blur pass in y direction using tex1 as input
			// pass y
			// store results in tex2

			inputTexture = tex1;
			glBindTexture(GL_TEXTURE_2D, inputTexture);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex2, 0);
			
			gl->setUniform(gausFilter->getUniformLoc("dir"), Vec2f(0, 1.0));
			gl->setUniform(gausFilter->getUniformLoc("step"), 1.0f / m_size.y);

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			inputTexture = tex2;
		}
		

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}


};