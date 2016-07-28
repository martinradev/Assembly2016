#include "ForwardShading.h"
#include "TexturePool.h"
#include "MeshRenderHelper.h"
#include "Globals.h"
#include "Samplers.h"
namespace FW {

	ForwardShadingScene::ForwardShadingScene(const std::string & sceneShader, GLContext * ctx, unsigned width, unsigned height) :

		m_sceneShader(sceneShader),
		m_programName(sceneShader),
		m_aoProgram("basic_ssao_program"),
		m_mesh(nullptr),
		m_particleSystem(nullptr),
		m_renderDMTMesh(true),
		m_shadowMapProgramName("shadowmap_prog"),
		m_blurOut(0.0f)
	{
		loadShaders(ctx);
		m_gbuffer.reset(new GBuffer(width, height));
		m_ssaoKernel = SSAOKernel(16);
		m_gaussianFilter = GaussianFilter(ctx, Vec2i(width, height));

		glGenFramebuffers(1, &m_depthFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_depthFBO);
		
		m_depthMap = TEXTURE_POOL->request(TextureDescriptor(GL_R32F, width, height, GL_RED, GL_FLOAT))->m_texture;
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_depthMap, 0);

		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_gbuffer->getRealDepthMap(), 0);
		GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT3 };

		glDrawBuffers(1, DrawBuffers);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	}

	void ForwardShadingScene::loadShaders(GLContext * ctx) {
		const char vertexShader[] = "shaders/shading_comparison/vertex_transform.glsl";
		const char fragmentShader[] = "shaders/shading_comparison/forward_fragment.glsl";
		const char hgSdfShader[] = "shaders/raymarch/hg_sdf.glsl"; // mercury's sdf library
		const char declShader[] = "shaders/raymarch/decl.glsl";
		const char knobsShader[] = "shaders/raymarch/knobs.glsl";

		loadShader(ctx, 
			
		{ declShader, hgSdfShader, vertexShader },
		{ declShader, fragmentShader }, m_programName);
		ctx->checkErrors();

		// ssao program

		const char vertexPassShader[] = "shaders/raymarch/quad.vertex";
		const char ssaoShader[] = "shaders/postprocess/ssao.glsl";

		loadShader(ctx,
			vertexPassShader,
			ssaoShader, m_aoProgram);
		ctx->checkErrors();

		// shadow map program
		const char shadowmapVertexShader[] = "shaders/shading_comparison/shadowmap_vert.glsl";
		const char shadowmapFragmentShader[] = "shaders/shading_comparison/shadowmap_frag.glsl";

		loadShader(ctx,
			shadowmapVertexShader,
			shadowmapFragmentShader, m_shadowMapProgramName);
		ctx->checkErrors();

		// update static uniforms

		Vec2f ldSamples[16]; // 4 different samplers
		int k = 0;
		for (int i = 0; i < 4; ++i) {
			LowDiscrepancySampler ldSampler(2);
			for (int j = 0; j < 4; ++j) {
				Vec2f cSample = 2.0f * ldSampler.getSample(j) - Vec2f(1.0f);
				ldSamples[k] = cSample;
				++k;
			}
		}
		GLContext::Program * prog = ctx->getProgram(m_aoProgram.c_str());

		prog->use();

		ctx->setUniformArray(prog->getUniformLoc("ldSamples"), 16, (Vec2f*)ldSamples);
	}
	
	void ForwardShadingScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		default:
			break;
		}
	}
	
	void ForwardShadingScene::activate(Window & wnd, CommonControls & controls) {
		updateGUI(wnd, controls);
	}
	
	void ForwardShadingScene::updateGUI(Window & wnd, CommonControls & controls) {
		
		cleanUpGUI(wnd, controls);

		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(-40.0f), Vec4f(40.0f)), // knob1
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob2
			std::make_pair(Vec4f(-1.0f), Vec4f(1.0f)), // knob3
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob4
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob5
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
	
	void ForwardShadingScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}
	}

	void ForwardShadingScene::updateUniforms(Window & wnd, const CameraControls & camera) {
		GLContext * ctx = wnd.getGL();

		// update ray march program uniforms
		GLContext::Program * prog = ctx->getProgram(m_programName.c_str());

		prog->use();

		ctx->setUniform(prog->getUniformLoc("knob1"), m_knobs[0]);
		ctx->setUniform(prog->getUniformLoc("knob2"), m_knobs[1]);
		ctx->setUniform(prog->getUniformLoc("knob3"), m_knobs[2]);
		ctx->setUniform(prog->getUniformLoc("knob4"), m_knobs[3]);
		ctx->setUniform(prog->getUniformLoc("knob5"), m_knobs[4]);
		ctx->setUniform(prog->getUniformLoc("knob6"), m_knobs[5]);
		ctx->setUniform(prog->getUniformLoc("knob7"), m_knobs[6]);
		ctx->setUniform(prog->getUniformLoc("knob8"), m_knobs[7]);
		ctx->setUniform(prog->getUniformLoc("knob9"), m_knobs[8]);
		ctx->setUniform(prog->getUniformLoc("knob10"), m_knobs[9]);

		ctx->setUniform(prog->getUniformLoc("bounceTime"), m_bTime);
		ctx->setUniform(prog->getUniformLoc("bouncePeriod"), m_bPeriod);
		ctx->setUniform(prog->getUniformLoc("bounceScale"), m_bScale);

	}

	void ForwardShadingScene::render(Window & wnd, const CameraControls & camera) {
		GLContext * gl = wnd.getGL();

		if (m_particleSystem) {
			m_particleSystem->update(gl);
		}

		if (m_dmtMesh && m_renderDMTMesh) {
			m_dmtMesh->update(gl);
		}

		if (!m_renderDMTMesh && m_dmParticleSystem) {
			m_dmParticleSystem->setNumParticles(m_dmtMesh->getNumTriangles());
			m_dmParticleSystem->updateSystemState(0, 1, 0.0, true);
			m_dmParticleSystem->update(gl);
		}

		updateUniforms(wnd, camera);
		Vec2i sz = wnd.getSize();

		Mat4f toCamera = camera.getWorldToCamera();
		Mat4f toScreen = camera.getWorldToClip();

		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_depthFBO);

		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shadowMapPass(wnd, camera);
		
		GLuint fbo = m_gbuffer->getFBO();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		glClearColor(0, 0, 0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLContext::Program * prog = gl->getProgram(m_programName.c_str());
		prog->use();


		gl->setUniform(prog->getUniformLoc("toCamera"), toCamera);
		gl->setUniform(prog->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(prog->getUniformLoc("normalToCamera"), toCamera.inverted().transposed());

		if (m_mesh) {

			glBindFragDataLocation(prog->getHandle(), 0, "diffuse");
			glBindFragDataLocation(prog->getHandle(), 1, "normal");
			glBindFragDataLocation(prog->getHandle(), 2, "position");
			glBindFragDataLocation(prog->getHandle(), 3, "depth");
			gl->setUniform(prog->getUniformLoc("ssaoMask"), 1.0f);

			Mat4f toWorld;
			toWorld.setIdentity();

			gl->setUniform(prog->getUniformLoc("toWorld"), toWorld);
			gl->setUniform(prog->getUniformLoc("bounceEffect"), true);

			renderMesh(gl, m_mesh.get(), prog);
		}

		
		if (m_renderDMTMesh && m_dmtMesh) {

			glBindFragDataLocation(prog->getHandle(), 0, "diffuse");
			glBindFragDataLocation(prog->getHandle(), 1, "normal");
			glBindFragDataLocation(prog->getHandle(), 2, "position");
			glBindFragDataLocation(prog->getHandle(), 3, "depth");
			Mat4f toWorld = Mat4f::translate(m_displacedMeshOff);
			gl->setUniform(prog->getUniformLoc("toWorld"), toWorld);
			Mat4f toCameraWW = toCamera*toWorld;
			Mat4f toScreenWW = toScreen*toWorld;
			gl->setUniform(prog->getUniformLoc("normalToCamera"), toCameraWW.inverted().transposed());
			gl->setUniform(prog->getUniformLoc("toScreen"), toScreenWW);
			gl->setUniform(prog->getUniformLoc("toCamera"), toCameraWW);
			gl->setUniform(prog->getUniformLoc("ssaoMask"), 1.0f);
			gl->setUniform(prog->getUniformLoc("bounceEffect"), false);
			m_dmtMesh->renderMesh(gl, prog);

		}
		else if (!m_renderDMTMesh && m_dmParticleSystem) {
			m_dmParticleSystem->render(gl, toScreen, m_gbuffer.get());
		}

		if (m_particleSystem) {
			m_particleSystem->render(gl, toScreen, m_gbuffer.get());
		}

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		

		// blur diffuse texture
		
		//m_gaussianFilter.process(gl, m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_DEPTH), m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_DEPTH), m_knobs[0].x);

		GLContext::Program * ssaoProg = gl->getProgram(m_aoProgram.c_str());
		ssaoProg->use();


		gl->setUniform(ssaoProg->getUniformLoc("toCamera"), toCamera);
		gl->setUniform(ssaoProg->getUniformLoc("normalToCamera"), toCamera.inverted().transposed());
		gl->setUniform(ssaoProg->getUniformLoc("toScreen"), toScreen);

		const static F32 posAttrib[] =
		{
			-1, -1, 0, 1,
			1, -1, 0, 1,
			-1, 1, 0, 1,
			1, 1, 0, 1
		};


		const static F32 texAttrib[] =
		{
			0, 1,
			1, 1,
			0, 0,
			1, 0
		};

		const static F32 texAttribINV[] =
		{
			0, 0,
			1, 0,
			0, 1,
			1, 1
		};

		gl->setAttrib(ssaoProg->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		gl->setAttrib(ssaoProg->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttribINV);

		gl->setUniform(ssaoProg->getUniformLoc("normalMap"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_NORMAL));

		gl->setUniform(ssaoProg->getUniformLoc("depthMap"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_DEPTH));

		gl->setUniform(ssaoProg->getUniformLoc("positionMap"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_POSITION));

		gl->setUniform(ssaoProg->getUniformLoc("diffuseMap"), 3);
		glActiveTexture(GL_TEXTURE2+1);
		glBindTexture(GL_TEXTURE_2D, m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_DIFFUSE));

		gl->setUniform(ssaoProg->getUniformLoc("shadowMapSampler"), 4);
		glActiveTexture(GL_TEXTURE2 + 2);
		glBindTexture(GL_TEXTURE_2D, m_depthMap);

		gl->setUniform(ssaoProg->getUniformLoc("numSSAOSamples"), m_ssaoKernel.getNumSamples());

		Mat4f toLightCamera, toLightClip;
		getLightMatrices(toLightCamera, toLightClip);

		gl->setUniform(ssaoProg->getUniformLoc("toLightClip"), toLightClip);
		gl->setUniform(ssaoProg->getUniformLoc("blurOut"), m_blurOut);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssaoKernel.getSSBOBuffer());

		glDisable(GL_DEPTH_TEST);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

		glEnable(GL_DEPTH_TEST);

	}

	void ForwardShadingScene::shadowMapPass(Window & wnd, const CameraControls & camera) {

		GLContext * gl = wnd.getGL();

		GLContext::Program * prog = gl->getProgram(m_shadowMapProgramName.c_str());
		prog->use();
		
		Mat4f worldToCamera, worldToClip;
		getLightMatrices(worldToCamera, worldToClip);

		//worldToCamera = camera.getWorldToCamera();
		//worldToClip = camera.getWorldToClip();
		
		gl->setUniform(prog->getUniformLoc("toScreen"), worldToClip);
		if (m_mesh) {
			glBindFragDataLocation(prog->getHandle(), 3, "depth");
			Mat4f toWorld;
			toWorld.setIdentity();

			gl->setUniform(prog->getUniformLoc("toWorld"), toWorld);
			gl->setUniform(prog->getUniformLoc("bounceEffect"), true);
			gl->setUniform(prog->getUniformLoc("bounceTime"), m_bTime);
			gl->setUniform(prog->getUniformLoc("bouncePeriod"), m_bPeriod);
			gl->setUniform(prog->getUniformLoc("bounceScale"), m_bScale);
			renderMeshCheap(gl, m_mesh.get(), prog);
		}

		if (m_renderDMTMesh && m_dmtMesh) {
			glBindFragDataLocation(prog->getHandle(), 3, "depth");
			Mat4f toWorld = Mat4f::translate(m_displacedMeshOff);
			gl->setUniform(prog->getUniformLoc("toWorld"), toWorld);
			Mat4f toCamera = worldToCamera*toWorld;
			Mat4f toScreen = worldToClip*toWorld;
			gl->setUniform(prog->getUniformLoc("normalToCamera"), toCamera.inverted().transposed());
			gl->setUniform(prog->getUniformLoc("toScreen"), toScreen);
			gl->setUniform(prog->getUniformLoc("bounceEffect"), false);
			m_dmtMesh->renderMeshCheap(gl, prog);

		}
		else if (!m_renderDMTMesh && m_dmParticleSystem) {
			m_dmParticleSystem->render(gl, worldToClip, m_gbuffer.get());
		}

		if (m_particleSystem) {
			m_particleSystem->render(gl, worldToClip, m_gbuffer.get());
		}

	}

	void ForwardShadingScene::getLightMatrices(Mat4f & toCamera, Mat4f & toScreen) {
		Mat4f cameraToClip = Mat4f::perspective(90.0f, 0.01f, 200.0f);
		toCamera.setRow(0, Vec4f(1.0f, 0.0f, 0.0f, 0.0f));
		toCamera.setRow(1, Vec4f(0.0f, 0.0f, -1.0f, 0.0f));
		toCamera.setRow(2, Vec4f(0.0f, 1.0f, 0.0f, -25.0f));
		toCamera.setRow(3, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
		
		toScreen = cameraToClip*toCamera;
	}

};