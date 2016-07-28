#pragma once

#include "gpu/GLContext.hpp"

namespace FW {

	struct TextureDescriptor {

		GLint m_internalFormat;
		GLsizei m_width;
		GLsizei m_height;
		GLenum m_format;
		GLenum m_type;

		TextureDescriptor(GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type) :
			m_internalFormat(internalFormat),
			m_width(width),
			m_height(height),
			m_format(format),
			m_type(type)
		{

		}

		TextureDescriptor() :
		m_internalFormat(0),
		m_width(0),
		m_height(0),
		m_format(0),
		m_type(0)
		{
			
		}

		bool operator ==(const TextureDescriptor & other) const {
			bool res = (m_internalFormat == other.m_internalFormat 
				&& m_format == other.m_format 
				&& m_height == other.m_height
				&& m_width == other.m_width
				&& m_type == other.m_type);
			return res;
		}

		// border always = 0
		// target always = GL_TEXTURE_2D
		// type always = GL_FLOAT

	};

	struct TextureContainer {

		TextureDescriptor m_descriptor;
		GLuint m_texture;
		bool m_inUse;

		TextureContainer(const TextureDescriptor & descriptor, GLuint texture) :
			m_descriptor(descriptor),
			m_texture(texture),
			m_inUse(false) {

		}

		TextureContainer() :
			m_texture(0),
			m_inUse(false)
		{

		}

	};

	class TexturePool {

	public:

		TexturePool();

		// DELETE THOSE METHODS. ONLY ONE INSTANCE OF TEXTURE POOL WILL BE USED
		TexturePool(const TexturePool & oth) = delete;
		TexturePool & operator=(const TexturePool & oth) = delete;
		TexturePool(TexturePool && oth) = delete;
		TexturePool & operator=(TexturePool && oth) = delete;

		

		/**
			request a texture with the given description
			if such is not available, it will be allocated.
			There's some control on maximum number of texture in the program - 64
			Upon return of the texturecontainer, it is marked as in use.
			It cannot be requested again.
		*/
		const TextureContainer * request(const TextureDescriptor & descriptor);

		/**
			Releases the texture so that it can be used by other parts of the program
		*/
		void release(const TextureContainer * texture);

	private:
		
		TextureContainer * allocateTexture(const TextureDescriptor & descriptor);
		void initContainer();

		static const unsigned int POOL_MAX_SIZE = 128;
		unsigned int m_poolSize;
		TextureContainer m_container[POOL_MAX_SIZE];

	};

};