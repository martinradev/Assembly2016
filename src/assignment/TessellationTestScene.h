#pragma once

#include "Scene.h"

#include "3d/Mesh.hpp"
#include "3d/CameraControls.hpp"
#include "GBuffer.h"
#include "SDFGeometryFactory.h"
#include "FBO.h"
#include "GaussianFilter.h"
#include "ParticleLogoSDF.h"
#include <memory>

namespace FW {

	struct MissileMeshVertex {

		MissileMeshVertex() {}
		MissileMeshVertex(const Vec3f & pos, const Vec3f & n, float u, float v)
			: position(Vec4f(pos, u)), normal(Vec4f(n, v)) {}

		Vec4f position;
		Vec4f normal;

	};

	struct MissileMeshVertexComparator {

		bool operator()(const MissileMeshVertex & a, const MissileMeshVertex & b) {
			return a.position.x < b.position.x;
		}

	};

	class TessellationTestScene : public Scene {

	public:

		TessellationTestScene(CameraControls * camPtr, const std::string & sceneShader, GLContext * ctx, unsigned width, unsigned height, FBO * lastPassFBO);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);

		TessellationTestScene::~TessellationTestScene() {
			
		}

	private:

		void loadShaders(GLContext * ctx);
		void setupBuffers();
		void updateUniforms(Window & wnd, const CameraControls & camera);
		void lightPass(Window & wnd, const CameraControls & camera);
		void getLightMatrix(const Vec3f & cameraPos, Mat4f & toLight, Mat4f & toLightClip) const;

		void renderMeshGeometry(GLContext * ctx, GLContext::Program * prog, const Mat4f & toScreen, const Mat4f & toCamera);
		void renderRibbons(GLContext * ctx, const Mat4f & toScreen, const Mat4f & toCamera);
		void renderMissile(GLContext * ctx, const Mat4f & toScreen, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & lightPos, const Vec3f & lightDir, const Vec3f & cameraPos);
		void renderMissileLight(GLContext * ctx, const Mat4f & toScreen);
		int mWndWidth;
		int mWndHeight;
		int m_numPatchesX;
		float m_sideHalfLength;

		std::string m_sceneShader;
		std::string m_programName;
		std::string m_displayProgramName;
		std::string m_lightRenderProgramName;
		std::string m_waterRenderProgramName;
		std::string m_combineProgramName;
		std::string m_meshRenderProgramName;
		std::string m_meshRenderLightProgramName;
		std::string m_renderCurveNoTessProgramName;

		GLContext::Program * m_tessProgram;
		GLContext::Program * m_displayProgram;
		GLContext::Program * m_combineProgram;
		GLContext::Program * m_lightRenderProgram;
		GLContext::Program * m_meshRenderProgram;
		GLContext::Program * m_meshRenderLightProgram;

		GLContext::Program * m_renderCurveNoTessProgram;

		std::unique_ptr<GBuffer> m_gbuffer;
		std::unique_ptr<FBO> m_terrainLightFBO;
		std::unique_ptr<FBO> m_ribbonFBO;
		std::unique_ptr<FBO> m_godrayFBO;
		std::unique_ptr<GaussianFilter> m_gaussianFilter;
		std::unique_ptr<SDFGeometryFactory> m_sdfGeometryFactory;

		GLContext::Program * mMeshCurveRenderProgram;
		GLContext::Program * mMeshCurveRenderLightProgram;

		GLuint m_planePatchVAO;
		GLuint m_planePatchVBO;

		GLuint m_quadVAO;
		GLuint m_quadVBO;

		GLuint m_lightPatchVAO;
		GLuint m_lightPatchVBO;

		GLuint m_blurResultTex;

		// knobs
		Vec4f m_knobs[10];

		Vec2i m_ProcessTexSize;


		/*
			
		*/
		//Curve ribbonCurve;
		CameraControls * mCamPtr;
		//std::vector<Vec3f> mRibbonPoints;
		bool mDebugRenderCurve;
		void saveRibbonPath(const std::string & fileName, const std::vector<Vec3f> & vec);
		void saveRibbonPaths();
		void debugRenderPath(GLContext * gl);
		void loadRibbonPath(const std::string & fileName, std::vector<Vec3f> & vec);
		void loadRibbonPaths();
		void setupCurveGLBuffers();
		

		// point editing
		void selectControlPoint(Vec2i mousePos);
		int mSelectedControlPointIndex;
		int mSelectedCurveIndex;
		Vec3f mSelectedPointOrigValue;
		int mSelectedCurveSlider;

		// logo
		std::unique_ptr<ParticleLogoSDF> mParticleLogo;

		// last pass
		FBO * mLastPassFBO;

		std::string mUnderwaterParticlesRenderProgramName;
		GLContext::Program * mUnderwaterParticlesRenderProgram;

		std::string mUnderwaterParticlesUpdateProgramName;
		GLContext::Program * mUnderwaterParticlesUpdateProgram;

		int numUnderwaterParticles;
		GLuint mBokehTexture;
		GLuint mUnderwaterParticlesVAO;
		GLuint mUnderwaterParticlesVBO;
		void renderUnderwaterParticles(GLContext * gl, const Mat4f & toCamera, const Mat4f & toClip, const Vec3f & seaColor);
		void updateUnderwaterParticles(GLContext * gl);

		std::string mWaterSurfaceProgramName;
		GLContext::Program * mWaterSurfaceProgram;

		GLuint mWaterSurfaceTex;
		GLuint mGodrayBlurTex;
		std::string mGodrayBlurProgramName;
		GLContext::Program * mGodrayBlurProgram;

		std::string mGodrayRaymarchProgramName;
		GLContext::Program * mGodrayRaymarchProgram;

		void godrayPass(GLContext * gl, const Mat4f & toClip, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & lightPos, const Mat4f & toLightClip, const Vec3f & lightDir, const Vec3f & camPos);
		std::unique_ptr<Mesh<VertexPNTC> > mRocketMesh;
		int mRocketMeshNumVertices;
		int mRocketRenderNumVertices;
		std::string mMissileProgramName;
		GLContext::Program * mMissileProgram;

		std::string mMissileLightProgramName;
		GLContext::Program * mMissileLightProgram;

		std::string mCityRenderProgramName;
		GLContext::Program * mCityRenderProgram;

		std::string mCityLightProgramName;
		GLContext::Program * mCityLightProgram;

		std::string mAuthorRenderProgramName;
		GLContext::Program * mAuthorRenderProgram;

		void setupMissile();
		GLuint mMissileVAO;
		GLuint mMissileVBO;
		GLuint mMissleTexture;
		Vec4f mMissileSpecular;

		MeshBase * mCityMesh;
		Mat4f getCityMeshWorldMatrix();

		void loadSubmarine();
		void explodeSubmarine(GLContext * gl);
		void renderSubmarine(GLContext * gl, const Mat4f & toScreen, const Vec3f & lightPosition, const Vec3f & lightDirection, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & camPos);
		Mesh<VertexPNTC> * mSubmarineMesh;
		GLuint mSubmarineVAO;
		GLuint mSubmarineVBO;
		GLuint mParticleMaterialSSBO;
		int mNumSubmarineParticles;
		std::vector<GLuint64> mTextureHandles;

		std::string mSubmarineParticleProgramName;
		GLContext::Program * mSubmarineParticleProgram;

		std::string mSubmarineParticleLightProgramName;
		GLContext::Program * mSubmarineParticleLightProgram;
		Mat4f getSubmarineToWorldMatrix();

		std::string mSubmarineSDFMoveProgramName;
		GLContext::Program * mSubmarineSDFMoveProgram;

		void loadCamPaths();
		void loadCamPath(const std::string & camPath, std::vector<Vec3f> & camVec);
		std::vector<std::vector<Vec3f> > mCameraPaths;
		Vec3f getCameraPosition();
		Vec3f getCameraForward();

		//std::vector<Surface> mCurveSurfaces;
		std::vector<std::vector<Vec4f> > mRibbons;
		std::vector<std::vector<Vec3f> > mCurveControlPoints;
		std::vector<Curve> mRibbonCurves;
		int mProfileNumIndices;
		std::vector<GLuint> mCurveSurfacesVAO;
		std::vector<int> mCurveNumTriangles;
		std::vector<GLuint> mCurveSurfacesVBO;
		std::vector<GLuint> mCurveSurfacesIBO;

		std::unique_ptr<Mesh<VertexPNTC> > mCceMesh;
		std::unique_ptr<Mesh<VertexPNTC> > mVarkoMesh;
		void renderSurfaces(GLContext * gl, const Mat4f & toScreen, const Vec3f & lightPosition, const Vec3f & lightDirection, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & camPos);
		void renderSurfacesLight(GLContext * gl, const Mat4f & toScreen);
		void renderAuthorLogos(GLContext * gl, const Mat4f & toScreen, const Vec3f & lightPosition, const Vec3f & lightDirection, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & camPos);
	};
};