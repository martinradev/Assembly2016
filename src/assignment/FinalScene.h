#pragma once

#include "gpu/GLContext.hpp"
#include "TessellationTestScene.h"
#include "ParticleLogoSDF.h"
#include "TunnelScene.h"
#include "Scene.h"
#include "FBO.h"
namespace FW {

	class FinalScene : public Scene {

	public:

		FinalScene(GLContext * ctx, unsigned width, unsigned height, FBO * lastFBO, CameraControls * camPtr, TunnelScene * tunnelScene, TessellationTestScene * tessScene, ParticleLogoSDF * logoScene);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);
		
	private:

		void loadShaders(GLContext * ctx);

		void setupBuffers();

		int mWidth;
		int mHeight;

		FBO * mLastFBO;
		FBO * godrayFBO;

		GLuint mQuadVAO;
		GLuint mQuadVBO;
	
		// knobs
		Vec4f m_knobs[10];

		CameraControls * mCamPtr;

		typedef GLContext::Program Program;

		Program * mDisplayProgram;
		Program * mGodrayRaymarchProgram;
		Program * mKnotRenderProgram;
		Program * mKnotLightProgram;
		Program * mCombineProgram;
		Program * mBackgroundProgram;

		FBO * mBackgroundFBO;

		TunnelScene * mTunnelScene;
		TessellationTestScene * mTessScene;
		ParticleLogoSDF * mLogoScene;

		void lightPass(GLContext * gl);

		void getLightMatrix(Mat4f & toLight, Mat4f & toLightClip);

		Vec3f getCameraPosition();
		Vec3f getCameraDirection();
	};

};