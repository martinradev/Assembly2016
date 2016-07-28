#pragma once

#include "Scene.h"
#include "FBO.h"
#include "Mesher.h"
#include "GaussianFilter.h"
#include "curve.h"
#include <memory>

namespace FW {


	class SpaceScene : public Scene {

	public:

		SpaceScene(GLContext * ctx, int width, int height, FBO * lastPass, CameraControls * camPtr);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);


	private:

		void loadShaders(GLContext * ctx);

		void loadNefertiti();

		void loadSkybox();
	
		// knobs
		Vec4f m_knobs[10];

		FBO * mLastFBO;

		std::string mRenderNefertitiProgramName;
		GLContext::Program * mRenderNefertitiProgram;

		std::string mRenderParticleCloudProgramName;
		GLContext::Program * mRenderParticleCloudProgram;

		std::string mParticleCloudMoveProgramName;
		GLContext::Program * mParticleCloudMoveProgram;

		std::string mNefertitiMoveProgramName;
		GLContext::Program * mNefertitiMoveProgram;

		std::string mCombineProgramName;
		GLContext::Program * mCombineProgram;

		std::string mDisplayProgramName;
		GLContext::Program * mDisplayProgram;

		std::string mBackgroundProgramName;
		GLContext::Program * mBackgroundProgram;

		std::string mMeshObjectProgramName;
		GLContext::Program * mMeshObjectProgram;

		void renderNefertiti(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition);

		Mesh<VertexPNTC> * mNeferittiMesh;
		GLuint mNeferittiVAO;
		GLuint mNefertitiVBO;
		GLuint mParticleMaterialSSBO;
		int mNumNefertitiParticles;
		std::vector<GLuint64> mTextureHandles;

		std::unique_ptr<FBO> mNefertitiFBO;

		void setupGLBuffers();
		GLuint mQuadVAO;
		GLuint mQuadVBO;


		Vec3f getLightPosition(int i);
		Mat4f getLightMatrix(int i);
		Vec3f getLightColor(int i);
		Vec3f getLightDirection(int i);

		//std::unique_ptr<FBO> mSkyBoxFBO;

		GLuint mBackgroundTex;
		void renderBackground(GLContext * gl);

		void renderStatueParticles(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition);

		GLuint mDynamicSkyBoxTex;

		int mParticleCloudNum;
		GLuint mParticleCloudVBO;
		GLuint mParticleCloudVAO;
		void generateParticleCloud();
		void moveParticleCloud(GLContext * gl);
		void moveNefertiti(GLContext * gl);

		std::unique_ptr<Mesher> mMesher;
		void renderMeshObject(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition);
		
		Vec3f getCameraForward();
		Vec3f getCameraPosition();
		CameraControls * mCamPtr;

		void loadRibbons();
		void loadRibbonPath(const std::string & fileName, std::vector<Vec3f> & vec);
		std::vector<std::vector<Vec3f> > mControlPoints;
		std::vector<GLuint> mRibbonVAOs;
		std::vector<GLuint> mRibbonVBOs;
		std::vector<GLuint> mRibbonIBOs;
		std::vector<int> mRibbonNumTriangles;
		void debugRenderRibbons();
		bool mDebugRenderCurve;
		void selectControlPoint(Vec2i pos);
		int mSelectedControlPointIndex;
		int mSelectedCurveIndex;
		Vec3f mSelectedPointOrigValue;
		Vec2i mWndSize;
		int mProfileNumIndices;
		void saveRibbonPaths();
		void saveRibbonPath(const std::string & fileName, const std::vector<Vec3f> & vec);
		void renderRibbons(GLContext * gl, const Mat4f & toScreen, const Vec3f & cameraPosition);

		GLContext::Program * mMeshCurveRenderProgram;
		GLContext::Program * mParticleGodrayProgram;
		GLContext::Program * mGodrayProgram;
		GLContext::Program * mGodrayBlurProgram;

		GLuint mSkyBoxTex;
		std::unique_ptr<FBO> mGodrayFBO;
		std::unique_ptr<FBO> mGodrayBlurFBO;
		std::unique_ptr<GaussianFilter> mGaussiaFilter;
		GLuint mGodrayBlurTex;
		void godrayPass(GLContext * gl, const Mat4f & nefertitiToScreen, const Mat4f & cloudToScreen, const Mat4f & sdfToScreen, const Mat4f & ribbonsToscreen);
		GLuint mAttractorSSBO;
		int mNumAttractors;
};

};