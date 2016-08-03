#include "FinalScene.h"
#include "gui/Image.hpp"
#include "Util.h"
#include "SyncVars.h"
#include "Globals.h"
#include "Samplers.h"
#include <fstream>
#include <algorithm>

namespace FW {

	FinalScene::FinalScene(GLContext * ctx, unsigned width, unsigned height, FBO * lastFBO, CameraControls * camPtr, TunnelScene * tunnelScene, TessellationTestScene * tessScene, ParticleLogoSDF * logoScene)
		:
		mWidth(width),
		mHeight(height),
		mCamPtr(camPtr),
		mLastFBO(lastFBO),
		mTunnelScene(tunnelScene),
		mTessScene(tessScene),
		mLogoScene(logoScene)
	{

		loadShaders(ctx);

		mBackgroundFBO = new FBO(mTessScene->m_gbuffer->getRealDepthMap());

		mBackgroundFBO->attachTexture(0, mTessScene->m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_DIFFUSE));

		m_knobs[0] = Vec4f(0.658, 0.581, 0.644, 0.0);
	}


	void FinalScene::render(Window & wnd, const CameraControls & camera) {

		GLContext * gl = wnd.getGL();

		mCamPtr->setFar(60000.0f);

		lightPass(gl);

		Vec3f dir = getCameraDirection();
		Vec3f pos = 1.2f*getCameraPosition();
		mCamPtr->setPosition(pos);
		mCamPtr->setForward(dir);

		Mat4f toLight, toLightClip;
		getLightMatrix(toLight, toLightClip);
		Mat4f toWorld = Mat4f::translate(Vec3f(0, 3500.0f, 0.0)) * rot(Vec3f(0,FWSync::knotRotate,0)) * Mat4f::scale(Vec3f(200.0f));
		Mat4f toScreen = mCamPtr->getWorldToClip();
		Mat4f meshToScreen = toScreen*toWorld;
		Vec3f cameraPosition = mCamPtr->getPosition();

		/*mBackgroundFBO->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		

		glDisable(GL_DEPTH_TEST);

		mLogoScene->mBackgroundProgram->use();

		glBindVertexArray(mTessScene->m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		glEnable(GL_DEPTH_TEST);

		mBackgroundFBO->unbind();*/

		GLuint fbo = mTessScene->m_gbuffer->getFBO();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mKnotRenderProgram->use();

		gl->setUniform(mKnotRenderProgram->getUniformLoc("toScreen"), meshToScreen);
		gl->setUniform(mKnotRenderProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mKnotRenderProgram->getUniformLoc("normalToWorld"), toWorld.inverted().transposed());
		gl->setUniform(mKnotRenderProgram->getUniformLoc("cameraPosition"), cameraPosition);

		gl->setUniform(mKnotRenderProgram->getUniformLoc("lightPos"), Vec3f(0.0f, 50000.0f, 0.0f));
		gl->setUniform(mKnotRenderProgram->getUniformLoc("lightColor"), m_knobs[0].getXYZ());
		gl->setUniform(mKnotRenderProgram->getUniformLoc("lightDirection"), Vec3f(0, -1, 0));

		gl->setUniform(mKnotRenderProgram->getUniformLoc("screenSize"), Vec2f(mWidth,mHeight));
		gl->setUniform(mKnotRenderProgram->getUniformLoc("toLightScreen"), toLightClip);
		gl->setUniform(mKnotRenderProgram->getUniformLoc("depthMap"), 3);

		glActiveTexture(GL_TEXTURE2+1);
		glBindTexture(GL_TEXTURE_2D, mTessScene->m_terrainLightFBO->getTexture(0));

		gl->setUniform(mKnotRenderProgram->getUniformLoc("diffuseTex"), 0);
		gl->setUniform(mKnotRenderProgram->getUniformLoc("specularTex"), 1);
		gl->setUniform(mKnotRenderProgram->getUniformLoc("normalTex"), 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mTunnelScene->mTunnelTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mTunnelScene->mTunnelSpecularTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mTunnelScene->mTunnelNormalTexture);

		glBindVertexArray(mTunnelScene->mTunnelVAO);
		glDrawElements(GL_TRIANGLES, mTunnelScene->mTunnelNumIndices, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);


		mTessScene->m_godrayFBO->bind();

		glViewport(0, 0, mWidth, mHeight);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		// render water plane white


		mGodrayRaymarchProgram->use();

		glBindFragDataLocation(mGodrayRaymarchProgram->getHandle(), 0, "outColor");

		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("camPos"), cameraPosition);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("lightPos"), Vec3f(0,50000,0));
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("lightDir"), Vec3f(0,-1,0));
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("toLightClip"), toLightClip);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("positionMap"), 0);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("lightColor"), m_knobs[0].getXYZ());
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("toInvClip"), toScreen.inverted());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mTessScene->m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_POSITION));

		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("depthMap"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mTessScene->m_terrainLightFBO->getTexture(0));

		glBindVertexArray(mTessScene->m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		mTessScene->m_godrayFBO->unbind();

		mTessScene->m_gaussianFilter->process(gl, mTessScene->m_godrayFBO->getTexture(0), mTessScene->mGodrayBlurTex, 6);

	
		mLastFBO->bind();

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mCombineProgram->use();

		gl->setUniform(mCombineProgram->getUniformLoc("meshTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mTessScene->m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_DIFFUSE));

		gl->setUniform(mCombineProgram->getUniformLoc("godrayBlurTex"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mTessScene->mGodrayBlurTex);
		//glBindTexture(GL_TEXTURE_2D, mTessScene->mGodrayBlurTex);
		//glBindTexture(GL_TEXTURE_2D, mTessScene->m_terrainLightFBO->getTexture(0));

		glBindVertexArray(mTessScene->m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		mLastFBO->unbind();

	}

	void FinalScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		default:
			break;
		}

	}

	void FinalScene::loadShaders(GLContext * ctx) {

		const char displayVertex[] = "shaders/common/display_vertex.glsl";
		const char displayFragment[] = "shaders/common/display_fragment.glsl";

		mDisplayProgram = loadShader(ctx, displayVertex, displayFragment, "final_display");


		const char godrayRaymarchFragment[] = "shaders/final/godray_raymarch_fragment.glsl";
		mGodrayRaymarchProgram = loadShader(ctx, displayVertex, godrayRaymarchFragment, "godray_raymarch_final");


		mKnotRenderProgram = loadShader(ctx, "shaders/final/mesh_curve_vert.glsl", "shaders/final/mesh_curve_frag.glsl", "final_mesh_curve");

		mKnotLightProgram = loadShader(ctx, "shaders/final/mesh_curve_light_vert.glsl", "shaders/final/mesh_curve_light_frag.glsl", "final_mesh_curve_light");

		mCombineProgram = loadShader(ctx, displayVertex, "shaders/final/combine_fragment.glsl", "combine_program_final");

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

		mKnotRenderProgram->use();
		ctx->setUniformArray(mKnotRenderProgram->getUniformLoc("ldSamples"), 36, (Vec2f*)ldSamples);
	}

	void FinalScene::activate(Window & wnd, CommonControls & controls) {

		updateGUI(wnd, controls);

	}

	void FinalScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}
	}

	void FinalScene::updateGUI(Window & wnd, CommonControls & controls) {

		cleanUpGUI(wnd, controls);


		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob1
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob2
			std::make_pair(Vec4f(-1.0f), Vec4f(1.0f)), // knob3
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

	void FinalScene::lightPass(GLContext * gl)
	{
		mTessScene->m_terrainLightFBO->bind();

		glViewport(0, 0, mWidth, mHeight);

		Mat4f toLight, toLightClip;

		getLightMatrix(toLight, toLightClip);

		Mat4f toWorld = Mat4f::translate(Vec3f(0, 3500.0f, 0.0)) * rot(Vec3f(0,FWSync::knotRotate, 0)) * Mat4f::scale(Vec3f(200.0f));

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mKnotLightProgram->use();

		gl->setUniform(mKnotLightProgram->getUniformLoc("toScreen"), toLightClip * toWorld);
		gl->setUniform(mKnotLightProgram->getUniformLoc("toWorld"), toWorld);
		glBindVertexArray(mTunnelScene->mTunnelVAO);
		glDrawElements(GL_TRIANGLES, mTunnelScene->mTunnelNumIndices, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);

		mTessScene->m_terrainLightFBO->unbind();
	}

	void FinalScene::getLightMatrix(Mat4f & toLight, Mat4f & toLightClip)
	{
		Vec3f z = Vec3f(-0.472f, 0.986f, 0.657f).normalized();
		Vec3f y(0, 1, 0);
		Vec3f x = cross(y, z).normalized();
		y = cross(z, x).normalized();

		static const Mat4f perspective = Mat4f::perspective(140.0f, 1000.0f, 82000.0f, 1.0f);

		toLight.setRow(0, Vec4f(1.0f, 0.0f, 0.0f, 0.0f));
		toLight.setRow(1, Vec4f(0.0f, 0.0f, 1.0f, -0.0f));
		toLight.setRow(2, Vec4f(0.0f, 1.0f, 0.0f, -50000.0));
		toLight.setRow(3, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));

		toLightClip = perspective * toLight;
	}

	Vec3f FinalScene::getCameraPosition()
	{
		return Vec3f(0, 3500, 25000);
	}
	Vec3f FinalScene::getCameraDirection()
	{
		return (Vec3f(0,6000,0) - getCameraPosition()).normalized();
	}

};