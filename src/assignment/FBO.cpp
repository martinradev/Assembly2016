#include "FBO.h"

namespace FW {

	FBO::FBO(GLuint depthTexture) {

		glGenFramebuffers(1, &mFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FBO::bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
	}

	void FBO::unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FBO::attachTexture(int attachment, GLuint texture) {
		bind();

		GLenum cColorAttachment = GL_COLOR_ATTACHMENT0 + attachment;
		if (std::find(mAttachments.begin(), mAttachments.end(), cColorAttachment) != mAttachments.end())
		{
			::printf("GL_COLOR_ATTACHMENT%d is already in use\n", attachment);
			exit(1);
		}
		mAttachments.push_back(cColorAttachment);
		mColorTextures.push_back(texture);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, cColorAttachment, GL_TEXTURE_2D, texture, 0);

		glDrawBuffers(mAttachments.size(), mAttachments.data());

		unbind();
	}

	GLuint FBO::getTexture(int loc) const {
		return mColorTextures[loc];
	}

	void FBO::setTexture(int loc, GLuint texture) {
		mColorTextures[loc] = texture;

		bind();
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, mAttachments[loc], GL_TEXTURE_2D, texture, 0);

		unbind();
	}

};