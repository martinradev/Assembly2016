#pragma once

#include "3d/Mesh.hpp"
#include "3d/CameraControls.hpp"
#include "ParticleModelNBody.h"
#include "FBO.h"
#include "GBuffer.h"
#include "GaussianFilter.h"
#include "Scene.h"
#include <memory>
#include <string>
#include <vector>

namespace FW {

	class ParticleLogoSDF : public Scene {

	public:

		ParticleLogoSDF(GLContext * gl, int numSamplesPerTrig, FBO * lastFBO, int width, int height, CameraControls & camera);
		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);

		void updateParticles(GLContext * gl);
		void explodeParticles(GLContext * gl);
		
		void loadShaders(GLContext * ctx);


	private:
		void moveParticles(GLContext * gl);
		void renderBackground(GLContext * gl);
		void renderParticles(GLContext * gl, const Mat4f & toScreen, const Mat4f & toCamera, const Mat4f & toWorld, const Mat4f & toLightClip, const Vec3f & camPos, const Vec3f & cameraZ);
		void renderParticlesShadowMap(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld);
		void generateParticles(int numSamplesPerTrig);
		void setupBuffers();
		void combineResult(GLContext * gl);
		void shadowMapPass(GLContext * gl, const Mat4f & toScreen, const Mat4f & particleToWorld);
		void godrayPass(GLContext * gl, const Mat4f & toClip, const Vec3f & lightColor, const Vec3f & lightPos, const Mat4f & toLightClip, const Vec3f & lightDir, const Vec3f & camPos);
		void getLightMatrix(Mat4f & toLightScreen);

		std::unique_ptr<Mesh<VertexPNTC> > mMesh;

		std::string mRenderLogoProgramName;
		GLContext::Program * mRenderLogoProgram;

		std::string mRenderLogoShadowMapProgramName;
		GLContext::Program * mRenderLogoShadowMapProgram;

		std::string mSDFLogoProgramName;
		GLContext::Program * mSDFLogoProgram;

		std::string mExplodeLogoProgramName;
		GLContext::Program * mExplodeLogoProgram;

		std::string mCombineProgramName;
		GLContext::Program * mCombineProgram;

		std::string mDisplayProgramName;
		GLContext::Program * mDisplayProgram;

		std::string mBackgroundProgramName;
		GLContext::Program * mBackgroundProgram;

		std::string mGodrayProgramName;
		GLContext::Program * mGodrayProgram;

		std::string mGodrayBlurProgramName;
		GLContext::Program * mGodrayBlurProgram;

		int mNumParticles;
		GLuint mParticleVAO;
		GLuint mParticleVBO;

		GLuint mParticleMaterialSSBO;
		std::vector<GLuint64> mTextureHandles;

		std::unique_ptr<GBuffer> mGBuffer;
		FBO *mLastPassFBO;

		Vec4f m_knobs[10];

		GLuint m_quadVAO;
		GLuint m_quadVBO;

		Vec3f mLightPos;
		Vec3f mLightColor;


		/*
			shadow mapping
		*/
		std::unique_ptr<GaussianFilter> mGaussiaFilter;
		std::unique_ptr<FBO> mShadowMapFBO;
		std::unique_ptr<FBO> mGodrayFBO;
		std::unique_ptr<FBO> mGodrayBlurFBO;
		Vec2i mShadowViewportSize;
		GLuint mGodrayBlurTex;
		GLuint mGodrayGaussianBlurredTex;

		CameraControls * mCamPtr;

		friend class FinalScene;
	};

};