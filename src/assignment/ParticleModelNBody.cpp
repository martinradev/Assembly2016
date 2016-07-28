#include "ParticleModelNBody.h"
#include "gui/Image.hpp"
#include "ShaderSetup.h"
#include "Util.h"
#include "MeshRenderHelper.h"
#include <fstream>
#include <unordered_map>
#include <algorithm>

namespace FW {

	ParticleModelNBodyScene::ParticleModelNBodyScene(const std::string & modelFile, GLContext * ctx) :

		mMeshRenderProgramName("particle_nbody_render_particles"),
		mParticleMoveProgramName("particle_nbody_move_particles")
	{

		mMesh.reset((Mesh<VertexPNTC>*)importMesh(modelFile.c_str()));
		loadShaders(ctx);
		generateParticles();
		generateAttractors();

		m_knobs[0].y = 0.05f;
	}


	void ParticleModelNBodyScene::render(Window & wnd, const CameraControls & camera) {

		GLContext * gl = wnd.getGL();
		
		updateParticles(gl);
		
		glClearColor(0, 0, 0, 1);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		mMeshRenderProgram->use();

		Mat4f toWorld;
		toWorld.setIdentity();
		toWorld = Mat4f::scale(Vec3f(0.15f));

		Mat4f toScreen = camera.getWorldToClip() * toWorld;
		Mat4f posToWorld = toWorld;
		Mat4f normalToWorld = toWorld.inverted().transposed();

		gl->setUniform(mMeshRenderProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mMeshRenderProgram->getUniformLoc("posToWorld"), posToWorld);
		gl->setUniform(mMeshRenderProgram->getUniformLoc("normalToWorld"), normalToWorld);
		gl->setUniform(mMeshRenderProgram->getUniformLoc("lightPos"), camera.getPosition());
		for (size_t i = 0; i < mTextureHandles.size(); ++i) {
			if (!glIsTextureHandleResidentARB(mTextureHandles[i])) glMakeTextureHandleResidentARB(mTextureHandles[i]);
			// bound to unit
			char buf[32];
			sprintf_s(buf, "diffuseSamplers[%d]", int(i));
			GLint loc = mMeshRenderProgram->getUniformLoc(buf);
			glUniformHandleui64ARB(loc, mTextureHandles[i]);
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleMaterialSSBO);

		glBindVertexArray(mParticleVAO);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, mNumParticles);
		glBindVertexArray(0);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

		for (size_t i = 0; i < mTextureHandles.size(); ++i) {
			if (glIsTextureHandleResidentARB(mTextureHandles[i])) {
				glMakeTextureHandleNonResidentARB(mTextureHandles[i]);
			}
		}
		
	}

	void ParticleModelNBodyScene::updateParticles(GLContext * gl) {

		mParticleMoveProgram->use();

		static float dtPrev = m_knobs[0].y;
		


		gl->setUniform(mParticleMoveProgram->getUniformLoc("numParticles"), mNumParticles);
		gl->setUniform(mParticleMoveProgram->getUniformLoc("knob1"), m_knobs[0]);

		gl->setUniform(mParticleMoveProgram->getUniformLoc("dtUniform"), m_knobs[0].y);
		gl->setUniform(mParticleMoveProgram->getUniformLoc("dtPrevUniform"), dtPrev);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleVBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mAttractorSSBO);

		int localSizeX = 128;

		int groupSizeX = (mNumParticles + localSizeX - 1) / localSizeX;

		glDispatchCompute(groupSizeX, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

		dtPrev = m_knobs[0].y;
	}

	void ParticleModelNBodyScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		default:
			break;
		}

	}

	void ParticleModelNBodyScene::generateParticles() {

		std::vector<ParticleMaterial> materials(mMesh->numSubmeshes());

		// texture handle, location
		std::unordered_map<GLuint, int> textureMap;
		std::unordered_map<GLuint, int>::iterator textureMapIterator;

		int numTriangles = mMesh->numTriangles();
		int particlesPerTriangle = 3;
		int cParticle = 0;
		
		mNumParticles = particlesPerTriangle * numTriangles;
		Random rnd;

		std::vector<ParticleInfo> particleData(mNumParticles);

		for (int i = 0; i < mMesh->numSubmeshes(); ++i)
		{

			// copy material
			const auto & cMaterial = mMesh->material(i);;
			
			int useDiffuseTexId = -1;
			const auto & diffuseTex = cMaterial.textures[MeshBase::TextureType_Diffuse];
			if (diffuseTex.exists())
			{
				GLuint textureHandle = diffuseTex.getGLTexture();
				textureMapIterator = textureMap.find(textureHandle);

				if (textureMapIterator == textureMap.end())
				{
					GLuint64 cTexHandle = glGetTextureHandleARB(textureHandle);
					useDiffuseTexId = mTextureHandles.size();
					mTextureHandles.push_back(cTexHandle);
					textureMap[cTexHandle] = useDiffuseTexId;
				}
				else {
					useDiffuseTexId = textureMapIterator->second;
				}
			}

			materials[i] = ParticleMaterial(cMaterial.diffuse.getXYZ(), useDiffuseTexId, cMaterial.specular, -1);

			const Array<Vec3i>& idx = mMesh->indices(i);
			for (int j = 0; j < idx.getSize(); ++j)
			{

				const VertexPNTC &v0 = mMesh->vertex(idx[j][0]),
					&v1 = mMesh->vertex(idx[j][1]),
					&v2 = mMesh->vertex(idx[j][2]);



				for (int k = 0; k < particlesPerTriangle; ++k) {
					float u1 = rnd.getF32(0.0f, 1.0f);
					float u2 = rnd.getF32(0.0f, 1.0f);
					float su1 = sqrtf(u1);
					float u = 1.0f - su1;
					float v = su1 * u2;
					Vec3f point = barycentricInterpolation(u, v, v0.p, v1.p, v2.p);
					Vec3f rndVec = rnd.getVec3f(-1.0f, 1.0f).normalized();
					Vec3f normal = barycentricInterpolation(u, v, v0.n, v1.n, v2.n).normalized();
					Vec2f trigUV = barycentricInterpolation(u, v, v0.t, v1.t, v2.t);
					particleData[cParticle] = ParticleInfo(Vec4f(point, trigUV.x), Vec4f(normal, trigUV.y), i);
					++cParticle;
				}

			}
		}

		glGenBuffers(1, &mParticleMaterialSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mParticleMaterialSSBO);

		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleMaterial) * materials.size(), materials.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



		glGenVertexArrays(1, &mParticleVAO);
		glBindVertexArray(mParticleVAO);

		glGenBuffers(1, &mParticleVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mParticleVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleInfo) * mNumParticles, particleData.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + sizeof(Vec4f)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + 2*sizeof(Vec4f)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	void ParticleModelNBodyScene::generateAttractors() {

		std::vector<Attractor> attractors(1);
		attractors[0] = Attractor(Vec3f(0.0f, 20.0f, 0.0f), 250.0f);

		glGenBuffers(1, &mAttractorSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mAttractorSSBO);

		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Attractor)  * attractors.size(), attractors.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	}

	void ParticleModelNBodyScene::loadShaders(GLContext * ctx) {

		const char renderParticleVertex[] = "shaders/mesh_particle_nbody/render_vert.glsl";
		const char renderParticleFragment[] = "shaders/mesh_particle_nbody/render_frag.glsl";

		mMeshRenderProgram = loadShader(ctx,
		
			renderParticleVertex,
			renderParticleFragment
		, mMeshRenderProgramName);

		
		const char moveParticleCS[] = "shaders/mesh_particle_nbody/attractor_cs.glsl";
		mParticleMoveProgram = loadShader(ctx,
			moveParticleCS, mParticleMoveProgramName
			);


	}

	void ParticleModelNBodyScene::activate(Window & wnd, CommonControls & controls) {

		updateGUI(wnd, controls);

	}

	void ParticleModelNBodyScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}


	}

	void ParticleModelNBodyScene::updateGUI(Window & wnd, CommonControls & controls) {

		cleanUpGUI(wnd, controls);

		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(-40.0f), Vec4f(40.0f)), // knob1
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob2
			std::make_pair(Vec4f(-1.0f), Vec4f(1.0f)), // knob3
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob4
			std::make_pair(Vec4f(-10.0f), Vec4f(10.0f)), // knob5
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob6
			std::make_pair(Vec4f(-50.0f), Vec4f(50.0f)), // knob7
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob8
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob9
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)) // knob10
		};


		String xStr = sprintf("Knob%u.x = %%f", activeKnob + 1);
		String yStr = sprintf("Knob%u.y = %%f", activeKnob + 1);
		String zStr = sprintf("Knob%u.z = %%f", activeKnob + 1);
		String wStr = sprintf("Knob%u.w = %%f", activeKnob + 1);

		controls.beginSliderStack();
		controls.addSlider(&m_knobs[activeKnob].x, KNOB_SLIDE_DATA[activeKnob].first.x, KNOB_SLIDE_DATA[activeKnob].second.x, false, FW_KEY_NONE, FW_KEY_NONE, xStr);
		controls.addSlider(&m_knobs[activeKnob].y, KNOB_SLIDE_DATA[activeKnob].first.y, KNOB_SLIDE_DATA[activeKnob].second.y, false, FW_KEY_NONE, FW_KEY_NONE, yStr);
		controls.addSlider(&m_knobs[activeKnob].z, KNOB_SLIDE_DATA[activeKnob].first.z, KNOB_SLIDE_DATA[activeKnob].second.z, false, FW_KEY_NONE, FW_KEY_NONE, zStr);
		controls.addSlider(&m_knobs[activeKnob].w, KNOB_SLIDE_DATA[activeKnob].first.w, KNOB_SLIDE_DATA[activeKnob].second.w, false, FW_KEY_NONE, FW_KEY_NONE, wStr);
		controls.endSliderStack();

	}

};