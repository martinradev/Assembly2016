#include "ToneMapper.h"

#include "base/Main.hpp"

namespace FW {

	ToneMapper::ToneMapper(GLContext * ctx, Vec2i size) :
		m_size(size),
		m_programName("tonemapper")
	{
		loadShaders(ctx);

		//Image * img = importImage("assets/FilmLut.tga");
		//m_paletteTexture = img->createGLTexture();
		//delete img;
	}


	void ToneMapper::loadShaders(GLContext * ctx) {

		const char vertexShader[] = "shaders/raymarch/quad.vertex";
		//const char declShader[] = "shaders/raymarch/decl.glsl"; // header stuff, declerations
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms
		const char tonemapperShader[] = "shaders/postprocess/tonemapper.glsl"; // tonemapper uniforms
		//const char displayShader[] = "shaders/postprocess/display.glsl"; // display uniforms
		loadShader(ctx,  vertexShader ,
		
			tonemapperShader
		, m_programName);

	}


	void ToneMapper::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
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

	void ToneMapper::render(Window & wnd, const CameraControls & camera) {

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

		GLContext::Program * tonemapperProgram = ctx->getProgram(m_programName.c_str());

		tonemapperProgram->use();
		ctx->setUniform(tonemapperProgram->getUniformLoc("blurOut"), FWSync::blurOut);
		/*ctx->setUniform(tonemapperProgram->getUniformLoc("inImage"), 0);
		ctx->setUniform(tonemapperProgram->getUniformLoc("filmLutImage"), 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);*/

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mBloomTexture);
		
		ctx->setUniform(tonemapperProgram->getUniformLoc("inImage"), 0);
		ctx->setUniform(tonemapperProgram->getUniformLoc("bloomImage"), 1);
		ctx->setUniform(tonemapperProgram->getUniformLoc("exposure"), mExposure);
		ctx->setAttrib(tonemapperProgram->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(tonemapperProgram->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttribINV);

		// render y pass to texture

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		

	}

	void ToneMapper::updateUniforms(Window & wnd, const CameraControls & camera) {

	}

	void ToneMapper::activate(Window & wnd, CommonControls & controls) {
		updateGUI(wnd, controls);
	}

	void ToneMapper::updateGUI(Window & wnd, CommonControls & controls) {
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

	void ToneMapper::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		controls.removeControl(&actionExt);
	}


	void ToneMapper::saveTexture() {



		glBindTexture(GL_TEXTURE_2D, m_texture);

		Image image(m_size, ImageFormat::R8_G8_B8_A8);

		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)image.getMutablePtr());

		glBindTexture(GL_TEXTURE_2D, 0);

		exportImage((m_programName + ".png").c_str(), &image);
	}

};