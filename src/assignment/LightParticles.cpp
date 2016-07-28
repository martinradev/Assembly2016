#include "LightParticles.h"

#include "ShaderSetup.h"
#include "GPUBvh.h"
#include "SBVH.h"
#include "Util.h"
#include "MemoryAllignedAllocator.h"
#include "GlobalSyncVars.h"

namespace FW {

	LightParticleSystem::LightParticleSystem(GLContext * gl, GPUBvh_Buffers & bvhBuffer, const std::string & programName, unsigned numParticles, const Vec3f & origin) :
		m_programName(programName),
		m_origin(origin),
		m_numParticles(numParticles),
		m_particlesSSBO(0),
		m_mesh(nullptr),
		m_bvhBuffer(bvhBuffer),
		m_renderParticlesProgram("render_particles_txt"),
		m_transCSProgramName("particles_move_trans_cs"),
		m_systemState(ParticleSystemState::SLEEP),
		m_offset(0.0f),
		m_explode(0.0f),
		m_isOn(false),
		m_pointSize(6.5),
		m_sceneType(1.0f)

	{
		setupShaders(gl);
		restartSystem(gl);
	}
	
	
	LightParticleSystem::LightParticleSystem(GLContext * gl, GPUBvh_Buffers & bvhBuffer, const std::string & programName, unsigned numParticles, Mesh<VertexPNTC> *  mesh) :
		m_programName(programName),
		m_origin(Vec3f(0)),
		m_numParticles(numParticles),
		m_particlesSSBO(0),
		m_bvhBuffer(bvhBuffer),
		m_transCSProgramName("particles_move_trans_cs"),
		m_systemState(ParticleSystemState::SLEEP),
		m_offset(0.0f),
		m_explode(0.0f),
		m_isOn(false),
		m_pointSize(6.5),
		m_sceneType(1.0f)
	{
		m_mesh = mesh;
		setupShaders(gl);
		restartSystem(gl);
	}

	LightParticleSystem::LightParticleSystem(GLContext * gl, GPUBvh_Buffers & bvhBuffer, const std::string & programName, GLuint particleSSBO)
		:
		m_programName(programName),
		m_origin(Vec3f(0)),
		m_numParticles(0),
		m_particlesSSBO(particleSSBO),
		m_bvhBuffer(bvhBuffer),
		m_transCSProgramName("particles_move_trans_cs"),
		m_systemState(ParticleSystemState::SLEEP),
		m_offset(0.0f),
		m_explode(0.0f),
		m_isOn(false),
		m_mesh(nullptr),
		m_pointSize(6.5),
		m_sceneType(1.0f)
	{
		setupShaders(gl);
	}
	
	void LightParticleSystem::setupShaders(GLContext * gl) {

		const char bvhCS[] = "shaders/bvh/bvh.glsl";
		const char declShader[] = "shaders/raymarch/decl.glsl";
		const char knobsShader[] = "shaders/raymarch/knobs.glsl";
		const char noiseShader[] = "shaders/raymarch/noise.glsl";

		loadShader(gl,
		{ declShader, bvhCS, m_programName }, m_programName);
		gl->checkErrors();

		const char partTransShader[] = "shaders/shading_comparison/transform_particles.glsl";
		loadShader(gl,
		 partTransShader , m_transCSProgramName);
		gl->checkErrors();

		const char vertexShader[] = "shaders/shading_comparison/particle_vertex.glsl";
		const char fragShader[] = "shaders/shading_comparison/particle_fragment.glsl";
		loadShader(gl,

		{ declShader, vertexShader },
		{ declShader, noiseShader, fragShader }, m_renderParticlesProgram);
		gl->checkErrors();
	}

	void LightParticleSystem::update(GLContext * gl) {

		if (m_isOn == false) return;

		if (m_systemState == ParticleSystemState::SLEEP) return;

		if (m_systemState == ParticleSystemState::EXPLODE) {

			GLContext::Program * prog = gl->getProgram(m_programName.c_str());
			prog->use();

			gl->setUniform(prog->getUniformLoc("numParticles"), m_numParticles);
			gl->setUniform(prog->getUniformLoc("sceneType"), m_sceneType);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_particlesSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, m_bvhBuffer.m_triangles_ssbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, m_bvhBuffer.m_indices_ssbo);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, m_bvhBuffer.m_nodes_ssbo);

			static const int localGroupSize = 64;
			unsigned numGroups = (m_numParticles + localGroupSize - 1) / localGroupSize;
			
			glDispatchCompute(numGroups, 1, 1);

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, 0);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, 0);
		}
		else if (m_systemState == ParticleSystemState::MOVE) {
			GLContext::Program * prog = gl->getProgram(m_transCSProgramName.c_str());
			prog->use();

			gl->setUniform(prog->getUniformLoc("numParticles"), m_numParticles);
			gl->setUniform(prog->getUniformLoc("transDirection"), Vec3f(-11,0,0)*m_offset);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_particlesSSBO);


			static const int localGroupSize = 64;
			unsigned numGroups = (m_numParticles + localGroupSize - 1) / localGroupSize;
			
			glDispatchCompute(numGroups, 1, 1);

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		}
	}

	void LightParticleSystem::restartSystem(GLContext * gl) {
		GLContext::Program * prog = gl->getProgram(m_programName.c_str());
		prog->use();

		if (m_particlesSSBO == 0) {
			glGenBuffers(1, &m_particlesSSBO);
		}
		int numTriangles = m_mesh->numTriangles();
		
		if (m_numParticles < numTriangles) m_numParticles = numTriangles;

		if (m_numParticles % numTriangles) {
			// make divisable
			m_numParticles -= m_numParticles % numTriangles;
			m_numParticles += numTriangles;
		}

		Random rnd;

		std::vector<LightParticleDef, aligned_allocator<LightParticleDef, 16> > particleData(m_numParticles);

		if (m_mesh == nullptr) {
			for (int i = 0; i < m_numParticles; ++i) {
				particleData[i] = LightParticleDef(m_origin, rnd.getVec3f(-1.0f, 1.0f).normalized(), Vec3f(0,1,0), 0);
			}
		}
		else {
			int numTriangles = m_mesh->numTriangles();
			int particlesPerTriangle = m_numParticles / numTriangles;
			int cParticle = 0;
			for (int i = 0; i < m_mesh->numSubmeshes(); ++i)
			{
				const Array<Vec3i>& idx = m_mesh->indices(i);
				for (int j = 0; j < idx.getSize(); ++j)
				{

					const VertexPNTC &v0 = m_mesh->vertex(idx[j][0]),
						&v1 = m_mesh->vertex(idx[j][1]),
						&v2 = m_mesh->vertex(idx[j][2]);

					

					for (int k = 0; k < particlesPerTriangle; ++k) {
						float u1 = rnd.getF32(0.0f, 1.0f);
						float u2 = rnd.getF32(0.0f, 1.0f);
						float su1 = sqrtf(u1);
						float u = 1.0f - su1;
						float v = su1 * u2;
						Vec3f point = barycentricInterpolation(u, v, v0.p, v1.p, v2.p);
						Vec3f rndVec = rnd.getVec3f(-1.0f, 1.0f).normalized();
						Vec3f normal = barycentricInterpolation(u, v, v0.n, v1.n, v2.n).normalized();
						particleData[cParticle] =
							LightParticleDef(point, rndVec, normal, 0);
						++cParticle;
					}

				}
			}
		}

		
		glBindBuffer(GL_ARRAY_BUFFER, m_particlesSSBO);
		glBufferData(GL_ARRAY_BUFFER, m_numParticles * sizeof(LightParticleDef), particleData.data(), GL_STATIC_DRAW);

		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void LightParticleSystem::restartSystem(GLContext * gl, Mesh<VertexPNTC> * mesh) {
		m_mesh = mesh;
		restartSystem(gl);
	}

	void LightParticleSystem::render(GLContext * gl, const Mat4f & posToClip, const GBuffer * gbuffer) {
		if (m_isOn == false) return;
		GLContext::Program * prog = gl->getProgram(m_renderParticlesProgram.c_str());

		glBindFragDataLocation(prog->getHandle(), 0, "diffuse");
		glBindFragDataLocation(prog->getHandle(), 1, "normal");
		glBindFragDataLocation(prog->getHandle(), 2, "position");
		glBindFragDataLocation(prog->getHandle(), 3, "depth");

		prog->use();

		gl->setUniform(prog->getUniformLoc("posToClip"), posToClip);
		gl->setUniform(prog->getUniformLoc("ageUniform"), m_absorb);
		gl->setUniform(prog->getUniformLoc("ssaoMask"), 0.0f);

		glEnable(GL_POINT_SMOOTH);
		glPointSize(m_pointSize);

		glBindBuffer(GL_ARRAY_BUFFER, m_particlesSSBO);
		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LightParticleDef), 0);

		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LightParticleDef), (GLvoid*)((char*)NULL + sizeof(float)*8));

		glEnableVertexAttribArray(2); // age
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(LightParticleDef), (GLvoid*)((char*)NULL + sizeof(float) * 11));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glDrawArrays(GL_POINTS, 0, m_numParticles);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisable(GL_BLEND);
	}

	void LightParticleSystem::updateSystemState(float offset, float explode, float absorb, bool isOn) {
		m_offset = offset;
		m_explode = explode;
		m_absorb = absorb;
		m_isOn = isOn;
		if (m_offset != 0.0f) {
			m_systemState = ParticleSystemState::MOVE;
		}
		else if (m_explode != 0.0f) {
			m_systemState = ParticleSystemState::EXPLODE;
		}
		else {
			m_systemState = ParticleSystemState::SLEEP;
		}
	}

};