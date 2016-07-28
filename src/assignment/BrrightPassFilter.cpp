#include "BrightPassFilter.h"

namespace FW {

	BrightPassFilter::BrightPassFilter(GLContext * ctx, Vec2i size) :
		m_size(size),
		m_programName("bright_pass_filter"),
		m_displayProgramName("display_image")
	{
		loadShaders(ctx);

		// then we have the possibility to render to a texture at some point
		glGenFramebuffers(1, &m_frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

		glGenRenderbuffers(1, &m_renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, size.x, size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_renderBuffer);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


	}


	void BrightPassFilter::loadShaders(GLContext * ctx) {

		const char vertexShader[] = "shaders/raymarch/quad.vertex";
		const char declShader[] = "shaders/raymarch/decl.glsl"; // header stuff, declerations
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms
		const char brightPassShader[] = "shaders/postprocess/brightpass.glsl"; // gaussian uniforms
		const char displayShader[] = "shaders/postprocess/display.glsl"; // gaussian uniforms
		loadShader(ctx, { vertexShader },
		{
			declShader,
			knobsShader,
			brightPassShader
		}, m_programName);

		loadShader(ctx, { vertexShader },
		{
			declShader,
			knobsShader,
			displayShader
		}, m_displayProgramName);
	}


	void BrightPassFilter::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
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

	void BrightPassFilter::render(Window & wnd, const CameraControls & camera) {

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

		GLContext * ctx = wnd.getGL();

		GLContext::Program * brightPassProgram = ctx->getProgram(m_programName.c_str());
		GLContext::Program * displayProgram = ctx->getProgram(m_displayProgramName.c_str());
		updateUniforms(wnd, camera);

		brightPassProgram->use();

		ctx->setUniform(brightPassProgram->getUniformLoc("inImage"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		ctx->setAttrib(brightPassProgram->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(brightPassProgram->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttribINV);

		// render y pass to texture

		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_resultTexture, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);



		displayProgram->use();
		ctx->setAttrib(displayProgram->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(displayProgram->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttrib);

		ctx->setUniform(displayProgram->getUniformLoc("inImage"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_resultTexture);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	}

	void BrightPassFilter::updateUniforms(Window & wnd, const CameraControls & camera) {
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
	}

	void BrightPassFilter::activate(Window & wnd, CommonControls & controls) {
		updateGUI(wnd, controls);
	}

	void BrightPassFilter::updateGUI(Window & wnd, CommonControls & controls) {
		cleanUpGUI(wnd, controls);

		controls.addButton((S32*)&actionExt, (S32)Action::Action_SaveTexture, FW_KEY_NONE, "Save texture");

		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob1
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob2
			std::make_pair(Vec4f(-1.0f), Vec4f(1.0f)), // knob3
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob4
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob5
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob6
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob7
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob8
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob9
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob10
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

	void BrightPassFilter::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		controls.removeControl(&actionExt);
	}


	void BrightPassFilter::saveTexture() {



		glBindTexture(GL_TEXTURE_2D, m_texture);

		Image image(m_size, ImageFormat::R8_G8_B8_A8);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)image.getMutablePtr());

		glBindTexture(GL_TEXTURE_2D, 0);

		exportImage((m_programName + ".png").c_str(), &image);
	}

};