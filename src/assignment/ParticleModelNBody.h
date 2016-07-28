#pragma once

#include "Scene.h"

#include "3d/Mesh.hpp"
#include <memory>
#include <string>

namespace FW {

	struct ParticleInfo {

		Vec4f position; // w: s coordinate
		Vec4f normal; // w: t coordinate
		float materialIndex;
		Vec3f oldPosition;

		ParticleInfo() {}
		ParticleInfo(const Vec4f & pos, const Vec4f & norm, int matIndex) :
			position(pos),
			normal(norm),
			materialIndex(matIndex),
			oldPosition(pos.getXYZ())
			{
		}

	};

	struct ParticleMaterial {

		ParticleMaterial() {};

		ParticleMaterial(const Vec3f & difC, int difTexI, const Vec3f & specC, int normTexI)
			: diffuseColor(difC), diffuseTextureIndex(difTexI), specularColor(specC), normalTextureIndex(normTexI) {}

		Vec3f diffuseColor;
		int diffuseTextureIndex;
		Vec3f specularColor;
		int normalTextureIndex;
	};

	struct Attractor {

		Attractor() {}
		Attractor(const Vec3f & pos, const float m) :
			position(pos), mass(m) {}

		Vec3f position;
		float mass;

	};

	class ParticleModelNBodyScene : public Scene {

	public:

		ParticleModelNBodyScene(const std::string & modelFile, GLContext * ctx);

		void render(Window & wnd, const CameraControls & camera);
		void handleAction(const Action & action, Window & wnd, CommonControls & controls);
		void activate(Window & wnd, CommonControls & controls);
		void updateGUI(Window & wnd, CommonControls & controls);
		void cleanUpGUI(Window & wnd, CommonControls & controls);
		

	private:

		void loadShaders(GLContext * ctx);
		void generateParticles();
		void generateAttractors();
		void updateParticles(GLContext * gl);

		std::string mMeshRenderProgramName;
		std::string mParticleMoveProgramName;

		GLContext::Program * mMeshRenderProgram;
		GLContext::Program * mParticleMoveProgram;

		std::unique_ptr<Mesh<VertexPNTC> > mMesh;

		int mNumParticles;
		GLuint mParticleVAO;
		GLuint mParticleVBO;

		GLuint mParticleMaterialSSBO;
		std::vector<GLuint64> mTextureHandles;

		GLuint mAttractorSSBO;
		int mNumAttractors;

		// knobs
		Vec4f m_knobs[10];

	};

};