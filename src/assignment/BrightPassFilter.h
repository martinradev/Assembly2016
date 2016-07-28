#pragma once

#include "Scene.h"

namespace FW {

	class BrightPassFilter : public Scene {

	public:

		BrightPassFilter(GLContext * ctx, Vec2i size);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);

		void setResultTexture(GLuint tex) {
			m_resultTexture = tex;
		}

		void setTexture(GLuint tex) {
			m_texture = tex;
		}
		void saveTexture();
	private:

		void loadShaders(GLContext * ctx);
		void updateUniforms(Window & wnd, const CameraControls & camera);

		std::string m_programName;
		std::string m_displayProgramName;

		Vec2i m_size;
		GLuint m_texture;
		GLuint m_resultTexture;
		GLuint m_frameBuffer;
		GLuint m_renderBuffer;

		// knobs
		Vec4f m_knobs[10];

	};

};