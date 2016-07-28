#pragma once

#include "Scene.h"

namespace FW {

	class ProceduralTextureScene : public Scene {

	public:

		ProceduralTextureScene(const std::string & sceneShader, GLContext * ctx, Vec2i size, GLuint texture = 0);

		void render(Window & wnd, const CameraControls & camera);
		void renderToTexture(Window & wnd, const CameraControls & camera);

		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);

		void saveTexture();

		GLuint getTextureHandle() const {
			return m_texture;
		}

	private:

		void loadShaders(GLContext * ctx);
		void updateUniforms(Window & wnd, const CameraControls & camera);

		std::string m_sceneShader;
		Vec2i m_size;
		GLuint m_texture;
		GLuint m_frameBuffer;
		GLuint m_renderBuffer;

		// knobs
		Vec4f m_knobs[10];

	};

};