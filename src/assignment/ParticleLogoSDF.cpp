#include "ParticleLogoSDF.h"
#include "Util.h"
#include "SyncVars.h"
#include "Globals.h"
#include "Samplers.h"
#include <unordered_map>
namespace FW {

	ParticleLogoSDF::ParticleLogoSDF(GLContext * gl, int numSamplesPerTrig, FBO * lastFBO, int width, int height, CameraControls & camera) :

		mRenderLogoProgramName("particle_logo_render_particles"),
		mSDFLogoProgramName("particle_logoy_move_particles"),
		mExplodeLogoProgramName("particle_logo_move_particles"),
		mCombineProgramName("particle_logo_combine_result"),
		mDisplayProgramName("particle_logo_display_result"),
		mBackgroundProgramName("particle_logo_background"),
		mRenderLogoShadowMapProgramName("particle_logo_render_shadowmap_particles"),
		mGodrayProgramName("particle_ss_godray_logo"),
		mGodrayBlurProgramName("ss_godray_blur"),
		mLastPassFBO(lastFBO),
		mLightPos(0.0,50.0f,0.0),
		mShadowViewportSize(width, height),
		mLightColor(Vec3f(0.104, 0.33, 0.51))
	{
		camera.setPosition(Vec3f(-4.0f, 5.0f, 50.0f));
		mGBuffer.reset(new GBuffer(width, height));

		GLuint fboDepthTex = TEXTURE_POOL->request(TextureDescriptor(GL_DEPTH_COMPONENT32F, mShadowViewportSize.x, mShadowViewportSize.y, GL_DEPTH_COMPONENT, GL_FLOAT))->m_texture;
		mShadowMapFBO.reset(new FBO(fboDepthTex));

		mGodrayFBO.reset(new FBO(mGBuffer->getRealDepthMap()));
		mGodrayBlurFBO.reset(new FBO(fboDepthTex));
		GLuint shadowMapTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, mShadowViewportSize.x, mShadowViewportSize.y, GL_RGBA, GL_FLOAT))->m_texture;
		GLuint godrayTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, mShadowViewportSize.x, mShadowViewportSize.y, GL_RGBA, GL_FLOAT))->m_texture;
		mGodrayBlurTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, mShadowViewportSize.x, mShadowViewportSize.y, GL_RGBA, GL_FLOAT))->m_texture;
		
		glBindTexture(GL_TEXTURE_2D, shadowMapTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, mGodrayBlurTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		mShadowMapFBO->attachTexture(0, shadowMapTex);
		mGodrayFBO->attachTexture(0, godrayTex);
		mGodrayBlurFBO->attachTexture(0, mGodrayBlurTex);

		mGaussiaFilter.reset(new GaussianFilter(gl, mShadowViewportSize));
		mMesh.reset((Mesh<VertexPNTC>*)importMesh("assets/logo/logo8.obj"));
		loadShaders(gl);
		generateParticles(numSamplesPerTrig);
		setupBuffers();
	}


	void ParticleLogoSDF::updateParticles(GLContext * gl) {

		static float prevdt = FWSync::curlStep;

		float curdt = FWSync::curlStep;

		float curlratio = abs(prevdt) < 1e-7 ? 1.0f : curdt / prevdt;

		mSDFLogoProgram->use();


		gl->setUniform(mSDFLogoProgram->getUniformLoc("numParticles"), mNumParticles);

		gl->setUniform(mSDFLogoProgram->getUniformLoc("dtCurlUniform"), curdt);
		gl->setUniform(mSDFLogoProgram->getUniformLoc("dtCurlRatioUniform"), curlratio);
		gl->setUniform(mSDFLogoProgram->getUniformLoc("dtGlobalUniform"), GLOBAL_DT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleVBO);

		int localSizeX = 128;

		int groupSizeX = (mNumParticles + localSizeX - 1) / localSizeX;

		glDispatchCompute(groupSizeX, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

		prevdt = curdt;
	}

	void ParticleLogoSDF::generateParticles(int numSamplesPerTrig) {

		std::vector<ParticleMaterial> materials(mMesh->numSubmeshes());

		// texture handle, location
		std::unordered_map<GLuint, int> textureMap;
		std::unordered_map<GLuint, int>::iterator textureMapIterator;

		int numTriangles = mMesh->numTriangles();
		int particlesPerTriangle = numSamplesPerTrig;
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
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + 2 * sizeof(Vec4f)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	void ParticleLogoSDF::setupBuffers() {
		const static F32 posAttrib[] =
		{
			-1, -1, 0, 1,
			1, -1, 0, 1,
			-1, 1, 0, 1,
			1, 1, 0, 1
		};

		glGenVertexArrays(1, &m_quadVAO);
		glGenBuffers(1, &m_quadVBO);
		glBindVertexArray(m_quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, posAttrib, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void ParticleLogoSDF::loadShaders(GLContext * ctx) {

		const char renderParticleVertex[] = "shaders/logo_particle/render_vert.glsl";
		const char renderParticleFragment[] = "shaders/logo_particle/render_frag.glsl";

		mRenderLogoProgram = loadShader(ctx,

			renderParticleVertex,
			renderParticleFragment
			, mRenderLogoProgramName);

		const char renderParticleShadowMapVertex[] = "shaders/logo_particle/render_vert_shadowmap.glsl";
		const char renderParticleShadowMapFragment[] = "shaders/logo_particle/render_frag_shadowmap.glsl";

		mRenderLogoShadowMapProgram = loadShader(ctx,

			renderParticleShadowMapVertex,
			renderParticleShadowMapFragment
			, mRenderLogoShadowMapProgramName);

		const char combineVertex[] = "shaders/logo_particle/combine_vertex.glsl";
		const char combinetFragment[] = "shaders/logo_particle/combine_fragment.glsl";

		mCombineProgram = loadShader(ctx,
			combineVertex,
			combinetFragment
			, mCombineProgramName);


		const char moveParticleCS[] = "shaders/logo_particle/sdf_cs.glsl";
		mSDFLogoProgram = loadShader(ctx,
			moveParticleCS, mSDFLogoProgramName
			);

		const char displayVertex[] = "shaders/common/display_vertex.glsl";
		const char displayFragment[] = "shaders/common/display_fragment.glsl";

		mDisplayProgram = loadShader(ctx, displayVertex, displayFragment, mDisplayProgramName);

		const char backgroundFragment[] = "shaders/logo_particle/background_fragment.glsl";

		mBackgroundProgram = loadShader(ctx,
			combineVertex,
			backgroundFragment
			, mBackgroundProgramName);

		Vec2f ldSamples[36]; // 4 different samplers
		int k = 0;
		for (int i = 0; i < 4; ++i) {
			LowDiscrepancySampler ldSampler(3);
			for (int j = 0; j < 9; ++j) {
				Vec2f cSample = 12.0f * ldSampler.getSample(j) - Vec2f(6.0f);
				ldSamples[k] = cSample;
				++k;
			}
		}

		mRenderLogoProgram->use();
		ctx->setUniformArray(mRenderLogoProgram->getUniformLoc("ldSamples"), 36, (Vec2f*)ldSamples);


		const char godrayParticlesVertex[] = "shaders/logo_particle/ss_godray_vert.glsl";
		const char godrayParticlesFragment[] = "shaders/logo_particle/ss_godray_frag.glsl";
		mGodrayProgram = loadShader(ctx, godrayParticlesVertex, godrayParticlesFragment, mGodrayProgramName);

		const char godrayBlurFrag[] = "shaders/logo_particle/godray_blur_frag.glsl";
		mGodrayBlurProgram = loadShader(ctx,
			combineVertex,
			godrayBlurFrag
			, mGodrayBlurProgramName);
	}

	void ParticleLogoSDF::renderParticles(GLContext * gl, const Mat4f & toScreen, const Mat4f & toCamera, const Mat4f & toWorld, const Mat4f & toLightClip, const Vec3f & cameraPos, const Vec3f & cameraZ) {


		mRenderLogoProgram->use();

		glBindFragDataLocation(mRenderLogoProgram->getHandle(), 0, "diffuseColorOUT");
		glBindFragDataLocation(mRenderLogoProgram->getHandle(), 1, "normalOUT");
		glBindFragDataLocation(mRenderLogoProgram->getHandle(), 2, "positionOUT");
		glBindFragDataLocation(mRenderLogoProgram->getHandle(), 3, "depthOUT");

		Mat4f normalToWorld = toWorld.inverted().transposed();

		gl->setUniform(mRenderLogoProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mRenderLogoProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mRenderLogoProgram->getUniformLoc("normalToWorld"), normalToWorld);
		gl->setUniform(mRenderLogoProgram->getUniformLoc("lightPos"), mLightPos);
		gl->setUniform(mRenderLogoProgram->getUniformLoc("lightColor"), mLightColor);
		gl->setUniform(mRenderLogoProgram->getUniformLoc("cameraDir"), cameraZ);
		gl->setUniform(mRenderLogoProgram->getUniformLoc("toLightClip"), toLightClip);
		gl->setUniform(mRenderLogoProgram->getUniformLoc("shadowMapTex"), 0);
		gl->setUniform(mRenderLogoProgram->getUniformLoc("shadowMapSize"), Vec2f(mShadowViewportSize));
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mShadowMapFBO->getTexture(0));



		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleMaterialSSBO);

		glBindVertexArray(mParticleVAO);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, mNumParticles);
		glBindVertexArray(0);

		glDisable(GL_BLEND);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

	}

	void ParticleLogoSDF::renderParticlesShadowMap(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld) {
		mRenderLogoShadowMapProgram->use();

		gl->setUniform(mRenderLogoShadowMapProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mRenderLogoShadowMapProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mRenderLogoShadowMapProgram->getUniformLoc("lightPos"), mLightPos);

		glBindVertexArray(mParticleVAO);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, mNumParticles);
		glBindVertexArray(0);

	}


	void ParticleLogoSDF::render(Window & wnd, const CameraControls & camera) {

		GLContext * gl = wnd.getGL();
		updateParticles(gl);

		//mLightColor = m_knobs[2].getXYZ();
		Mat4f toWorld;
		toWorld.setIdentity();
		toWorld = Mat4f::scale(Vec3f(5.0f));
		Mat4f toClip = camera.getWorldToClip() * toWorld;
		Mat4f toCamera = camera.getWorldToCamera() * toWorld;
		Vec3f cameraZ = camera.getForward();
		Vec2i sz = wnd.getSize();

		Vec3f cameraPos = camera.getPosition();

		
		
		Mat4f toLightClip;
		getLightMatrix(toLightClip);
		toLightClip *= toWorld;

		shadowMapPass(gl, toLightClip, toWorld);

		GLuint fbo = mGBuffer->getFBO();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glViewport(0, 0, sz.x, sz.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);
		renderBackground(gl);
		glEnable(GL_DEPTH_TEST);

		renderParticles(gl, toClip, toCamera, toWorld, toLightClip, cameraPos, cameraZ);



		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		godrayPass(gl, toClip, mLightColor, mLightPos, toLightClip, (-mLightPos).normalized(), cameraPos);

		combineResult(gl);

	}

	void ParticleLogoSDF::combineResult(GLContext * gl) {
		
		mLastPassFBO->bind();

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		mCombineProgram->use();

		glBindFragDataLocation(mCombineProgram->getHandle(), 0, "color");

		gl->setUniform(mCombineProgram->getUniformLoc("gradUniform"), FWSync::gradMix);
		gl->setUniform(mCombineProgram->getUniformLoc("gradBottom"), Vec3f(FWSync::gradR, FWSync::gradG, FWSync::gradB));

		gl->setUniform(mCombineProgram->getUniformLoc("meshColorTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGBuffer->getTexture(GBUFFER_TYPES::GBUFFER_DIFFUSE));

		gl->setUniform(mCombineProgram->getUniformLoc("godrayColorTex"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mGodrayFBO->getTexture(0));

		/*mDisplayProgram->use();

		gl->setUniform(mDisplayProgram->getUniformLoc("inTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGodrayFBO->getTexture(0));*/

		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		mLastPassFBO->unbind();
		
	}

	void ParticleLogoSDF::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		default:
			break;
		}

	}


	void ParticleLogoSDF::activate(Window & wnd, CommonControls & controls) {

		updateGUI(wnd, controls);

	}

	void ParticleLogoSDF::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		
		controls.removeControl(&actionExt);
	}

	void ParticleLogoSDF::updateGUI(Window & wnd, CommonControls & controls) {

		cleanUpGUI(wnd, controls);

		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob1
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob2
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob3
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob4
			std::make_pair(Vec4f(-10.0f), Vec4f(10.0f)), // knob5
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob6
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob7
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

	void ParticleLogoSDF::renderBackground(GLContext * gl) {
		mBackgroundProgram->use();
		gl->setUniform(mBackgroundProgram->getUniformLoc("knob1"), m_knobs[0]);
		gl->setUniform(mBackgroundProgram->getUniformLoc("knob2"), m_knobs[1]);
		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

	}
	

	void ParticleLogoSDF::shadowMapPass(GLContext * gl, const Mat4f & toScreen, const Mat4f & particleToWorld) {

		mShadowMapFBO->bind();

		glViewport(0, 0, mShadowViewportSize.x, mShadowViewportSize.y);

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderParticlesShadowMap(gl, toScreen, particleToWorld);

		mShadowMapFBO->unbind();

	}

	void ParticleLogoSDF::getLightMatrix(Mat4f & toLightScreen) {

		Vec3f lightDir = (-mLightPos).normalized();
		Vec3f lightX = Vec3f(1, 0, 0);
		Vec3f lightY = cross(lightDir, lightX).normalized();
		lightX = cross(lightY, lightDir).normalized();

		Mat4f depthMapUVN;
		depthMapUVN.setRow(0, Vec4f(lightX, 0.0f));
		depthMapUVN.setRow(1, Vec4f(lightY, 0.0f));
		depthMapUVN.setRow(2, Vec4f(lightDir, 0.0f));
		depthMapUVN.setRow(3, Vec4f(0.0, 0.0f, 0.0, 1.0f));

		toLightScreen = Mat4f::perspective(90.0f, 1.0f, 150.0f) * depthMapUVN * Mat4f::translate(-mLightPos);
	}

	void ParticleLogoSDF::godrayPass(GLContext * gl, const Mat4f & toClip, const Vec3f & lightColor, const Vec3f & lightPos, const Mat4f & toLightClip, const Vec3f & lightDir, const Vec3f & camPos) {

		Vec3f lightWorldPos = Vec3f(0.0f, 0.0f, -20.0f);

		mGodrayFBO->bind();

		glViewport(0, 0, mShadowViewportSize.x, mShadowViewportSize.y);

		glClearColor(0.0, 0.01, 0.02, 1.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		mGodrayProgram->use();

		gl->setUniform(mGodrayProgram->getUniformLoc("toScreen"), toClip);
		gl->setUniform(mGodrayProgram->getUniformLoc("colorUniform"), Vec3f(0.008f));

		glBindVertexArray(mParticleVAO);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, mNumParticles);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glBindVertexArray(0);

		glUseProgram(0);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(&toClip(0, 0));
		glMatrixMode(GL_MODELVIEW);
		Mat4f ident;
		ident.setIdentity();
		glLoadMatrixf(&ident(0, 0));

		glEnable(GL_POINT_SMOOTH);
		glPointSize(220);

		glBegin(GL_POINTS);
		glColor3f(FWSync::logoGodrayColorR, FWSync::logoGodrayColorG, FWSync::logoGodrayColorB);
		glVertex3f(0.0f, 0.0f, -20.0f);
		glEnd();

		mGodrayFBO->unbind();

		mGodrayBlurFBO->bind();

		mGodrayBlurProgram->use();

		Vec4f lightClipSpace = toClip * Vec4f(lightWorldPos, 1.0f);
		lightClipSpace /= lightClipSpace.w;
		lightClipSpace.x = lightClipSpace.x*0.5 + 0.5;
		lightClipSpace.y = lightClipSpace.y*0.5 + 0.5;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGodrayFBO->getTexture(0));

		gl->setUniform(mGodrayBlurProgram->getUniformLoc("bwSmapler"), 0);
		gl->setUniform(mGodrayBlurProgram->getUniformLoc("lightPos"), lightClipSpace.getXY());
		gl->setUniform(mGodrayBlurProgram->getUniformLoc("DECAY"), FWSync::logoGodrayDecay);

		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);
		mGodrayBlurFBO->unbind();

		mGaussiaFilter->process(gl, mGodrayBlurFBO->getTexture(0), mGodrayFBO->getTexture(0), 5);
	

		


	}



};