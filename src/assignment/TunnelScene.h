#pragma once

#include "Scene.h"
#include "FBO.h"
#include "GBuffer.h"
#include "Mesher.h"
#include "TessellationTestScene.h"
#include "Spline.h"
#include <memory>
#include <vector>

namespace FW {

	class TunnelScene : public Scene {

	public:

		TunnelScene(GLContext * ctx, int width, int height, FBO * lastPass, CameraControls * camPtr, TessellationTestScene * tessScene);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);


	private:

		void loadShaders(GLContext * ctx);
		void generateParticles();
		
		int mWidth;
		int mHeight;
		FBO * mLastFBO;
		CameraControls * mCamPtr;
	
		std::unique_ptr<FBO> mGBuffer;
		std::unique_ptr<FBO> mGodrayFBO;
		std::unique_ptr<FBO> mFogFBO;

		GLContext::Program * mCombineProgram;
		GLContext::Program * mDisplayProgram;

		GLuint mTunnelVAO;
		GLuint mTunnelVBO;
		GLuint mTunnelIBO;
		int mTunnelNumIndices;
		GLuint mTunnelTexture;
		GLuint mTunnelNormalTexture;
		GLuint mTunnelSpecularTexture;
		GLuint mBokehTexture;
		GLuint mBokehTextureStrip;

		void setupGLBuffers();
		GLuint mQuadVAO;
		GLuint mQuadVBO;

		// knobs
		Vec4f m_knobs[10];

		void generateTunnel();
		void generateRibbon();

		//std::unique_ptr<Mesher> mMesher;
		//void renderMeshObject(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition);

		Vec3f getCameraForward();
		Vec3f getCameraUp(float t);
		Vec3f getCameraPosition();

		TessellationTestScene * mTessScene;

		friend class FinalScene;

		int mNumCityParticles;
		std::vector<GLuint64> mTextureHandles;

		GLuint mCityVAO;
		GLuint mCityVBO;
		GLuint mParticleMaterialSSBO;

		void lightPass();

		typedef GLContext::Program Program;

		Program * mCityRenderProgram;
		Program * mCityLightRenderProgram;

		Program * mMeteorRenderProgram;

		void renderCity(GLContext * gl, const Mat4f & toScreen, const Vec3f & lightPosition, const Vec3f & lightDirection, const Vec3f & lightColor, const Vec3f & fogColor);

		GLuint mMeteorVAO;
		GLuint mMeteorVBO;
		GLuint mMeteorIBO;
		int mMeteorNumIndices;

		Program * mParticleMoveProgram;

		void renderMeteor(GLContext * gl, const Mat4f & toScreen);
		void explodeCity(GLContext * gl);

		std::vector<std::vector<Vec3f> > mCameraPaths;
		void loadCamPaths();

		FW::Random pRnd;

		GLContext::Program * mGodrayBlurProgram;

		std::unique_ptr<FBO> mGodrayBlurFBO;
		std::unique_ptr<GaussianFilter> mGaussiaFilter;
		GLuint mGodrayBlurTex;

		void godrayPass(GLContext * gl, const Mat4f & toScreen);

		void restartAnimation();
		GLuint mCityVBOCopy;

		Program * mBackgroundRenderProgram;
	};

};