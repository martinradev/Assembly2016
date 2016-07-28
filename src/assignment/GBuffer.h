#pragma once

#include "gpu/GLContext.hpp"

#include <cstdio>

namespace FW {

	enum GBUFFER_TYPES : int {

		GBUFFER_DEPTH = 0,
		GBUFFER_DIFFUSE,
		GBUFFER_NORMAL,
		GBUFFER_POSITION,
		GBUFFER_MAX

	};

	class GBuffer {

	public:

		GBuffer(const unsigned width, const unsigned height);

		GLuint getFBO() const {
			return m_fbo;
		}

		GLuint getTexture(GBUFFER_TYPES type) const {
			if (type < 0 || type >= GBUFFER_MAX) {
				::printf("Cannot get texture %d\n", type);
				exit(1);
			}
			return m_textures[type];
		}

		void bindAll();

		GLuint getRealDepthMap() const {
			return m_realDepthMap;
		}

		void setTexture(GBUFFER_TYPES type, GLuint tex) {
			if (type < 0 || type >= GBUFFER_MAX) {
				::printf("Cannot get texture %d\n", type);
				exit(1);
			}
			m_textures[type] = tex;
		}

	private:

		unsigned m_width;
		unsigned m_height;

		GLuint m_fbo;
		GLuint m_textures[4];
		GLuint m_realDepthMap;

		void init();

	};

};