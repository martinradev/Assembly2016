#include "BloomFilter.h"

namespace FW {

	BloomFilter::BloomFilter(GLContext * ctx, Vec2i size, GLuint texture) :
		m_size(size),
		m_texture(texture),
		m_csProgramName("bloom_filter"),
		m_displayProgramName("display_image")
	{
		loadShaders(ctx);

		m_intKnob.x = 3;
		m_intKnob.y = 3;

	}


	void BloomFilter::loadShaders(GLContext * ctx) {

		const char vertexShader[] = "shaders/raymarch/quad.vertex";
		const char declShader[] = "shaders/raymarch/decl.glsl"; // header stuff, declerations
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms
		const char gaussianCSShader[] = "shaders/postprocess/gaussian_cs.glsl"; // gaussian uniforms
		const char displayShader[] = "shaders/postprocess/display.glsl"; // gaussian uniforms
		loadShader(ctx,
		{
			declShader,
			knobsShader,
			gaussianCSShader
		}, m_csProgramName);

		loadShader(ctx, { vertexShader },
		{
			declShader,
			knobsShader,
			displayShader
		}, m_displayProgramName);

	}


	void BloomFilter::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
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

	void BloomFilter::render(Window & wnd, const CameraControls & camera) {

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

		GLContext::Program * gaussianProgram = ctx->getProgram(m_csProgramName.c_str());

		GLContext::Program * displayProgram = ctx->getProgram(m_displayProgramName.c_str());

		updateUniforms(wnd, camera);

		gaussianProgram->use();

		glBindImageTexture(0, m_originalTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, m_texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(2, m_blurredTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(3, m_resultTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	
		int threadBlocksX = ceil(m_size.x / 16.0f);
		int threadBlocksY = ceil(m_size.y / 16.0f);

		glDispatchCompute(threadBlocksX, threadBlocksY, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
		glBindImageTexture(2, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glBindImageTexture(3, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		displayProgram->use();

		ctx->setAttrib(displayProgram->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(displayProgram->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttrib);

		ctx->setUniform(displayProgram->getUniformLoc("inImage"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_resultTexture);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// first render to ping texture

		/*
		// stupid but we have to flip
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingTextures[1], 0);
		displayProgram->use();
		ctx->setAttrib(displayProgram->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(displayProgram->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttrib);

		ctx->setUniform(displayProgram->getUniformLoc("inImage"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pingTextures[2]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		sumProgram->use();

		ctx->setUniform(sumProgram->getUniformLoc("lastPass"), true);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingTextures[2], 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pingTextures[1]);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_pingTextures[3]);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// final pass to combine images

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		
		displayProgram->use();
		ctx->setAttrib(displayProgram->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(displayProgram->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttribINV);

		ctx->setUniform(displayProgram->getUniformLoc("inImage"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_pingTextures[2]);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);*/
	}

	void BloomFilter::updateUniforms(Window & wnd, const CameraControls & camera) {
		GLContext * ctx = wnd.getGL();

		// update ray march program uniforms
		GLContext::Program * prog = ctx->getProgram(m_csProgramName.c_str());

		prog->use();

		ctx->setUniform(prog->getUniformLoc("knob1"), m_knobs[0]);
		ctx->setUniform(prog->getUniformLoc("knob2"), m_knobs[1]);
		ctx->setUniform(prog->getUniformLoc("knob3"), m_knobs[2]);
		ctx->setUniform(prog->getUniformLoc("knob4"), m_knobs[3]);
		ctx->setUniform(prog->getUniformLoc("knob5"), m_knobs[4]);
		ctx->setUniform(prog->getUniformLoc("knob6"), m_knobs[5]);
		ctx->setUniform(prog->getUniformLoc("knob7"), m_knobs[6]);
		ctx->setUniform(prog->getUniformLoc("knob8"), m_knobs[7]);

		ctx->setUniform(prog->getUniformLoc("intKnob"), m_intKnob);

		ctx->setUniform(prog->getUniformLoc("screenSize"), m_size);
	}

	void BloomFilter::activate(Window & wnd, CommonControls & controls) {
		updateGUI(wnd, controls);
	}

	void BloomFilter::updateGUI(Window & wnd, CommonControls & controls) {
		cleanUpGUI(wnd, controls);

		controls.addButton((S32*)&actionExt, (S32)Action::Action_SaveTexture, FW_KEY_NONE, "Save texture");

		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob1
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

		if (activeKnob == 8) {
			// int knob
			String xStr = sprintf("Knob%u.x = %%d", activeKnob + 1);
			String yStr = sprintf("Knob%u.y = %%d", activeKnob + 1);
			String zStr = sprintf("Knob%u.z = %%d", activeKnob + 1);
			String wStr = sprintf("Knob%u.w = %%d", activeKnob + 1);

			controls.beginSliderStack();
			controls.addSlider(&m_intKnob.x, 0, 10, false, FW_KEY_NONE, FW_KEY_NONE, xStr);
			controls.addSlider(&m_intKnob.y, 0, 120, false, FW_KEY_NONE, FW_KEY_NONE, yStr);
			controls.addSlider(&m_intKnob.z, 0, 10, false, FW_KEY_NONE, FW_KEY_NONE, zStr);
			controls.addSlider(&m_intKnob.w, 0, 10, false, FW_KEY_NONE, FW_KEY_NONE, wStr);
			controls.endSliderStack();
		}
		else {
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

		

	}

	void BloomFilter::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		controls.removeControl(&m_intKnob.x);
		controls.removeControl(&m_intKnob.y);
		controls.removeControl(&m_intKnob.z);
		controls.removeControl(&m_intKnob.w);

		controls.removeControl(&actionExt);
	}

	void BloomFilter::saveTexture() {

		

		glBindTexture(GL_TEXTURE_2D, m_texture);

		Image image(m_size, ImageFormat::R8_G8_B8_A8);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)image.getMutablePtr());

		glBindTexture(GL_TEXTURE_2D, 0);

		exportImage((m_csProgramName + ".png").c_str(), &image);
	}

};