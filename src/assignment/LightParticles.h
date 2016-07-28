#pragma once

#include "gpu/GLContext.hpp"
#include "base/Math.hpp"
#include "3d/Mesh.hpp"
#include "GBuffer.h"
#include "GPUBvh.h"
#include <string>
#include <memory>

namespace FW {

	struct LightParticleDef {

		Vec4f position;
		Vec4f direction;
		Vec4f normal;
		Vec4f original_position;

		LightParticleDef() {}

		LightParticleDef(const Vec3f & pos, const Vec3f & dir, const Vec3f & n, float age) :
			position(Vec4f(pos, 1)),
			direction(Vec4f(dir, age)),
			normal(Vec4f(n, 0)),
			original_position(Vec4f(pos, 1))
		{
		}

	};

	enum class ParticleSystemState {
		SLEEP, MOVE, EXPLODE, ABSORB
	};

	class LightParticleSystem {

	public:

		explicit LightParticleSystem(GLContext * gl, GPUBvh_Buffers & bvhBuffer, const std::string & programName, unsigned numParticles, const Vec3f & origin);

		explicit LightParticleSystem(GLContext * gl, GPUBvh_Buffers & bvhBuffer, const std::string & programName, unsigned numParticles, Mesh<VertexPNTC>*  mesh);

		explicit LightParticleSystem(GLContext * gl, GPUBvh_Buffers & bvhBuffer, const std::string & programName, GLuint particleSSBO);

		void update(GLContext * gl);

		void render(GLContext * gl, const Mat4f & posToClip, const GBuffer * gbuffer);

		void setupShaders(GLContext * gl);

		void updateSystemState(float off, float explode, float absorb, bool isOn);

		void restartSystem(GLContext * gl, Mesh<VertexPNTC> * mesh);

		void setNumParticles(unsigned numParticles) {
			m_numParticles = numParticles;
		}

		void setSceneType(float val) {
			m_sceneType = val;
		}

		void setPointSize(float sz) {
			m_pointSize = sz;
		}

	private:

		
		void restartSystem(GLContext * gl);

		std::string m_programName;
		std::string m_transCSProgramName;
		std::string m_renderParticlesProgram;
		Vec3f m_origin;
		Mesh<VertexPNTC> *  m_mesh;
		unsigned m_numParticles;

		GLuint m_particlesSSBO;

		GPUBvh_Buffers m_bvhBuffer;

		ParticleSystemState m_systemState;

		float m_offset;
		float m_explode;
		float m_absorb;
		bool m_isOn;
		float m_pointSize;
		float m_sceneType;

	};

};