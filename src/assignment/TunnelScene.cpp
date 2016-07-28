#include "TunnelScene.h"
#include "gui/Image.hpp"
#include "surf.h"
#include "Globals.h"
#include "TessellationTestScene.h"
#include "SyncVars.h"
#include <fstream>
#include <algorithm>

namespace FW {

	TunnelScene::TunnelScene(GLContext * ctx, int width, int height, FBO * lastPass, CameraControls * camPtr)

		: mWidth(width),
		  mHeight(height),
		  mLastFBO(lastPass),
		  mCamPtr(camPtr)

	{
		GLuint depthTex = TEXTURE_POOL->request(TextureDescriptor(GL_DEPTH_COMPONENT32F, width, height, GL_DEPTH_COMPONENT, GL_FLOAT))->m_texture;
		GLuint colorTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;
		GLuint normalTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;
		GLuint velocityTex = TEXTURE_POOL->request(TextureDescriptor(GL_RG16F, width, height, GL_RG, GL_FLOAT))->m_texture;
		GLuint motionBlurTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;

		mGBuffer.reset(new FBO(depthTex));
		mGBuffer->attachTexture(0, colorTex);
		mGBuffer->attachTexture(1, normalTex);
		mGBuffer->attachTexture(2, velocityTex);

		mMotionBlurFBO.reset(new FBO(depthTex));
		mMotionBlurFBO->attachTexture(0, motionBlurTex);

		loadShaders(ctx);
		setupGLBuffers();
		generateTunnel();


		Image * tunnelDiffuseImg = FW::importImage("assets/tunnel/synthetic_metal_04_diffuse.png");
		mTunnelTexture = tunnelDiffuseImg->createGLTexture();

		Image * tunnelNormalImg = FW::importImage("assets/tunnel/synthetic_metal_04_normal.png");
		mTunnelNormalTexture = tunnelNormalImg->createGLTexture();

		Image * tunnelSpecularImg = FW::importImage("assets/tunnel/synthetic_metal_04_specular.png");
		mTunnelSpecularTexture = tunnelSpecularImg->createGLTexture();

		mMesher.reset(new Mesher(ctx, Vec4f(-20.0f, -20.0f, -20.0f, 0.625f), "shaders/tunnel/sdf.glsl"));
	}


	void TunnelScene::render(Window & wnd, const CameraControls & camera) {
		GLContext *gl = wnd.getGL();

		GLContext::Program * mesherProg = mMesher->getProgram();
		mesherProg->use();
		gl->setUniform(mesherProg->getUniformLoc("knob0"), Vec4f(FWSync::sdf1, FWSync::sdf2, FWSync::sdf3, FWSync::sdf4));
		gl->setUniform(mesherProg->getUniformLoc("knob1"), Vec4f(FWSync::sdf5, FWSync::sdf6, FWSync::sdf7, FWSync::sdf8));
		gl->setUniform(mesherProg->getUniformLoc("knob2"), Vec4f(FWSync::sdf9, FWSync::sdf10, FWSync::sdf11, FWSync::sdf12));
		gl->setUniform(mesherProg->getUniformLoc("knob3"), Vec4f(FWSync::sdf13, FWSync::sdf14, FWSync::sdf15, FWSync::sdf16));
		mMesher->update(gl);

		Vec3f cameraPosition = getCameraPosition(FWSync::trefoilTime);

		mCamPtr->setPosition(cameraPosition);
		mCamPtr->setForward(getCameraForward(FWSync::trefoilTime));
		//mCamPtr->setUp(getCameraUp());
		mCamPtr->setNear(2.0f);

		Mat4f toScreen = mCamPtr->getWorldToClip();
		Mat4f toWorld = Mat4f::scale(Vec3f(200.0f));

		mGBuffer->bind();

		glClearColor(0.05, 0.05, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//////////////////////////////////////////////////

		mTunnelProgram->use();
		
		Vec3f lightDiffuseColor = Vec3f(0.02, 0.1, 0.16);
		Vec3f lightSpecularColor = Vec3f(0.016, 0.05, 0.03);

		static Mat4f prevTunnelToScreen = toScreen*toWorld;

		Mat4f tunnelToScreen = toScreen*toWorld;

		gl->setUniform(mTunnelProgram->getUniformLoc("lightDiffuseColor"), lightDiffuseColor);
		gl->setUniform(mTunnelProgram->getUniformLoc("lightSpecularColor"), lightSpecularColor);
		gl->setUniform(mTunnelProgram->getUniformLoc("toScreen"), tunnelToScreen);
		gl->setUniform(mTunnelProgram->getUniformLoc("prevToScreen"), prevTunnelToScreen);
		gl->setUniform(mTunnelProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mTunnelProgram->getUniformLoc("normalToWorld"), toWorld.inverted().transposed());
		gl->setUniform(mTunnelProgram->getUniformLoc("cameraPosition"), cameraPosition);
		gl->setUniform(mTunnelProgram->getUniformLoc("diffuseTexture"), 0);
		gl->setUniform(mTunnelProgram->getUniformLoc("specularTexture"), 1);
		gl->setUniform(mTunnelProgram->getUniformLoc("normalTexture"), 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mTunnelTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mTunnelSpecularTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mTunnelNormalTexture);

		glBindVertexArray(mTunnelVAO);
		glDrawElements(GL_TRIANGLES, mTunnelNumIndices, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);



		/////////////////////////////////////

		mTunnelEngravingProgram->use();
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("lightDiffuseColor"), lightDiffuseColor);
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("lightSpecularColor"), lightSpecularColor);
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("toScreen"), tunnelToScreen);
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("prevToScreen"), prevTunnelToScreen);
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("normalToWorld"), toWorld.inverted().transposed());
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("cameraPosition"), cameraPosition);
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("diffuseTexture"), 0);
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("specularTexture"), 1);
		gl->setUniform(mTunnelEngravingProgram->getUniformLoc("normalTexture"), 2);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mTunnelTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mTunnelSpecularTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mTunnelNormalTexture);

		glBindVertexArray(mTunnelEngravingVAO);
		glDrawElementsInstanced(GL_TRIANGLES, mTunnelNumIndices, GL_UNSIGNED_INT, NULL, mNumEngravings);
		glBindVertexArray(0);
		////////////////////////////




		Mat4f objToWorld = Mat4f::translate(getCameraPosition(FWSync::trefoilTime + 0.05f)) * Mat4f::scale(Vec3f(10.0f));
		Mat4f objToScreen = toScreen * objToWorld;

		renderMeshObject(gl, objToScreen, objToWorld, objToWorld.inverted().transposed(), cameraPosition);

		mGBuffer->unbind();

		////////////////////////////

		mMotionBlurFBO->bind();

		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		mMotionBlurProgram->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGBuffer->getTexture(2));
		gl->setUniform(mMotionBlurProgram->getUniformLoc("velocityTex"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mGBuffer->getTexture(0));
		gl->setUniform(mMotionBlurProgram->getUniformLoc("colorTex"), 1);

		glBindVertexArray(mQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		glEnable(GL_DEPTH_TEST);
		mMotionBlurFBO->unbind();
		

		////////////////////////////

		mLastFBO->bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mCombineProgram->use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGBuffer->getTexture(0));
		glBindTexture(GL_TEXTURE_2D, mMotionBlurFBO->getTexture(0));
		gl->setUniform(mTunnelProgram->getUniformLoc("meshColorTex"), 0);

		glBindVertexArray(mQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);


		mLastFBO->unbind();

		prevTunnelToScreen = toScreen*toWorld;
	}

	void TunnelScene::generateTunnel()
	{
		std::vector<Vec3f> profilePoints = { Vec3f(0.000000,-0.250000, 0),
			Vec3f(0.000000, -0.250000, 0.0),
			Vec3f(0.550000, -0.550000, 0.0),
			Vec3f(0.250000, 0.000000, 0.0),
			Vec3f(0.550000, 0.550000, 0.0),
			Vec3f(-0.550000, 0.550000, 0.0),
			Vec3f(-0.250000, 0.000000, 0.0),
			Vec3f(0.000000, -0.250000, 0.0),
			Vec3f(0.550000, -0.550000, 0.0),
			Vec3f(0.250000, 0.000000, 0.0) };

		

		for (size_t i = 0; i < profilePoints.size(); ++i) profilePoints[i] *= 8.6f;

		Curve sweepCurve = evalTrefoilKnot(400.0);
		for (size_t i = 0; i < sweepCurve.size(); ++i) {
			sweepCurve[i].V *= 16.0f;
		}
		Curve profileCurve = evalBspline(profilePoints, 60.0, false, 0.0, 0.0);
		Surface tunnelSurface = makeGenCyl(profileCurve, sweepCurve);

		mTunnelNumIndices = tunnelSurface.VF.size() * 3;
		std::vector<MissileMeshVertex> vertexData(tunnelSurface.VV.size());

		for (size_t k = 0; k < vertexData.size(); ++k)
		{
			vertexData[k] = MissileMeshVertex(tunnelSurface.VV[k], tunnelSurface.VN[k].normalized(), tunnelSurface.VT[k].x, tunnelSurface.VT[k].y);
		}
		glGenVertexArrays(1, &mTunnelVAO);
		glBindVertexArray(mTunnelVAO);

		glGenBuffers(1, &mTunnelIBO);
		glGenBuffers(1, &mTunnelVBO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTunnelIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, tunnelSurface.VF.size() * sizeof(Vec3i), tunnelSurface.VF.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, mTunnelVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(MissileMeshVertex), vertexData.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), (char*)NULL + sizeof(Vec4f));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		for (size_t i = 0; i < profilePoints.size(); ++i) profilePoints[i] *= 1.0f;
		Curve cheapProfileCurve = evalBspline(profilePoints, 30.0, false, 0.0, 0.0);
		Curve smallProfile = evalCircle(0.05f, 20);
		Surface engraveSurface = makeGenCyl(smallProfile, cheapProfileCurve);
		mTunnelEngravingNumIndices = engraveSurface.VF.size() * 3;
		std::vector<MissileMeshVertex> vertexEngravingData(engraveSurface.VV.size());

		for (size_t k = 0; k < vertexEngravingData.size(); ++k)
		{
			vertexEngravingData[k] = MissileMeshVertex(engraveSurface.VV[k], engraveSurface.VN[k].normalized(), engraveSurface.VT[k].x, engraveSurface.VT[k].y);
		}

		mNumEngravings = 20;
		std::vector<Mat4f> engravingTransformations(mNumEngravings);
		size_t engravingStep = sweepCurve.size() / mNumEngravings;

		size_t idx = 0;
		for (size_t i = 0; i < mNumEngravings; ++i)
		{
			engravingTransformations[i] = sweepCurve[idx].getTransformation();
			idx += engravingStep;
		}
		

		glGenVertexArrays(1, &mTunnelEngravingVAO);
		glBindVertexArray(mTunnelEngravingVAO);

		glGenBuffers(1, &mTunnelEngravingIBO);
		glGenBuffers(1, &mTunnelEngravingVBO);
		glGenBuffers(1, &mTunnelEngravingTransformationVBO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mTunnelEngravingIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, engraveSurface.VF.size() * sizeof(Vec3i), engraveSurface.VF.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, mTunnelEngravingVBO);
		glBufferData(GL_ARRAY_BUFFER, vertexEngravingData.size() * sizeof(MissileMeshVertex), vertexEngravingData.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), (char*)NULL + sizeof(Vec4f));

		glBindBuffer(GL_ARRAY_BUFFER, mTunnelEngravingTransformationVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Mat4f) * engravingTransformations.size(), engravingTransformations.data(), GL_STATIC_DRAW);

		for (int i = 0; i < 4; ++i)
		{
			glEnableVertexAttribArray(2+i);
			glVertexAttribPointer(2+i, 4, GL_FLOAT, GL_FALSE, sizeof(Mat4f), (char*)NULL + sizeof(Vec4f)*i);
			glVertexAttribDivisor(2 + i, 1);
		}



		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void TunnelScene::setupGLBuffers() {
		const static F32 posAttrib[] =
		{
			-1, -1, 0, 1,
			1, -1, 0, 1,
			-1, 1, 0, 1,
			1, 1, 0, 1
		};

		glGenVertexArrays(1, &mQuadVAO);
		glGenBuffers(1, &mQuadVBO);
		glBindVertexArray(mQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 16, posAttrib, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void TunnelScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		default:
			break;
		}

	}

	void TunnelScene::loadShaders(GLContext * ctx) {

		if (mMesher) mMesher->loadShaders(ctx);

		mCombineProgram = loadShader(ctx, "shaders/tunnel/combine_vertex.glsl", "shaders/tunnel/combine_fragment.glsl", "tunnel_combine");
		mTunnelProgram = loadShader(ctx, "shaders/tunnel/tunnel_vert.glsl", "shaders/tunnel/tunnel_frag.glsl", "tunnel_render_program");
		mTunnelEngravingProgram = loadShader(ctx, "shaders/tunnel/tunnel_engraving_vert.glsl", "shaders/tunnel/tunnel_engraving_frag.glsl", "tunnel_engraving_render_program");
		mDisplayProgram = loadShader(ctx, "shaders/common/display_vertex.glsl", "shaders/common/display_fragment.glsl", "tunnel_display_program");
		mMeshProgram = loadShader(ctx, "shaders/tunnel/mesh_object_vert.glsl", "shaders/tunnel/mesh_object_frag.glsl", "tunnel_mesh_render");
		mMotionBlurProgram = loadShader(ctx, "shaders/common/display_vertex.glsl", "shaders/tunnel/motion_blur_frag.glsl", "motion_blur_tunnel");
	}


	void TunnelScene::activate(Window & wnd, CommonControls & controls) {

		updateGUI(wnd, controls);

	}

	void TunnelScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		// remove toggles
		controls.removeControl(&actionExt);
	}

	void TunnelScene::updateGUI(Window & wnd, CommonControls & controls) {

		cleanUpGUI(wnd, controls);

		controls.addSeparator();

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

	Vec3f TunnelScene::getCameraPosition(float t) {

		float tmp = (2.0f + cosf(3.0f*t));
		float xCoord = tmp * cosf(2.0f * t);
		float yCoord = tmp * sinf(2.0f * t);
		float zCoord = sinf(3.0f * t);

		return 3200.0f*Vec3f(xCoord, yCoord, zCoord);

	}

	Vec3f TunnelScene::getCameraForward(float t) {

		float tNext = t+0.1f;
		float tmp = (2.0f + cosf(3.0f*tNext));
		float xCoord = tmp * cosf(2.0f * tNext);
		float yCoord = tmp * sinf(2.0f * tNext);
		float zCoord = sinf(3.0f * tNext);

		return normalize(3200.0f*Vec3f(xCoord, yCoord, zCoord) - getCameraPosition(t));

	}

	Vec3f TunnelScene::getCameraUp(float t)
	{
		t += 0.1f;
		return (3200.0f*Vec3f(
			0.5f * (-cosf(t) - 16.0f * cosf(2.0f * t) - 25.0f * cosf(5.0f * t)),
			0.5f * (sinf(t) - 16.0f * sinf(2.0f * t) - 25.0f * sinf(5.0f * t)),
			-9.0f * sinf(3.0f * t))).normalized();
	}

	void TunnelScene::renderMeshObject(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition)
	{
		mMeshProgram->use();

		static Mat4f prevMeshToScreen = toScreen;

		gl->setUniform(mMeshProgram->getUniformLoc("prevToScreen"), prevMeshToScreen);
		gl->setUniform(mMeshProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mMeshProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mMeshProgram->getUniformLoc("normalToWorld"), normalToWorld);
		gl->setUniform(mMeshProgram->getUniformLoc("cameraPos"), cameraPosition);
		gl->setUniform(mMeshProgram->getUniformLoc("lightColor"), Vec3f(0.35f, 0.267f, 0.5f));

		/*gl->setUniform(mMeshObjectProgram->getUniformLoc("envMapTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBoxTex);*/

		glBindBuffer(GL_ARRAY_BUFFER, mMesher->getTriangleVBO());

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), 0);

		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), (GLvoid*)((char*)NULL + sizeof(float) * 8));
		int numTrigs = mMesher->getNumTriangles();
		if (0<numTrigs)glDrawArrays(GL_TRIANGLES, 0, numTrigs);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		prevMeshToScreen = toScreen;

	}

};