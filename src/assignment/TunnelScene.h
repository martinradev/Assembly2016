#pragma once

#include "Scene.h"
#include "FBO.h"
#include "GBuffer.h"
#include "Mesher.h"

#include <memory>
#include <vector>

namespace FW {

	class TunnelScene : public Scene {

	public:

		TunnelScene(GLContext * ctx, int width, int height, FBO * lastPass, CameraControls * camPtr);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);


	private:

		void loadShaders(GLContext * ctx);

		
		int mWidth;
		int mHeight;
		FBO * mLastFBO;
		CameraControls * mCamPtr;
	
		std::unique_ptr<FBO> mGBuffer;
		std::unique_ptr<FBO> mGodrayFBO;
		std::unique_ptr<FBO> mMotionBlurFBO;

		GLContext::Program * mCombineProgram;
		GLContext::Program * mDisplayProgram;
		GLContext::Program * mTunnelProgram;
		GLContext::Program * mMotionBlurProgram;
		GLContext::Program * mMeshProgram;
		GLContext::Program * mTunnelEngravingProgram;

		GLuint mTunnelVAO;
		GLuint mTunnelVBO;
		GLuint mTunnelIBO;
		int mTunnelNumIndices;
		GLuint mTunnelTexture;
		GLuint mTunnelNormalTexture;
		GLuint mTunnelSpecularTexture;

		GLuint mTunnelEngravingVAO;
		GLuint mTunnelEngravingVBO;
		GLuint mTunnelEngravingIBO;
		GLuint mTunnelEngravingTransformationVBO;
		int mTunnelEngravingNumIndices;
		int mNumEngravings;

		void setupGLBuffers();
		GLuint mQuadVAO;
		GLuint mQuadVBO;

		// knobs
		Vec4f m_knobs[10];

		void generateTunnel();

		std::unique_ptr<Mesher> mMesher;
		void renderMeshObject(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition);

		Vec3f getCameraForward(float t);
		Vec3f getCameraUp(float t);
		Vec3f getCameraPosition(float t);


		friend class FinalScene;
	};

};