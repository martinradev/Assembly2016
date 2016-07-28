#pragma once

#include "Scene.h"
#include "GBuffer.h"
#include "ForwardShading.h"
#include "SSAOKernel.h"
#include "GaussianFilter.h"
#include "LightParticles.h"
#include "DisplacedMesh.h"
#include "DynamicMarchingTetrahedra.h"
#include <memory>

namespace FW {

	class ForwardShadingScene : public Scene {

	public:

		ForwardShadingScene(
			const std::string & sceneShader,
			GLContext * ctx,
			unsigned width, unsigned height);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);
		void setMesh(MeshBase * mesh) {
			m_mesh.reset(mesh);
		}

		ForwardShadingScene::~ForwardShadingScene() {

		}

		void setDisplacedMesh(DisplacedMesh * dispMesh) {
			m_displacedMesh.reset(dispMesh);
		}

		void setDisplacedMeshNewPos(const Vec3f & newPos) {
			m_displacedMeshOff = newPos;
		}

		GBuffer * getGBufferPtr() {
			return m_gbuffer.get();
		}

		void setParticleSystem(LightParticleSystem * system) {
			m_particleSystem = system;
		}

		void setDMTMesh(DynamicMarchingTetrahedra * dmtMesh) {
			m_dmtMesh.reset(dmtMesh);
		}

		void setDMParticleSystem(LightParticleSystem * dmParticleSystem) {
			m_dmParticleSystem = dmParticleSystem;
		}

		void setDMTRenderState(bool newState) {
			m_renderDMTMesh = newState;
		}

		void updateBounce(float bTime, float bPeriod, float bScale) {
			m_bTime = bTime;
			m_bPeriod = bPeriod;
			m_bScale = bScale;
		}

		void setBlurOut(float blurOut) {
			m_blurOut = blurOut;
		}


	private:
		void loadShaders(GLContext * ctx);
		void updateUniforms(Window & wnd, const CameraControls & camera);
		void shadowMapPass(Window & wnd, const CameraControls & camera);
		void getLightMatrices(Mat4f & toCamera, Mat4f & toScreen);

		std::string m_sceneShader;
		std::string m_programName;
		std::string m_aoProgram;
		std::string m_shadowMapProgramName;

		std::unique_ptr<MeshBase> m_mesh;
		std::unique_ptr<GBuffer> m_gbuffer;
		GLuint m_depthFBO;
		GLuint m_depthMap; // for depth shadow mapping
		std::unique_ptr<DisplacedMesh> m_displacedMesh;
		std::unique_ptr<DynamicMarchingTetrahedra> m_dmtMesh;
		bool m_renderDMTMesh;
		LightParticleSystem * m_particleSystem;
		LightParticleSystem * m_dmParticleSystem;

		Vec3f m_displacedMeshOff;
		float m_bScale;
		float m_bTime;
		float m_bPeriod;
		float m_blurOut;

		// knobs
		Vec4f m_knobs[10];

		SSAOKernel m_ssaoKernel;

		GaussianFilter m_gaussianFilter;

	};

}