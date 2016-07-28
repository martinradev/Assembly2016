#include "ProceduralTextureScene.h"

namespace FW {

	ProceduralTextureScene::ProceduralTextureScene(const std::string & sceneShader, GLContext * ctx, Vec2i size, GLuint texture) :
		m_sceneShader(sceneShader),
		m_size(size),
		m_texture(texture)
	{
		loadShaders(ctx);

		if (m_texture != 0) {
			// then we have the possibility to render to a texture at some point
			glGenFramebuffers(1, &m_frameBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

			glGenRenderbuffers(1, &m_renderBuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, size.x, size.y);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBuffer);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	}


	void ProceduralTextureScene::loadShaders(GLContext * ctx) {

		const char vertexShader[] = "shaders/raymarch/quad.vertex";
		const char declShader[] = "shaders/raymarch/decl.glsl"; // header stuff, declerations
		const char hgSdfShader[] = "shaders/raymarch/hg_sdf.glsl"; // mercury's sdf library
		const char noiseShader[] = "shaders/raymarch/noise.glsl"; // raymarch utilities
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms

		loadShader(ctx, { vertexShader },
		{
			declShader,
			noiseShader,
			hgSdfShader,
			knobsShader,
			m_sceneShader
		}, m_sceneShader);

	}

	void ProceduralTextureScene::renderToTexture(Window & wnd, const CameraControls & camera) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

		render(wnd, camera);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	/*void ProceduralTextureScene::generateTextures() {
	GLint oldTex = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTex);
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, false);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, sf->glInternalFormat,
	img->getSize().x, img->getSize().y,
	0, sf->glFormat, sf->glType, img->getPtr());
	glBindTexture(GL_TEXTURE_2D, oldTex);
	}*/

	void ProceduralTextureScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		case Action::Action_SaveTexture:
			saveTexture();
			break;
		default:
			break;
		}

	}

	void ProceduralTextureScene::render(Window & wnd, const CameraControls & camera) {

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

		GLContext * ctx = wnd.getGL();

		GLContext::Program * prog = ctx->getProgram(m_sceneShader.c_str());

		updateUniforms(wnd, camera);

		prog->use();

		ctx->setAttrib(prog->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(prog->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttrib);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	}

	void ProceduralTextureScene::updateUniforms(Window & wnd, const CameraControls & camera) {
		GLContext * ctx = wnd.getGL();

		// update ray march program uniforms
		GLContext::Program * prog = ctx->getProgram(m_sceneShader.c_str());

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
	}

	void ProceduralTextureScene::activate(Window & wnd, CommonControls & controls) {
		updateGUI(wnd, controls);
	}

	void ProceduralTextureScene::updateGUI(Window & wnd, CommonControls & controls) {
		cleanUpGUI(wnd, controls);

		controls.addButton((S32*)&actionExt, (S32)Action::Action_SaveTexture, FW_KEY_NONE, "Save texture");

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

	void ProceduralTextureScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		controls.removeControl(&actionExt);
	}

	void ProceduralTextureScene::saveTexture() {



		glBindTexture(GL_TEXTURE_2D, m_texture);

		Image image(m_size, ImageFormat::R8_G8_B8_A8);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)image.getMutablePtr());

		glBindTexture(GL_TEXTURE_2D, 0);

		exportImage((m_sceneShader + ".png").c_str(), &image);
	}

};