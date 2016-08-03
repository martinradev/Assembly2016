#include "TexturePool.h"

#include <cstdio>

namespace FW {

	TexturePool::TexturePool() :
		m_poolSize(0)
	{
		initContainer();
	}

	void TexturePool::initContainer() {

		for (unsigned int i = 0; i < TexturePool::POOL_MAX_SIZE; ++i) {
			m_container[i] = TextureContainer(); // no in use, not initialized
		}

	}

	void TexturePool::release(const TextureContainer * texture) {

		ptrdiff_t offset = (texture - m_container);
		int pos = offset / sizeof(TextureContainer);

		if (pos >= POOL_MAX_SIZE) {
			::printf("Cannot release the address\n");
			return;
		}

		m_container[pos].m_inUse = false;

	}

	TextureContainer * TexturePool::allocateTexture(const TextureDescriptor & descriptor) {

		if (m_poolSize >= POOL_MAX_SIZE) {
			::printf("Could not allocate texture. Max texture count reached.");
			exit(1);
		}

		GLint oldTex = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);

		GLuint tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);

		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		/*glTexImage2D(GL_TEXTURE_2D, 0, descriptor.m_internalFormat,
			descriptor.m_width, descriptor.m_height,
			0, descriptor.m_format, descriptor.m_type, NULL);*/

		glTexStorage2D(GL_TEXTURE_2D, 1, descriptor.m_internalFormat, descriptor.m_width, descriptor.m_height);
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, descriptor.m_width, descriptor.m_height, descriptor.m_format, descriptor.m_type, NULL);

		m_container[m_poolSize] = TextureContainer(descriptor, tex);

		glBindTexture(GL_TEXTURE_2D, oldTex);

		TextureContainer * result = &m_container[m_poolSize];
		++m_poolSize;
		return result;
	}

	const TextureContainer * TexturePool::request(const TextureDescriptor & descriptor) {

		for (int i = 0; i < m_poolSize; ++i) {
			if (m_container[i].m_descriptor == descriptor && m_container[i].m_inUse == false) {
				m_container[i].m_inUse = true;
				return &m_container[i];
			}
		}

		TextureContainer * obj = allocateTexture(descriptor);
		obj->m_inUse = true;
		return obj;
	}

};