#pragma once

#include "Scene.h"
#include "SyncVars.h"
namespace FW {

	class ToneMapper : public Scene {

	public:

		ToneMapper(GLContext * ctx, Vec2i size);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);

		void setTexture(GLuint tex) {
			m_texture = tex;
		}
		void setBloomTexture(GLuint tex) {
			mBloomTexture = tex;
		}
		void setExposure(float exposure) { mExposure = exposure; }
		void saveTexture();
	private:

		void loadShaders(GLContext * ctx);
		void updateUniforms(Window & wnd, const CameraControls & camera);

		std::string m_programName;

		Vec2i m_size;
		GLuint m_texture;
		//GLuint m_paletteTexture;
		GLuint mBloomTexture;

		// knobs
		Vec4f m_knobs[10];
		float mExposure;
	};

};