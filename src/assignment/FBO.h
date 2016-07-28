#pragma once

#include "gpu/GLContext.hpp"

#include <vector>

namespace FW {

	class FBO {

	public:

		FBO(GLuint depthTexture);

		void bind();
		void unbind();
		void attachTexture(int attachment, GLuint texture);
		GLuint getTexture(int loc) const;
		void setTexture(int loc, GLuint texture);

	private:

		std::vector<GLenum> mAttachments;
		std::vector<GLuint> mColorTextures;

		GLuint mFbo;

	};

};