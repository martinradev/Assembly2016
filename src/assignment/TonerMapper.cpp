#include "ToneMapper.h"

#include "base/Main.hpp"
#include "Globals.h"
namespace FW {

	ToneMapper::ToneMapper(GLContext * ctx, Vec2i size) :
		m_size(size),
		m_programName("tonemapper")
	{
		loadShaders(ctx);

		//Image * img = importImage("assets/FilmLut.tga");
		//m_paletteTexture = img->createGLTexture();
		//delete img;

		for (int i = 1; i <= 6; ++i)
		{
			std::string fileName = "assets/overlays/" + std::to_string(i) + ".png";
			FW::Image * tmpImg = FW::importImage(String(fileName.c_str()));
			mOverlays[i-1] = tmpImg->createGLTexture();
			delete tmpImg;
		}

		glGenTextures(6, mColorGradingTexture);

		for (int i = 0; i < 6; ++i)
		{
			std::string name = "assets/color_grading/" + std::to_string(i) + ".png";
			FW::Image * colorLutImage = importImage(name.c_str());


			ImageFormat::ID formatID = colorLutImage->getFormat().getGLFormat();
			const ImageFormat::StaticFormat* sf = ImageFormat(formatID).getStaticFormat();
			FW_ASSERT(sf);

			
			glBindTexture(GL_TEXTURE_3D, mColorGradingTexture[i]);
			glTexParameteri(GL_TEXTURE_3D, GL_GENERATE_MIPMAP, false);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


			glTexImage3D(GL_TEXTURE_3D, 0, sf->glInternalFormat,
				32, 32, 32,
				0, sf->glFormat, sf->glType, colorLutImage->getPtr());




			delete colorLutImage;
		}
		

		GLContext::checkErrors();
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

		Vec2i sz = wnd.getSize();

		glViewport(0, 0, sz.x, sz.y);

		GLContext * ctx = wnd.getGL();

		GLContext::Program * tonemapperProgram = ctx->getProgram(m_programName.c_str());

		tonemapperProgram->use();
		ctx->setUniform(tonemapperProgram->getUniformLoc("blurOut"), FWSync::blurOut);
		ctx->setUniform(tonemapperProgram->getUniformLoc("fadeMix"), FWSync::fadeMix);
		ctx->setUniform(tonemapperProgram->getUniformLoc("fadeColor"), FWSync::fadeColor);
		ctx->setUniform(tonemapperProgram->getUniformLoc("time"), GLOBAL_TIMER.getElapsed());
		/*ctx->setUniform(tonemapperProgram->getUniformLoc("inImage"), 0);
		ctx->setUniform(tonemapperProgram->getUniformLoc("filmLutImage"), 1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);*/

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mBloomTexture);

		int overlayIndexINT = int(FWSync::overlayIndex);
		if (overlayIndexINT >= 0 && overlayIndexINT < 6)
		{
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, mOverlays[overlayIndexINT]);
			ctx->setUniform(tonemapperProgram->getUniformLoc("overlay"), 2);
			ctx->setUniform(tonemapperProgram->getUniformLoc("useOverlay"), true);
			ctx->setUniform(tonemapperProgram->getUniformLoc("overlayAlpha"), FWSync::overlayAlpha);
		}
		else {
			ctx->setUniform(tonemapperProgram->getUniformLoc("useOverlay"), false);
		}

		int colorGradingIndex = floor(FWSync::colorGradingIndex);
		colorGradingIndex = clamp(colorGradingIndex, 0, 5);

		glActiveTexture(GL_TEXTURE2+1);
		glBindTexture(GL_TEXTURE_3D, mColorGradingTexture[colorGradingIndex]);
		ctx->setUniform(tonemapperProgram->getUniformLoc("colorGradingLUT"), 3);
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