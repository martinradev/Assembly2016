#pragma once

#include "Scene.h"

namespace FW {
	
	class BloomFilter : public Scene {

	public:

		BloomFilter(GLContext * ctx, Vec2i size, GLuint texture);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);

		void saveTexture();

		void setResultTexture(GLuint resultTex) {
			m_resultTexture = resultTex;
		}

		void setOriginalTexture(GLuint originalTexture) {
			m_originalTexture = originalTexture;
		}

		void setBlurredTexture(GLuint blurredTexture) {
			m_blurredTexture = blurredTexture;
		}

	private:

		void loadShaders(GLContext * ctx);
		void updateUniforms(Window & wnd, const CameraControls & camera);

		std::string m_csProgramName;
		std::string m_displayProgramName;

		Vec2i m_size;
		GLuint m_texture; // brightpass texture
		GLuint m_resultTexture;
		GLuint m_blurredTexture;;
		GLuint m_originalTexture;

		// knobs
		Vec4i m_intKnob;
		Vec4f m_knobs[10];

	};

};