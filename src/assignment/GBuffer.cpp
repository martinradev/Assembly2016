#include "GBuffer.h"
#include "Globals.h"
namespace FW {

	GBuffer::GBuffer(const unsigned width, const unsigned height) : 
		m_width(width),
		m_height(height),
		m_fbo(0)
	{
		for (int i = 0; i < GBUFFER_MAX; ++i) m_textures[i] = 0;
		init();
	}

	void GBuffer::init() {

		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);



		m_textures[GBUFFER_DEPTH] =
			TEXTURE_POOL->request(TextureDescriptor(GL_R32F, m_width, m_height, GL_RED, GL_FLOAT))->m_texture;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_textures[GBUFFER_DEPTH], 0);

		m_realDepthMap = TEXTURE_POOL->request(TextureDescriptor(GL_DEPTH_COMPONENT32F, m_width, m_height, GL_DEPTH_COMPONENT, GL_FLOAT))->m_texture;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_realDepthMap, 0);

		m_textures[GBUFFER_DIFFUSE] =
			TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_width, m_height, GL_RGBA, GL_FLOAT))->m_texture;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[GBUFFER_DIFFUSE], 0);

		m_textures[GBUFFER_NORMAL] =
			TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_width, m_height, GL_RGBA, GL_FLOAT))->m_texture;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textures[GBUFFER_NORMAL], 0);

		m_textures[GBUFFER_POSITION] =
			TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_width, m_height, GL_RGBA, GL_FLOAT))->m_texture;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_textures[GBUFFER_POSITION], 0);
		
		GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	
		glDrawBuffers(4, DrawBuffers);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	}

	void GBuffer::bindAll() {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_textures[GBUFFER_DEPTH], 0);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textures[GBUFFER_DIFFUSE], 0);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_textures[GBUFFER_NORMAL], 0);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_textures[GBUFFER_POSITION], 0);

		GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

		glDrawBuffers(4, DrawBuffers);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

};