#pragma once

#include "Scene.h"
#include "GBuffer.h"
#include "gpu/GLContext.hpp"

#include <memory>

namespace FW {

	enum NBodyTest_Textures : int {

		NBODY_POS_TEXTURE = 0,
		NBODY_POS_TEXTURE1,
		NBODY_TEX_COUNT

	};

	class NBodyTest : public Scene {

	public:

		NBodyTest(GLContext * ctx,
			unsigned width, unsigned height);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);

		NBodyTest::~NBodyTest() {

		}

	private:

		void loadShaders(GLContext * ctx);
		void createBuffers();

		void moveParticles(GLContext * gl);

		std::unique_ptr<GBuffer> m_gBuffer;
		std::unique_ptr<GBuffer> m_gBufferAcc;

		std::string m_computeProgramName;
		std::string m_renderProgramName;
		std::string m_motionBlurProgramName;
		std::string m_displayProgramName;

		GLuint m_textures[NBODY_TEX_COUNT];
		GLuint m_accumulationTexture;

		GLuint m_particleVAO;
		GLuint m_particleVBO;

		GLuint m_quadVAO;
		GLuint m_quadVBO;

		GLuint m_cubeVAO;
		GLuint m_cubeIBO;
		GLuint m_cubeVBO;

		// knobs
		Vec4f m_knobs[10];

		unsigned m_imageX; // width
		unsigned m_imageY; // height

	};

};
