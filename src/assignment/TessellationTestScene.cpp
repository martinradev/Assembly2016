#include "TessellationTestScene.h"

#include "MeshRenderHelper.h"
#include "base/Math.hpp"
#include "Globals.h"
#include "SyncVars.h"
#include "IntersectionTest.h"
#include "Util.h"
#include "Samplers.h"
#include "ParallelSort.h"
#include "Spline.h"
#include <fstream>
#include <iostream>
#include <unordered_map>

namespace FW {

	TessellationTestScene::TessellationTestScene(
		CameraControls * camPtr,
		const std::string & sceneShader,
		GLContext * ctx,
		unsigned width, unsigned height,
		FBO * lastPass) :

		m_sceneShader(sceneShader),
		m_programName(sceneShader),
		m_displayProgramName("tessDisplayProgram"),
		m_lightRenderProgramName("tessLightRender"),
		m_waterRenderProgramName("tessWaterRender"),
		m_combineProgramName("tessCombine"),
		m_meshRenderProgramName("tessStaticMeshRender"),
		m_meshRenderLightProgramName("tessStaticMeshLightRender"),
		m_renderCurveNoTessProgramName("renderCurveNoTessProgram"),
		mMissileProgramName("missileProgram"),
		mMissileLightProgramName("mMissileLightProgramName"),
		mCityRenderProgramName("mCityProgramName"),
		mCityLightProgramName("mCityLightProgramName"),
		mAuthorRenderProgramName("mCityAuthorLogoProgramName"),
		m_planePatchVAO(0),
		m_planePatchVBO(0),
		m_numPatchesX(40),
		m_sideHalfLength(60.0f),
		m_ProcessTexSize(width, height),
		mCamPtr(camPtr),
		mDebugRenderCurve(false),
		mSelectedControlPointIndex(-1),
		mSelectedCurveIndex(-1),
		mWndWidth(width),
		mWndHeight(height),
		mSelectedCurveSlider(0),
		mLastPassFBO(lastPass),
		mUnderwaterParticlesRenderProgramName("underwater_particle_render"),
		mUnderwaterParticlesUpdateProgramName("underwater_particle_update"),
		mWaterSurfaceProgramName("water_surface_render_tess"),
		mGodrayBlurProgramName("godray_blur_name_tess"),
		mGodrayRaymarchProgramName("godray_raymarch_tess"),
		mSubmarineParticleProgramName("render_submarine"),
		mSubmarineParticleLightProgramName("render_light_submarine"),
		mSubmarineSDFMoveProgramName("submarine_sdf_move"),
		mRocketMeshNumVertices(0),
		mRocketRenderNumVertices(0)
	{
		loadShaders(ctx);

		m_gbuffer.reset(new GBuffer(width, height));

		GLuint fboDepthTex = TEXTURE_POOL->request(TextureDescriptor(GL_DEPTH_COMPONENT32F, m_ProcessTexSize.x, m_ProcessTexSize.y, GL_DEPTH_COMPONENT, GL_FLOAT))->m_texture;

		m_terrainLightFBO.reset(new FBO(fboDepthTex));
		m_ribbonFBO.reset(new FBO(m_gbuffer->getRealDepthMap()));
		m_godrayFBO.reset(new FBO(m_gbuffer->getRealDepthMap()));
		m_gaussianFilter.reset(new GaussianFilter(ctx, m_ProcessTexSize));

		GLuint terrainTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_ProcessTexSize.x, m_ProcessTexSize.y, GL_RGBA, GL_FLOAT))->m_texture;
		GLuint ribbonTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_ProcessTexSize.x, m_ProcessTexSize.y, GL_RGBA, GL_FLOAT))->m_texture;
		mWaterSurfaceTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_ProcessTexSize.x, m_ProcessTexSize.y, GL_RGBA, GL_FLOAT))->m_texture;
		mGodrayBlurTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_ProcessTexSize.x, m_ProcessTexSize.y, GL_RGBA, GL_FLOAT))->m_texture;

		m_blurResultTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_ProcessTexSize.x, m_ProcessTexSize.y, GL_RGBA, GL_FLOAT))->m_texture;

		mGodrayFBO.reset(new FBO(fboDepthTex));
		GLuint godrayBlurTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, m_ProcessTexSize.x, m_ProcessTexSize.y, GL_RGBA, GL_FLOAT))->m_texture;
		mGodrayFBO->attachTexture(0, godrayBlurTex);

		glBindTexture(GL_TEXTURE_2D, terrainTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, ribbonTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, m_blurResultTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, mWaterSurfaceTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, mGodrayBlurTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, 0);

		m_terrainLightFBO->attachTexture(0, terrainTex);
		m_ribbonFBO->attachTexture(0, ribbonTex);
		m_godrayFBO->attachTexture(0, mWaterSurfaceTex);
		setupBuffers();

		m_knobs[1].x = 0.6f;

		/*m_sdfGeometryFactory.reset(new SDFGeometryFactory(ctx));

		SDFGeometryDescription desc2 = SDFGeometryDescription(
			"shaders/geometry_generation/geom3.glsl",
			Vec3f(-14.5, -12.4, -7.1),
			Vec3i(48, 64, 48),
			0.65f,
			Vec3f(1500.0f, -140.0, 1700.0f),
			Vec3f(0.5f * FW_PI, 0.5f * FW_PI, 0.0f),
			Vec3f(38.0f));

		m_sdfGeometryFactory->produceGeometry(ctx, desc2, &mCurvyRocks);

		SDFGeometryDescription desc3(
			"shaders/geometry_generation/geom4.glsl",
			Vec3f(-17.25, -13.9, -17.71),
			Vec3i(64, 64, 64),
			0.56f,
			Vec3f(2150.0f, -305.0, 1900.0f),
			Vec3f(0.0f, 0.5f * FW_PI, 0.0f),
			Vec3f(18.0f));
		m_sdfGeometryFactory->produceGeometry(ctx, desc3, &mCurvyDonut);

		SDFGeometryDescription desc(
			"shaders/geometry_generation/geom2.glsl",
			Vec3f(-9.1, -7.1, -7.1),
			Vec3i(48, 64, 48),
			0.37f,
			Vec3f(2900.0f, -155.0, 900.0f),
			Vec3f(0.0f, 1.6*FW_PI, 0.0f),
			Vec3f(80.0f));
		m_sdfGeometryFactory->produceGeometry(ctx, desc, &mSphere);*/

		Image * sparkleImage = importImage("assets/sparkle.png");

		mLensFlareParticleTex = sparkleImage->createGLTexture();

		delete sparkleImage;

		loadRibbonPaths();

		//mParticleLogo.reset(new ParticleLogoSDF(ctx, 80));
		/*for (int i = 0; i < 20; ++i) {
		mParticleLogo->setDt(0.2f * float(i+1));
		mParticleLogo->updateParticles(ctx);
		}*/
		Image * img = importImage("assets/bokeh.png");
		mBokehTexture = img->createGLTexture();
		delete img;

		setupMissile();

		mCityMesh = importMesh("assets/city/Scifi downtown city.obj");
		mCityMesh->getVBO().getGLBuffer();
		for (int i = 0; i < mCityMesh->numSubmeshes(); i++)
		{
			const MeshBase::Material& mat = mCityMesh->material(i);
			mat.textures[MeshBase::TextureType_Diffuse].getGLTexture();
			mat.textures[MeshBase::TextureType_Normal].getGLTexture();
			mat.textures[MeshBase::TextureType_Specular].getGLTexture();


		}

		loadSubmarine();

		loadCamPaths();


		mCceMesh.reset((Mesh<VertexPNTC>*)importMesh("assets/city_logos/cce.obj"));
		mVarkoMesh.reset((Mesh<VertexPNTC>*)importMesh("assets/city_logos/varko.obj"));

		mCceMesh->getVBO().getGLBuffer();
		for (int i = 0; i < mCceMesh->numSubmeshes(); i++)
		{
			const MeshBase::Material& mat = mCceMesh->material(i);
			mat.textures[MeshBase::TextureType_Diffuse].getGLTexture();
			mat.textures[MeshBase::TextureType_Normal].getGLTexture();
			mat.textures[MeshBase::TextureType_Specular].getGLTexture();


		}
		mVarkoMesh->getVBO().getGLBuffer();
		for (int i = 0; i < mVarkoMesh->numSubmeshes(); i++)
		{
			const MeshBase::Material& mat = mVarkoMesh->material(i);
			mat.textures[MeshBase::TextureType_Diffuse].getGLTexture();
			mat.textures[MeshBase::TextureType_Normal].getGLTexture();
			mat.textures[MeshBase::TextureType_Specular].getGLTexture();


		}
	}

	void TessellationTestScene::getLightMatrix(const Vec3f & cameraPos, Mat4f & toLight, Mat4f & toLightClip) const {

		Vec3f z = Vec3f(-0.472f, 0.986f, 0.657f).normalized();
		Vec3f y(0, 1, 0);
		Vec3f x = cross(y, z).normalized();
		y = cross(z, x).normalized();

		static const Mat4f perspective = Mat4f::perspective(90.0f, 100.0f, 82000.0f, 1.0f);

		toLight.setRow(0, Vec4f(1.0f, 0.0f, 0.0f, -2100.0f));
		toLight.setRow(1, Vec4f(0.0f, 0.0f, 1.0f, -1900.0f));
		toLight.setRow(2, Vec4f(0.0f, 1.0f, 0.0f, -14000.0f));
		toLight.setRow(3, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));

		toLightClip = perspective * toLight;

	}

	void TessellationTestScene::lightPass(Window & wnd, const CameraControls & camera) {

		GLContext * gl = wnd.getGL();

		m_terrainLightFBO->bind();

		glViewport(0, 0, m_ProcessTexSize.x, m_ProcessTexSize.y);

		Mat4f toLight, toLightClip;

		getLightMatrix(camera.getPosition(), toLight, toLightClip);

		m_lightRenderProgram->use();

		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob1"), m_knobs[0]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob2"), m_knobs[1]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob3"), m_knobs[2]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob4"), m_knobs[3]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob5"), m_knobs[4]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob6"), m_knobs[5]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob7"), m_knobs[6]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob8"), m_knobs[7]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob9"), m_knobs[8]);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("knob10"), m_knobs[9]);

		gl->setUniform(m_lightRenderProgram->getUniformLoc("toScreen"), toLightClip);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("numInstancesX"), m_numPatchesX);
		gl->setUniform(m_lightRenderProgram->getUniformLoc("sideHalfLength"), m_sideHalfLength);



		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindFragDataLocation(m_lightRenderProgram->getHandle(), 0, "position");

		/*glBindVertexArray(m_planePatchVAO);
		glDrawArraysInstanced(GL_PATCHES, 0, 18, m_numPatchesX*m_numPatchesX);
		glBindVertexArray(0);*/

		m_meshRenderLightProgram->use();

		glBindFragDataLocation(m_meshRenderLightProgram->getHandle(), 0, "position");

		if (FWSync::rocketRibbonAlpha < 1.0f)
		{
			renderMissileLight(gl, toLightClip);
		}

		mCityLightProgram->use();
		Mat4f cityToWorld = getCityMeshWorldMatrix();
		gl->setUniform(mCityLightProgram->getUniformLoc("toScreen"), toLightClip*cityToWorld);
		gl->setUniform(mCityLightProgram->getUniformLoc("toWorld"), cityToWorld);
		renderMeshCheap(gl, mCityMesh, mCityLightProgram);

		mSubmarineParticleLightProgram->use();

		Mat4f submarineToWorld = getSubmarineToWorldMatrix();

		gl->setUniform(mSubmarineParticleLightProgram->getUniformLoc("toScreen"), toLightClip*submarineToWorld);
		gl->setUniform(mSubmarineParticleLightProgram->getUniformLoc("toWorld"), submarineToWorld);
		glBindVertexArray(mSubmarineVAO);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, mNumSubmarineParticles);
		glBindVertexArray(0);

		renderSurfacesLight(gl, toLightClip);

		m_terrainLightFBO->unbind();

	}

	void TessellationTestScene::render(Window & wnd, const CameraControls & camera) {
		mCamPtr->setNear(0.5f);
		mCamPtr->setFar(14950.0f);
		GLContext * gl = wnd.getGL();
		Vec3f seaColor = m_knobs[7].getXYZ();
		Vec3f lightColor = Vec3f(0.125, 0.104, 0.184);
		seaColor = Vec3f(0.001242, 0.0f, 0.007453f);
		//lightColor = Vec3f(1.0, 0.0745f, 0.453416f);

		mCamPtr->setPosition(getCameraPosition());
		mCamPtr->setForward(getCameraForward());

		Mat4f camToClip = camera.getWorldToClip();
		Mat4f camToCamera = camera.getWorldToCamera();
		Vec3f cameraDir = camera.getForward();
		Vec3f cameraPos = camera.getPosition();

		Mat4f toLight, toLightClip;
		getLightMatrix(cameraPos, toLight, toLightClip);

		Vec3f lightPos = (toLight.inverted() * Vec4f(0, 0, 0, 1)).getXYZ();
		Vec3f lightDir = ((toLight * Vec4f(0, 0, 0, 1)).getXYZ() - lightPos).normalized();

		updateUnderwaterParticles(gl);

		glEnable(GL_DEPTH_TEST);

		lightPass(wnd, camera);

		updateUniforms(wnd, camera);

		Vec2i sz = wnd.getSize();

		glViewport(0, 0, sz.x, sz.y);


		/*
		START RENDER GEOMETRY
		*/

		GLuint fbo = m_gbuffer->getFBO();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		glViewport(0, 0, sz.x, sz.y);

		glClearColor(seaColor.x, seaColor.y, seaColor.z, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		m_tessProgram->use();

		glBindFragDataLocation(m_tessProgram->getHandle(), 0, "diffuseColor");
		glBindFragDataLocation(m_tessProgram->getHandle(), 1, "normal");
		glBindFragDataLocation(m_tessProgram->getHandle(), 2, "position");
		glBindFragDataLocation(m_tessProgram->getHandle(), 3, "outDepth");

		gl->setUniform(m_tessProgram->getUniformLoc("numInstancesX"), m_numPatchesX);
		gl->setUniform(m_tessProgram->getUniformLoc("sideHalfLength"), m_sideHalfLength);
		gl->setUniform(m_tessProgram->getUniformLoc("seaColor"), seaColor);
		gl->setUniform(m_tessProgram->getUniformLoc("lightColorUniform"), lightColor);
		gl->setUniform(m_tessProgram->getUniformLoc("lightDirectionUniform"), lightDir);
		gl->setUniform(m_tessProgram->getUniformLoc("lightPosUniform"), lightPos);
		gl->setUniform(m_tessProgram->getUniformLoc("toLightScreen"), toLightClip);
		gl->setUniform(m_tessProgram->getUniformLoc("screenSize"), Vec2f(sz));
		gl->setUniform(m_tessProgram->getUniformLoc("depthMap"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_terrainLightFBO->getTexture(0));

		// draw terrain
		/*glBindVertexArray(m_planePatchVAO);
		glDrawArraysInstanced(GL_PATCHES, 0, 18, m_numPatchesX*m_numPatchesX);
		glBindVertexArray(0);*/

		// draw object

		/*m_meshRenderProgram->use();

		glBindFragDataLocation(m_meshRenderProgram->getHandle(), 0, "diffuseColor");
		glBindFragDataLocation(m_meshRenderProgram->getHandle(), 1, "normal");
		glBindFragDataLocation(m_meshRenderProgram->getHandle(), 2, "position");
		glBindFragDataLocation(m_meshRenderProgram->getHandle(), 3, "outDepth");
		gl->setUniform(m_meshRenderProgram->getUniformLoc("seaColor"), seaColor);
		gl->setUniform(m_meshRenderProgram->getUniformLoc("lightColorUniform"), lightColor);
		gl->setUniform(m_meshRenderProgram->getUniformLoc("lightDirectionUniform"), lightDir);
		gl->setUniform(m_meshRenderProgram->getUniformLoc("lightPosUniform"), lightPos);
		gl->setUniform(m_meshRenderProgram->getUniformLoc("cameraPosUniform"), camera.getPosition());
		gl->setUniform(m_meshRenderProgram->getUniformLoc("cameraDirUniform"), cameraDir);
		gl->setUniform(m_meshRenderProgram->getUniformLoc("toLightScreen"), toLightClip);
		gl->setUniform(m_meshRenderProgram->getUniformLoc("screenSize"), Vec2f(sz));
		gl->setUniform(m_meshRenderProgram->getUniformLoc("depthMap"), 0);

		renderMeshGeometry(gl, m_meshRenderProgram, camToClip, camToCamera);*/

		if (FWSync::rocketRibbonAlpha < 1.0)
		{
			renderMissile(gl, camToClip, lightColor, seaColor, lightPos, lightDir, cameraPos);
		}
		

		mCityRenderProgram->use();
		Mat4f cityToWorld = getCityMeshWorldMatrix();
		gl->setUniform(mCityRenderProgram->getUniformLoc("toScreen"), camToClip*cityToWorld);
		gl->setUniform(mCityRenderProgram->getUniformLoc("toWorld"), cityToWorld);
		gl->setUniform(mCityRenderProgram->getUniformLoc("normalToWorld"), cityToWorld);
		gl->setUniform(mCityRenderProgram->getUniformLoc("cameraPos"), cameraPos);
		gl->setUniform(mCityRenderProgram->getUniformLoc("sineT"), FWSync::citySineWaveTime);

		gl->setUniform(mCityRenderProgram->getUniformLoc("seaColor"), seaColor);
		gl->setUniform(mCityRenderProgram->getUniformLoc("lightColor"), lightColor);
		gl->setUniform(mCityRenderProgram->getUniformLoc("lightDirection"), lightDir);
		gl->setUniform(mCityRenderProgram->getUniformLoc("lightPosition"), lightPos);
		gl->setUniform(mCityRenderProgram->getUniformLoc("cameraPosition"), cameraPos);
		gl->setUniform(mCityRenderProgram->getUniformLoc("toLightScreen"), toLightClip);
		gl->setUniform(mCityRenderProgram->getUniformLoc("screenSize"), Vec2f(sz));
		gl->setUniform(mCityRenderProgram->getUniformLoc("depthMap"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_terrainLightFBO->getTexture(0));
		renderMesh(gl, mCityMesh, mCityRenderProgram);

		renderSubmarine(gl, camToClip, lightPos, lightDir, lightColor, seaColor, cameraPos);

		//renderAuthorLogos(gl, camToClip, lightPos, lightDir, lightColor, seaColor, cameraPos);
		renderSurfaces(gl, camToClip, lightPos, lightDir, lightColor, seaColor, cameraPos);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		/*
		END RENDER GEOMETRY
		*/

		/* START RENDER RIBBONS */

		m_ribbonFBO->bind();

		glViewport(0, 0, sz.x, sz.y);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		// draw points

		// render ribbons

		/*if (FWSync::waterLightParticleSize > 0.0f)
		{
			glUseProgram(0);
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(&camToClip(0, 0) );
			glMatrixMode(GL_MODELVIEW);
			Mat4f ident;
			ident.setIdentity();
			glLoadMatrixf(&ident(0, 0));

			glEnable(GL_POINT_SMOOTH);
			glPointSize(FWSync::waterLightParticleSize);

			glBegin(GL_POINTS);
			glColor3f(FWSync::logoGodrayColorR, FWSync::logoGodrayColorG, FWSync::logoGodrayColorB);
			glVertex3f(307.0f, 7007.0f, 2712.0f);
			glEnd();
			glDisable(GL_POINT_SMOOTH);
		}*/

		

		renderUnderwaterParticles(gl, camToCamera, camToClip, seaColor);

		Vec3f up = mCamPtr->getUp();
		Vec3f dir = mCamPtr->getForward();
		Vec3f horizontal = cross(up, dir).normalized();
		//renderFlares(gl, camToClip, mCamPtr->getPosition(), mCamPtr->getUp(), horizontal);

		m_ribbonFBO->unbind();

		/* END RENDER RIBBONS */

		/*
		START GODRAY PASS
		*/
		godrayPass(gl, camToClip, lightColor, seaColor, lightPos, toLightClip, lightDir, cameraPos);
		/*
		END GODRAY PASS
		*/


		/*
		START BLUR RIBBONS
		*/

		m_gaussianFilter->process(gl, m_ribbonFBO->getTexture(0), m_blurResultTex, 3);

		/*
		END BLUR RIBBONS
		*/
		if (FWSync::logoGodrayDecay > 0.0f)
		{
			glDisable(GL_DEPTH_TEST);
			mGodrayFBO->bind();
			glClear(GL_COLOR_BUFFER_BIT);
			mSubmarineGodrayProgram->use();

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_DIFFUSE));
			gl->setUniform(mSubmarineGodrayProgram->getUniformLoc("bwSampler"), 0);

			Vec4f submarineCenter = camToClip * getSubmarineToWorldMatrix() * Vec4f(0, 0, 0, 1);
			submarineCenter /= submarineCenter.w;
			submarineCenter.x = submarineCenter.x * 0.5f + 0.5f;
			submarineCenter.y = submarineCenter.y * 0.5f + 0.5f;

			gl->setUniform(mSubmarineGodrayProgram->getUniformLoc("lightPos"), submarineCenter.getXY());
			gl->setUniform(mSubmarineGodrayProgram->getUniformLoc("DECAY"), FWSync::logoGodrayDecay);

			glBindVertexArray(m_quadVAO);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glBindVertexArray(0);
		

		mGodrayFBO->unbind();
		glEnable(GL_DEPTH_TEST);
		}

		glViewport(0, 0, sz.x, sz.y);

		// radial blur of the buffer

		// combine

		// show stuff
		glDisable(GL_DEPTH_TEST); // TODO remove

		mLastPassFBO->bind();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



		m_combineProgram->use();

		glBindFragDataLocation(m_combineProgram->getHandle(), 0, "color");

		gl->setUniform(m_combineProgram->getUniformLoc("gradUniform"), FWSync::gradMix);
		gl->setUniform(m_combineProgram->getUniformLoc("gradBottom"), Vec3f(FWSync::gradR, FWSync::gradG, FWSync::gradB));

		gl->setUniform(m_combineProgram->getUniformLoc("terrainColorTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_DIFFUSE));
		

		gl->setUniform(m_combineProgram->getUniformLoc("particleColorTex"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_blurResultTex);

		gl->setUniform(m_combineProgram->getUniformLoc("godrayBlurTex"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mGodrayBlurTex);

		if (FWSync::logoGodrayDecay > 0.0f)
		{
			gl->setUniform(m_combineProgram->getUniformLoc("useSubmarineGodray"), true);
			gl->setUniform(m_combineProgram->getUniformLoc("submarineGodray"), 3);
			glActiveTexture(GL_TEXTURE2 + 1);
			glBindTexture(GL_TEXTURE_2D, mGodrayFBO->getTexture(0));
		}
		else {
			gl->setUniform(m_combineProgram->getUniformLoc("useSubmarineGodray"), false);
		}
		

		/*m_displayProgram->use();

		gl->setUniform(m_displayProgram->getUniformLoc("inTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGodrayBlurTex);*/

		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		if (mDebugRenderCurve)
		{
			debugRenderPath(gl);
		}

		mLastPassFBO->unbind();
		glEnable(GL_DEPTH_TEST); // TODO remove

	}

	void TessellationTestScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			//mParticleLogo->loadShaders(wnd.getGL());
			break;
		case Action::Action_AddRibbonPoint:
			mCurveControlPoints[mSelectedCurveSlider].push_back(mCamPtr->getPosition());
			break;
		case Action::Action_DeleteRibbonPoint:
			if (mCurveControlPoints[mSelectedCurveSlider].size() > 0u)
			{
				mCurveControlPoints[mSelectedCurveSlider].pop_back();
			}
			break;
		case Action::Action_SaveRibbonPath:
			saveRibbonPaths();
			break;
		case Action::Action_DebugRenderRibbonPath:
			mDebugRenderCurve = !mDebugRenderCurve;
			if (mDebugRenderCurve) {
				for (size_t k = 0; k < mCurveControlPoints.size(); ++k) {
					mRibbonCurves[k] = evalCatmullRomspline(mCurveControlPoints[k], 80.0f, false, 0.0f, 0.0f);
				}
			}
			break;
		case Action::Action_RightButton:
			selectControlPoint(wnd.getMousePos());
			break;
		default:
			break;
		}
	}

	void TessellationTestScene::activate(Window & wnd, CommonControls & controls) {
		updateGUI(wnd, controls);
	}
	void TessellationTestScene::updateGUI(Window & wnd, CommonControls & controls) {
		cleanUpGUI(wnd, controls);

		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(-40.0f), Vec4f(40.0f)), // knob1
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob2
			std::make_pair(Vec4f(-1.0f), Vec4f(1.0f)), // knob3
			std::make_pair(Vec4f(-20.0f), Vec4f(20.0f)), // knob4
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob5
			std::make_pair(Vec4f(-150.0f), Vec4f(150.0f)), // knob6
			std::make_pair(Vec4f(0.0f), Vec4f(10.0f)), // knob7
			std::make_pair(Vec4f(0.0f), Vec4f(0.2f)), // knob8
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

		controls.beginSliderStack();
		controls.addSlider(&mSelectedCurveSlider, 0, mCurveControlPoints.size() - 1, false, FW_KEY_NONE, FW_KEY_NONE, "curve id = %d");
		controls.endSliderStack();

		controls.beginSliderStack();
		controls.addSlider(&mRocketRenderNumVertices, 0, mRocketMeshNumVertices, false, FW_KEY_NONE, FW_KEY_NONE, "num trigs= %d");
		controls.endSliderStack();

		controls.addButton((S32*)&actionExt, (S32)Action::Action_AddRibbonPoint, FW_KEY_V, "Add p");
		controls.addButton((S32*)&actionExt, (S32)Action::Action_DeleteRibbonPoint, FW_KEY_NONE, "Pop p");
		controls.addButton((S32*)&actionExt, (S32)Action::Action_DebugRenderRibbonPath, FW_KEY_NONE, "Debug p");
		controls.addButton((S32*)&actionExt, (S32)Action::Action_SaveRibbonPath, FW_KEY_NONE, "Save p");
	}
	void TessellationTestScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}
		controls.removeControl(&mSelectedCurveSlider);
		controls.removeControl(&actionExt);
	}

	void TessellationTestScene::loadShaders(GLContext * ctx) {
		const char vertexShader[] = "shaders/tessellation/simple_vertex.glsl";
		const char tessControlShader[] = "shaders/tessellation/simple_tess_control.glsl";
		const char tessEvalShader[] = "shaders/tessellation/simple_tess_eval.glsl";
		const char fragmentShader[] = "shaders/tessellation/simple_fragment.glsl";

		const char declShader[] = "shaders/raymarch/decl.glsl";
		const char noiseShader[] = "shaders/raymarch/noise.glsl"; // noise utilities
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms

		m_tessProgram = loadShader(ctx, { vertexShader }, { tessControlShader },
		{
			declShader,
			noiseShader,
			knobsShader,
			tessEvalShader
		},
		{ declShader,
			noiseShader,
			knobsShader, fragmentShader }, m_programName);


		const char waterSurfaceVertex[] = "shaders/tessellation/water_vertex.glsl";
		const char waterControlShader[] = "shaders/tessellation/water_tess_control.glsl";
		const char waterEvalShader[] = "shaders/tessellation/water_tess_eval.glsl";
		const char waterFragmentShader[] = "shaders/tessellation/water_fragment.glsl";
		mWaterSurfaceProgram = loadShader(ctx, waterSurfaceVertex, waterControlShader,
			waterEvalShader,

			waterFragmentShader, mWaterSurfaceProgramName);


		const char displayVertex[] = "shaders/common/display_vertex.glsl";
		const char displayFragment[] = "shaders/common/display_fragment.glsl";

		m_displayProgram = loadShader(ctx, displayVertex, displayFragment, m_displayProgramName);

		const char godrayBlurFragment[] = "shaders/tessellation/godray_blur_frag.glsl";
		mGodrayBlurProgram = loadShader(ctx, displayVertex, godrayBlurFragment, mGodrayBlurProgramName);

		const char godrayRaymarchFragment[] = "shaders/tessellation/godray_raymarch_fragment.glsl";
		mGodrayRaymarchProgram = loadShader(ctx, displayVertex, godrayRaymarchFragment, mGodrayRaymarchProgramName);

		const char lightVertexShader[] = "shaders/tessellation/light_vertex.glsl";
		const char lightTessControlShader[] = "shaders/tessellation/light_tess_control.glsl";
		const char lightTessEvalShader[] = "shaders/tessellation/light_tess_eval.glsl";
		const char lightFragmentShader[] = "shaders/tessellation/light_fragment.glsl";

		m_lightRenderProgram = loadShader(ctx, { lightVertexShader }, { lightTessControlShader },
		{
			declShader,
			noiseShader,
			knobsShader,
			lightTessEvalShader
		},
		{ lightFragmentShader }, m_lightRenderProgramName);


		const char combineVertex[] = "shaders/tessellation/combine_vertex.glsl";
		const char combineFragment[] = "shaders/tessellation/combine_fragment.glsl";

		m_combineProgram = loadShader(ctx, combineVertex, combineFragment, m_combineProgramName);

		const char meshRenderVertexShader[] = "shaders/tessellation/mesh_render_vertex.glsl";
		const char meshRenderFragmentShader[] = "shaders/tessellation/mesh_render_fragment.glsl";

		m_meshRenderProgram = loadShader(ctx,
			meshRenderVertexShader, meshRenderFragmentShader, m_meshRenderProgramName
		);

		const char meshRenderLightVertexShader[] = "shaders/tessellation/mesh_render_light_vertex.glsl";
		const char meshRenderLightFragmentShader[] = "shaders/tessellation/mesh_render_light_fragment.glsl";

		m_meshRenderLightProgram = loadShader(ctx,
			meshRenderLightVertexShader, meshRenderLightFragmentShader, m_meshRenderLightProgramName
		);


		const char renderCarmullVertex[] = "shaders/curve_render/catmull_rom_vertex_no_tess.glsl";
		const char renderCarmullFragment[] = "shaders/curve_render/catmull_rom_frag_no_tess.glsl";

		m_renderCurveNoTessProgram = loadShader(ctx, renderCarmullVertex, renderCarmullFragment, m_renderCurveNoTessProgramName);

		const char renderMissileVertex[] = "shaders/missile/missile_vertex.glsl";
		const char renderMissileFragment[] = "shaders/missile/missile_fragment.glsl";

		mMissileProgram = loadShader(ctx, renderMissileVertex, renderMissileFragment, mMissileProgramName);

		const char renderMissileLightVertex[] = "shaders/missile/missile_light_vertex.glsl";
		const char renderMissileLightFragment[] = "shaders/missile/missile_light_fragment.glsl";

		mMissileLightProgram = loadShader(ctx, renderMissileLightVertex, renderMissileLightFragment, mMissileLightProgramName);

		const char renderCityMeshLightVertex[] = "shaders/city_mesh/city_light_vertex.glsl";
		const char renderCityMeshLightFragment[] = "shaders/city_mesh/city_light_fragment.glsl";
		mCityLightProgram = loadShader(ctx, renderCityMeshLightVertex, renderCityMeshLightFragment, mCityLightProgramName);

		const char renderCityMeshVertex[] = "shaders/city_mesh/city_vertex.glsl";
		const char renderCityMeshFragment[] = "shaders/city_mesh/city_fragment.glsl";
		mCityRenderProgram = loadShader(ctx, renderCityMeshVertex, renderCityMeshFragment, mCityRenderProgramName);

		const char renderCityLogoMeshVertex[] = "shaders/city_mesh/city_logo_vert.glsl";
		const char renderCityLogoMeshFragment[] = "shaders/city_mesh/city_logo_frag.glsl";
		mAuthorRenderProgram = loadShader(ctx, renderCityLogoMeshVertex, renderCityLogoMeshFragment, mAuthorRenderProgramName);

		const char renderUnderWaterVertex[] = "shaders/underwater/render_vert.glsl";
		const char renderUnderWaterFragment[] = "shaders/underwater/render_frag.glsl";
		const char updateUnderWaterCS[] = "shaders/underwater/update_cs.glsl";

		mUnderwaterParticlesRenderProgram = loadShader(ctx, renderUnderWaterVertex, renderUnderWaterFragment, mUnderwaterParticlesRenderProgramName);
		mUnderwaterParticlesUpdateProgram = loadShader(ctx, updateUnderWaterCS, mUnderwaterParticlesUpdateProgramName);


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

		m_tessProgram->use();
		ctx->setUniformArray(m_tessProgram->getUniformLoc("ldSamples"), 36, (Vec2f*)ldSamples);

		m_meshRenderProgram->use();
		ctx->setUniformArray(m_meshRenderProgram->getUniformLoc("ldSamples"), 36, (Vec2f*)ldSamples);

		mCityRenderProgram->use();
		ctx->setUniformArray(mCityRenderProgram->getUniformLoc("ldSamples"), 36, (Vec2f*)ldSamples);

		const char renderSubmarineVertex[] = "shaders/submarine/render_vert.glsl";
		const char renderSubmarineFragment[] = "shaders/submarine/render_frag.glsl";

		mSubmarineParticleProgram = loadShader(ctx, renderSubmarineVertex, renderSubmarineFragment, mSubmarineParticleProgramName);

		const char renderSubmarineLightVertex[] = "shaders/submarine/render_light_vert.glsl";
		const char renderSubmarineLightFragment[] = "shaders/submarine/render_light_frag.glsl";
		mSubmarineParticleLightProgram = loadShader(ctx, renderSubmarineLightVertex, renderSubmarineLightFragment, mSubmarineParticleLightProgramName);


		mSubmarineSDFMoveProgram = loadShader(ctx, "shaders/submarine/sdf_move.glsl", mSubmarineSDFMoveProgramName);

		mMeshCurveRenderProgram = loadShader(ctx, 
			"shaders/mesh_curves/mesh_curve_vert.glsl", 
			"shaders/mesh_curves/mesh_curve_frag.glsl", 
			"underwater_mesh_curve_render");

		mMeshCurveRenderLightProgram = loadShader(ctx,
			"shaders/mesh_curves/mesh_curve_light_vert.glsl",
			"shaders/mesh_curves/mesh_curve_light_frag.glsl",
			"underwater_mesh_curve_render_light");

		mSubmarineGodrayProgram = loadShader(ctx, "shaders/common/display_vertex.glsl", "shaders/submarine/godray_blur_frag.glsl", "submarine_godray_city_scene");

		mBillboardLensProgram = loadShader(ctx, "shaders/curve_render/billboard_lens_vert.glsl", "shaders/curve_render/billboard_lens_frag.glsl", "billboard_lens_city");

		ctx->checkErrors();
	}

	void TessellationTestScene::updateUniforms(Window & wnd, const CameraControls & camera) {
		GLContext * ctx = wnd.getGL();

		// update ray march program uniforms
		GLContext::Program * prog = m_tessProgram;

		prog->use();

		Mat4f toCamera = camera.getWorldToCamera();
		Mat4f toScreen = camera.getWorldToClip();


		ctx->setUniform(prog->getUniformLoc("toCamera"), toCamera);
		ctx->setUniform(prog->getUniformLoc("toScreen"), toScreen);
		ctx->setUniform(prog->getUniformLoc("normalToCamera"), toCamera.inverted().transposed());

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

		float currentTime = GLOBAL_TIMER.getElapsed();
		ctx->setUniform(prog->getUniformLoc("time"), currentTime);

	}

	void TessellationTestScene::setupBuffers() {

		static const float planePositions[] =
		{
			-1.0, 0.0, -1.0,
			1.0, 0.0, -1.0,
			1.0, 0.0, 1.0,

			-1.0, 0.0, -1.0,
			1.0, 0.0, 1.0,
			-1.0, 0.0, 1.0
		};

		glGenVertexArrays(1, &m_planePatchVAO);
		glBindVertexArray(m_planePatchVAO);

		glGenBuffers(1, &m_planePatchVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_planePatchVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18, planePositions, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

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

		static const GLfloat lightParticles[] = {
			-1.0f, -1.0f,
			1.0f, -1.0f,
			1.0f, 1.0f,

			-1.0f, -1.0f,
			1.0f, 1.0f,
			-1.0f, 1.0f
		};

		glGenVertexArrays(1, &m_lightPatchVAO);
		glGenBuffers(1, &m_lightPatchVBO);
		glBindVertexArray(m_lightPatchVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_lightPatchVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, lightParticles, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		FW::Random rnd(101778787);
		numUnderwaterParticles = 240000;
		std::vector<Vec4f> particlePositions(numUnderwaterParticles);

		for (size_t i = 0; i < particlePositions.size(); ++i)
		{
			float x = rnd.getF32(-8000, 8000.0f);
			float y = rnd.getF32(20.0f, 13000.0f);
			float z = rnd.getF32(-8000, 8000.0f);
			particlePositions[i] = Vec4f(x, y, z, 200.0f);
		}

		glGenVertexArrays(1, &mUnderwaterParticlesVAO);
		glBindVertexArray(mUnderwaterParticlesVAO);


		glGenBuffers(1, &mUnderwaterParticlesVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mUnderwaterParticlesVBO);
		glBufferData(GL_ARRAY_BUFFER, particlePositions.size() * sizeof(Vec4f), particlePositions.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glGenVertexArrays(1, &mFlareParticlesVAO);
		glBindVertexArray(mFlareParticlesVAO);

		glGenBuffers(1, &mFlareParticlesVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mFlareParticlesVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4f) * 4, NULL, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

	}

	void TessellationTestScene::saveRibbonPaths()
	{

		for (size_t i = 0; i < mCurveControlPoints.size(); ++i) {
			std::string filePath = "assets/ribbon_path_" + std::to_string(i + 1) + ".txt";
			saveRibbonPath(filePath, mCurveControlPoints[i]);
		}

	}

	void TessellationTestScene::saveRibbonPath(const std::string & fileName, const std::vector<Vec3f> & vec) {

		std::ofstream out(fileName);

		out << vec.size() << std::endl;

		for (size_t i = 0; i < vec.size(); ++i) {
			out << vec[i].x << " " << vec[i].y << " " << vec[i].z << std::endl;
		}
	}

	void TessellationTestScene::debugRenderPath(GLContext * gl) {

		static const Vec3f CURVE_COLORS[] = {
			Vec3f(0.3, 0.45,1.0), Vec3f(0.3,0.9,1.0),Vec3f(1.0,0.2,0.45),
			Vec3f(0.75,0.5,0.0), Vec3f(0.95,0.71,0.0), Vec3f(1.0, 0.4, 0.2),
			Vec3f(0.2,1.0,0.45), Vec3f(0.8,1.0,0.2), Vec3f(1,1,1)
		};

		static Vec4f deltaValue = m_knobs[5];
		if (deltaValue != m_knobs[5] && mSelectedCurveIndex != -1 && mSelectedControlPointIndex != -1) {
			deltaValue = m_knobs[5];
			mCurveControlPoints[mSelectedCurveIndex][mSelectedControlPointIndex]
				= mSelectedPointOrigValue + m_knobs[5].getXYZ();
			mRibbonCurves[mSelectedCurveIndex] =
				evalCatmullRomspline(mCurveControlPoints[mSelectedCurveIndex], 80.0f, false, 0.0f, 0.0f);
		}

		glUseProgram(0);
		glPushMatrix();
		glMatrixMode(GL_PROJECTION);
		Mat4f worldToCamera = mCamPtr->getWorldToCamera();
		Mat4f projection = /*gl->xformFitToView(Vec2f(-1.0f, -1.0f), Vec2f(2.0f, 2.0f)) **/ mCamPtr->getCameraToClip();

		glLoadMatrixf(&projection(0, 0));
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&worldToCamera(0, 0));

		glLineWidth(2.5f);
		glBegin(GL_LINES);

		for (size_t k = 0; k < mRibbonCurves.size(); ++k) {
			glColor3f(CURVE_COLORS[k].x, CURVE_COLORS[k].y, CURVE_COLORS[k].z);
			for (size_t i = 1; i < mRibbonCurves[k].size(); ++i) {
				CurvePoint cp1 = mRibbonCurves[k][i - 1];
				CurvePoint cp2 = mRibbonCurves[k][i];
				glVertex3f(cp1.V.x, cp1.V.y, cp1.V.z);
				glVertex3f(cp2.V.x, cp2.V.y, cp2.V.z);
			}
		}


		glEnd();

		glColor3f(1, 0, 0);
		glPointSize(7.0f);
		glBegin(GL_POINTS);

		for (size_t k = 0; k < mCurveControlPoints.size(); ++k) {
			for (size_t i = 0; i < mCurveControlPoints[k].size(); ++i) {
				Vec3f cp = mCurveControlPoints[k][i];
				glVertex3f(cp.x, cp.y, cp.z);
			}
		}

		glEnd();
		glPopMatrix();

	}

	void TessellationTestScene::loadRibbonPath(const std::string & fileName, std::vector<Vec3f> & vec) {

		std::ifstream in(fileName);

		size_t numPoints;
		in >> numPoints;
		vec.resize(numPoints);
		for (size_t i = 0; i < numPoints; ++i) {
			Vec3f p;
			in >> p.x >> p.y >> p.z;
			vec[i] = p;
		}
	}

	void TessellationTestScene::selectControlPoint(Vec2i mousePos) {

		if (mSelectedControlPointIndex != -1) {
			mCurveControlPoints[mSelectedCurveIndex][mSelectedControlPointIndex] =
				m_knobs[5].getXYZ() + mSelectedPointOrigValue;
		}

		mSelectedControlPointIndex = -1;
		mSelectedCurveIndex = -1;
		m_knobs[5] = Vec4f(0.0f);

		// generate ray
		Mat4f worldToCamera = mCamPtr->getWorldToCamera();
		Mat4f projection = mCamPtr->getCameraToClip();

		// inverse projection from clip space to world space
		Mat4f invP = (projection * worldToCamera).inverted();



		float x = float(mousePos.x) / mWndWidth *  2.0f - 1.0f;
		float y = float(mousePos.y) / mWndHeight * -2.0f + 1.0f;

		// point on front plane in homogeneous coordinates
		Vec4f P0(x, y, 0.0f, 1.0f);
		// point on back plane in homogeneous coordinates
		Vec4f P1(x, y, 1.0f, 1.0f);

		// apply inverse projection, divide by w to get object-space points
		Vec4f Roh = (invP * P0);
		Vec3f Ro = (Roh * (1.0f / Roh.w)).getXYZ();
		Vec4f Rdh = (invP * P1);
		Vec3f Rd = (Rdh * (1.0f / Rdh.w)).getXYZ();

		Rd = (Rd - Ro);

		float t = 1.0f;
		for (size_t k = 0; k < mCurveControlPoints.size(); ++k) {
			for (size_t i = 0; i < mCurveControlPoints[k].size(); ++i) {
				if (intersect_sphere(Ro, Rd, mCurveControlPoints[k][i], 10.0f, t)) {
					mSelectedCurveIndex = k;
					mSelectedControlPointIndex = i;
					mSelectedPointOrigValue = mCurveControlPoints[k][i];
				}
			}
		}
	}

	void TessellationTestScene::loadRibbonPaths() {
		static const int ribbonsCount = 4;

		mRibbons.resize(ribbonsCount);
		mCurveControlPoints.resize(ribbonsCount);
		mRibbonCurves.resize(ribbonsCount);

		for (size_t i = 0; i < ribbonsCount; ++i) {
			std::string filePath = "assets/ribbon_path_" + std::to_string(i + 1) + ".txt";
			loadRibbonPath(filePath, mCurveControlPoints[i]);
			mRibbonCurves[i] = evalCatmullRomspline(mCurveControlPoints[i], 120.0f, false, 0.0f, 0.0f);
		}

		std::vector<Vec3f> starControlPoints;
		loadRibbonPath("assets/star_curve.txt", starControlPoints);

		for (size_t i = 0; i < starControlPoints.size(); ++i)
		{
			starControlPoints[i] *= 8.0;
		}

		Curve starCurve = evalCatmullRomspline(starControlPoints, 20, false, 0.0, 0.0);
		mProfileNumIndices = starCurve.size();
		mCurveSurfacesVAO.resize(mRibbonCurves.size());
		mCurveSurfacesVBO.resize(mRibbonCurves.size());
		mCurveSurfacesIBO.resize(mRibbonCurves.size());
		mCurveNumTriangles.resize(mRibbonCurves.size());
		glGenVertexArrays(mCurveSurfacesVAO.size(), mCurveSurfacesVAO.data());
		glGenBuffers(mCurveSurfacesVBO.size(), mCurveSurfacesVBO.data());
		glGenBuffers(mCurveSurfacesIBO.size(), mCurveSurfacesIBO.data());
		for (size_t i = 0; i < mRibbonCurves.size(); ++i)
		{
			Surface surf = makeGenCyl(starCurve, mRibbonCurves[i]);
			mCurveNumTriangles[i] = surf.VF.size();

			std::vector<MissileMeshVertex> vertexData(surf.VV.size());

			for (size_t k = 0; k < vertexData.size(); ++k)
			{
				vertexData[k] = MissileMeshVertex(surf.VV[k], surf.VN[k], surf.VT[k].x, surf.VT[k].y);
			}

			glBindVertexArray(mCurveSurfacesVAO[i]);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mCurveSurfacesIBO[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, surf.VF.size() * sizeof(Vec3i), surf.VF.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, mCurveSurfacesVBO[i]);
			glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(MissileMeshVertex), vertexData.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), NULL);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), (char*)NULL + sizeof(Vec4f));

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

		}
	}

	void TessellationTestScene::renderUnderwaterParticles(GLContext * gl, const Mat4f & toCamera, const Mat4f & toClip, const Vec3f & seaColor) {

		mUnderwaterParticlesRenderProgram->use();
		gl->setUniform(mUnderwaterParticlesRenderProgram->getUniformLoc("toScreen"), toClip);
		gl->setUniform(mUnderwaterParticlesRenderProgram->getUniformLoc("bokehTexture"), 0);
		gl->setUniform(mUnderwaterParticlesRenderProgram->getUniformLoc("seaColor"), seaColor);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mBokehTexture);

		glBindVertexArray(mUnderwaterParticlesVAO);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glEnable(GL_POINT_SPRITE);
		glDrawArrays(GL_POINTS, 0, numUnderwaterParticles);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDisable(GL_POINT_SPRITE);
		glBindVertexArray(0);



	}

	void TessellationTestScene::updateUnderwaterParticles(GLContext * gl) {
		mUnderwaterParticlesUpdateProgram->use();

		static float dtPrev = m_knobs[0].y;



		gl->setUniform(mUnderwaterParticlesUpdateProgram->getUniformLoc("numParticles"), numUnderwaterParticles);
		gl->setUniform(mUnderwaterParticlesUpdateProgram->getUniformLoc("dtUniform"), GLOBAL_DT);
		//gl->setUniform(mUnderwaterParticlesUpdateProgram->getUniformLoc("dtUniform"), m_knobs[0].y);
		//gl->setUniform(mUnderwaterParticlesUpdateProgram->getUniformLoc("dtPrevUniform"), dtPrev);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mUnderwaterParticlesVBO);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mAttractorSSBO);

		int localSizeX = 128;

		int groupSizeX = (numUnderwaterParticles + localSizeX - 1) / localSizeX;

		glDispatchCompute(groupSizeX, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

		//dtPrev = m_knobs[0].y;
	}

	void TessellationTestScene::godrayPass(GLContext * gl, const Mat4f & toClip, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & lightPos, const Mat4f & toLightClip, const Vec3f & lightDir, const Vec3f & camPos) {


		// bind fbo
		m_godrayFBO->bind();

		glViewport(0, 0, m_ProcessTexSize.x, m_ProcessTexSize.y);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		// render water plane white


		mGodrayRaymarchProgram->use();

		glBindFragDataLocation(mGodrayRaymarchProgram->getHandle(), 0, "outColor");

		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("camPos"), camPos);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("lightPos"), lightPos);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("lightDir"), lightDir);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("toLightClip"), toLightClip);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("positionMap"), 0);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("lightColor"), lightColor);
		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("toInvClip"), toClip.inverted());
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gbuffer->getTexture(GBUFFER_TYPES::GBUFFER_POSITION));

		gl->setUniform(mGodrayRaymarchProgram->getUniformLoc("depthMap"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_terrainLightFBO->getTexture(0));

		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		m_godrayFBO->unbind();

		m_gaussianFilter->process(gl, m_godrayFBO->getTexture(0), mGodrayBlurTex, 6);

		/*mWaterSurfaceProgram->use();


		gl->setUniform(mWaterSurfaceProgram->getUniformLoc("time"), GLOBAL_TIMER.getElapsed());
		gl->setUniform(mWaterSurfaceProgram->getUniformLoc("toScreen"), toClip);
		gl->setUniform(mWaterSurfaceProgram->getUniformLoc("numInstancesX"), m_numPatchesX);
		gl->setUniform(mWaterSurfaceProgram->getUniformLoc("sideHalfLength"), 6.0f*m_sideHalfLength);
		gl->setUniform(mWaterSurfaceProgram->getUniformLoc("lightColor"), lightColor);
		gl->setUniform(mWaterSurfaceProgram->getUniformLoc("seaColor"), seaColor);

		// render geometry black
		glBindFragDataLocation(mWaterSurfaceProgram->getHandle(), 0, "color");

		glBindVertexArray(m_planePatchVAO);
		glDrawArraysInstanced(GL_PATCHES, 0, 18, m_numPatchesX*m_numPatchesX);
		glBindVertexArray(0);

		// unbind fbo
		m_godrayFBO->unbind();

		// blur upwards
		m_godrayBlurFBO->bind();
		glDisable(GL_DEPTH_TEST);
		mGodrayBlurProgram->use();

		gl->setUniform(mGodrayBlurProgram->getUniformLoc("lightPos"), lightPosInClip.getXY());
		gl->setUniform(mGodrayBlurProgram->getUniformLoc("bwSmapler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_godrayFBO->getTexture(0));

		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		m_godrayBlurFBO->unbind();
		glEnable(GL_DEPTH_TEST);*/


	}


	void TessellationTestScene::setupMissile() {
		mRocketMesh.reset((Mesh<VertexPNTC>*)importMesh("assets/AVMT300/AVMT300sub.obj"));
		Vec3f lo, hi;
		mRocketMesh->getBBox(lo, hi);
		mRocketMesh->xform(Mat4f::translate(Vec3f(0.0, 0.0, -13.0f))*rot(Vec3f(0.0f, 0.5f*FW_PI, 0.0f)) * Mat4f::translate((lo + hi) * -0.5f));


		Array<Vec3i> idx = mRocketMesh->indices(0);
		size_t numTriangles = idx.getSize();
		std::sort(idx.getPtr(0), idx.getPtr(numTriangles), [this](const Vec3i & trig1, const Vec3i & trig2) {

			const VertexPNTC &v0 = mRocketMesh->vertex(trig1.x),
				&v1 = mRocketMesh->vertex(trig1.y),
				&v2 = mRocketMesh->vertex(trig1.z);

			const VertexPNTC &q0 = mRocketMesh->vertex(trig2.x),
				&q1 = mRocketMesh->vertex(trig2.y),
				&q2 = mRocketMesh->vertex(trig2.z);

			Vec3f vavg = v0.p + v1.p + v2.p;
			Vec3f qavg = q0.p + q1.p + q2.p;

			return vavg.z > qavg.z;
		});


		std::vector<MissileMeshVertex> missleVertices(numTriangles * 3);

		size_t k = 0;
		for (size_t i = 0; i < numTriangles; ++i) {
			const VertexPNTC &v0 = mRocketMesh->vertex(idx[i][0]),
				&v1 = mRocketMesh->vertex(idx[i][1]),
				&v2 = mRocketMesh->vertex(idx[i][2]);
			missleVertices[k++] = MissileMeshVertex(v0.p, v0.n, v0.t.x, v0.t.y);
			missleVertices[k++] = MissileMeshVertex(v1.p, v1.n, v1.t.x, v1.t.y);
			missleVertices[k++] = MissileMeshVertex(v2.p, v2.n, v2.t.x, v2.t.y);
		}

		glGenVertexArrays(1, &mMissileVAO);
		glBindVertexArray(mMissileVAO);

		glGenBuffers(1, &mMissileVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mMissileVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(MissileMeshVertex) * missleVertices.size(), missleVertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), (char*)NULL + sizeof(Vec4f));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);

		mRocketMeshNumVertices = missleVertices.size();

		MeshBase::Material mat = mRocketMesh->material(0);

		Texture tex = mat.textures[MeshBase::TextureType_Diffuse];
		mMissleTexture = tex.getGLTexture();
		mMissileSpecular = Vec4f(mat.specular, mat.glossiness);
	}

	void TessellationTestScene::renderMissile(GLContext * gl, const Mat4f & toScreen, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & lightPos, const Vec3f & lightDir, const Vec3f & cameraPos) {

		static const Mat4f scaleMatrix = Mat4f::scale(Vec3f(20.0f, 20.0f, 29.0f));
		mMissileProgram->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mMissleTexture);
		gl->setUniform(mMissileProgram->getUniformLoc("diffuseTex"), 0);
		gl->setUniform(mMissileProgram->getUniformLoc("specularColor"), mMissileSpecular);

		gl->setUniform(mMissileProgram->getUniformLoc("specularColor"), mMissileSpecular);
		gl->setUniform(mMissileProgram->getUniformLoc("lightColor"), lightColor);
		gl->setUniform(mMissileProgram->getUniformLoc("seaColor"), seaColor);
		gl->setUniform(mMissileProgram->getUniformLoc("lightPos"), lightPos);
		gl->setUniform(mMissileProgram->getUniformLoc("lightDir"), lightDir);
		gl->setUniform(mMissileProgram->getUniformLoc("cameraPosition"), cameraPos);

		for (size_t i = 0; i < 1; ++i)
		{
			int endRibbon = FW::min(int(FWSync::ribbonEnd), int(mRibbonCurves[i].size())-1);

			Mat4f curveMatrix;
			CurvePoint dPoint = mRibbonCurves[i][endRibbon];
			curveMatrix.setCol(0, Vec4f(dPoint.B, 0.0f));
			curveMatrix.setCol(1, Vec4f(dPoint.N, 0.0f));
			curveMatrix.setCol(2, Vec4f(dPoint.T, 0.0f));
			curveMatrix.setCol(3, Vec4f(dPoint.V, 1.0f));

			Mat4f toWorld = curveMatrix * scaleMatrix;
			Mat4f normalToWorld = toWorld.inverted().transposed();


			gl->setUniform(mMissileProgram->getUniformLoc("toScreen"), toScreen*toWorld);
			gl->setUniform(mMissileProgram->getUniformLoc("toWorld"), toWorld);
			gl->setUniform(mMissileProgram->getUniformLoc("normalToWorld"), normalToWorld);

			glBindVertexArray(mMissileVAO);

			glDrawArrays(GL_TRIANGLES, 0, FW::min(int(FWSync::rocketTriangles), mRocketMeshNumVertices));

			glBindVertexArray(0);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void TessellationTestScene::renderMissileLight(GLContext * gl, const Mat4f & toScreen) {

		static const Mat4f scaleMatrix = Mat4f::scale(Vec3f(20.0f, 20.0f, 29.0f));
		mMissileLightProgram->use();

		glBindFragDataLocation(mMissileLightProgram->getHandle(), 0, "position");
		for (size_t i = 0; i < 1; ++i)
		{
			int endRibbon = FW::min(int(FWSync::ribbonEnd), int(mRibbonCurves[i].size()) - 1);

			Mat4f curveMatrix;
			CurvePoint dPoint = mRibbonCurves[i][endRibbon];
			curveMatrix.setCol(0, Vec4f(dPoint.B, 0.0f));
			curveMatrix.setCol(1, Vec4f(dPoint.N, 0.0f));
			curveMatrix.setCol(2, Vec4f(dPoint.T, 0.0f));
			curveMatrix.setCol(3, Vec4f(dPoint.V, 1.0f));

			Mat4f toWorld = curveMatrix * scaleMatrix;

			gl->setUniform(mMissileLightProgram->getUniformLoc("toScreen"), toScreen*toWorld);
			gl->setUniform(mMissileLightProgram->getUniformLoc("toWorld"), toWorld);

			glBindVertexArray(mMissileVAO);

			glDrawArrays(GL_TRIANGLES, 0, FW::min(int(FWSync::rocketTriangles), mRocketMeshNumVertices));

			glBindVertexArray(0);
		}

	}

	Mat4f TessellationTestScene::getCityMeshWorldMatrix() {
		return Mat4f::scale(Vec3f(50.0f)) * Mat4f::translate(Vec3f(50.0f, 20.0f, 66.0f));
	}

	void TessellationTestScene::loadSubmarine() {

		mSubmarineMesh = (Mesh<VertexPNTC>*)FW::importMesh("assets/Submarine/submarine.obj");
		std::vector<ParticleMaterial> materials(mSubmarineMesh->numSubmeshes());

		// texture handle, location
		std::unordered_map<GLuint, int> textureMap;
		std::unordered_map<GLuint, int>::iterator textureMapIterator;

		int numTriangles = mSubmarineMesh->numTriangles();
		int particlesPerTriangle = 100;
		int cParticle = 0;

		mNumSubmarineParticles = particlesPerTriangle * numTriangles;
		Random rnd;

		std::vector<ParticleInfo> particleData(mNumSubmarineParticles);

		for (int i = 0; i < mSubmarineMesh->numSubmeshes(); ++i)
		{

			// copy material
			const auto & cMaterial = mSubmarineMesh->material(i);;

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

			const Array<Vec3i>& idx = mSubmarineMesh->indices(i);
			for (int j = 0; j < idx.getSize(); ++j)
			{

				const VertexPNTC &v0 = mSubmarineMesh->vertex(idx[j][0]),
					&v1 = mSubmarineMesh->vertex(idx[j][1]),
					&v2 = mSubmarineMesh->vertex(idx[j][2]);



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



		glGenVertexArrays(1, &mSubmarineVAO);
		glBindVertexArray(mSubmarineVAO);

		glGenBuffers(1, &mSubmarineVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mSubmarineVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleInfo) * mNumSubmarineParticles, particleData.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + sizeof(Vec4f)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + 2 * sizeof(Vec4f)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	void TessellationTestScene::renderSubmarine(GLContext * gl, const Mat4f & toScreen, const Vec3f & lightPosition, const Vec3f & lightDirection, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & camPos)
	{
		if (FWSync::submarineExplode>0.0f) explodeSubmarine(gl);
		Mat4f toWorld = getSubmarineToWorldMatrix();

		mSubmarineParticleProgram->use();
		gl->setUniform(mSubmarineParticleProgram->getUniformLoc("toScreen"), toScreen*toWorld);
		gl->setUniform(mSubmarineParticleProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mSubmarineParticleProgram->getUniformLoc("normalToWorld"), toWorld.inverted().transposed());
		gl->setUniform(mSubmarineParticleProgram->getUniformLoc("lightPos"), lightPosition);
		gl->setUniform(mSubmarineParticleProgram->getUniformLoc("lightDirection"), lightDirection);
		gl->setUniform(mSubmarineParticleProgram->getUniformLoc("cameraPosition"), camPos);
		gl->setUniform(mSubmarineParticleProgram->getUniformLoc("lightColor"), lightColor);
		gl->setUniform(mSubmarineParticleProgram->getUniformLoc("seaColor"), seaColor);
		for (size_t i = 0; i < mTextureHandles.size(); ++i) {
			if (!glIsTextureHandleResidentARB(mTextureHandles[i])) glMakeTextureHandleResidentARB(mTextureHandles[i]);
			// bound to unit
			char buf[32];
			sprintf_s(buf, "diffuseSamplers[%d]", int(i));
			GLint loc = mSubmarineParticleProgram->getUniformLoc(buf);
			glUniformHandleui64ARB(loc, mTextureHandles[i]);
		}

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleMaterialSSBO);

		glBindVertexArray(mSubmarineVAO);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDrawArrays(GL_POINTS, 0, mNumSubmarineParticles);
		glBindVertexArray(0);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

		for (size_t i = 0; i < mTextureHandles.size(); ++i) {
			if (glIsTextureHandleResidentARB(mTextureHandles[i])) {
				glMakeTextureHandleNonResidentARB(mTextureHandles[i]);
			}
		}
	}

	Mat4f TessellationTestScene::getSubmarineToWorldMatrix() {
		Mat4f toWorld;
		toWorld = Mat4f::translate(Vec3f(4521.34 + FWSync::submarineOffset, 8622.39, 752.158)) * Mat4f::scale(Vec3f(2.5f));
		return toWorld;
	}

	void TessellationTestScene::explodeSubmarine(GLContext * gl) {
		mSubmarineSDFMoveProgram->use();

		gl->setUniform(mSubmarineSDFMoveProgram->getUniformLoc("numParticles"), mNumSubmarineParticles);

		gl->setUniform(mSubmarineSDFMoveProgram->getUniformLoc("dtUniform"), GLOBAL_DT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSubmarineVBO);

		int localSizeX = 128;

		int groupSizeX = (mNumSubmarineParticles + localSizeX - 1) / localSizeX;

		glDispatchCompute(groupSizeX, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	}

	void TessellationTestScene::loadCamPaths()
	{
		size_t numCamPaths = 3;
		mCameraPaths.resize(numCamPaths);
		for (size_t i = 0; i < numCamPaths; ++i)
		{
			std::string filePath = "assets/cam_path_water_" + std::to_string(i + 1) + ".txt";
			loadCamPath(filePath, mCameraPaths[i]);
		}
	}

	void TessellationTestScene::loadCamPath(const std::string & fileName, std::vector<Vec3f> & vec) {

		std::ifstream in(fileName);

		size_t numPoints;
		in >> numPoints;
		vec.resize(numPoints);
		for (size_t i = 0; i < numPoints; ++i) {
			Vec3f p;
			in >> p.x >> p.y >> p.z;
			vec[i] = p;
		}
	}

	Vec3f TessellationTestScene::getCameraPosition() {
		float t = FWSync::cameraTime;
		int idx = int(t);
		int camIndex = FWSync::cameraIndex;
		t -= float(idx);
		if (idx + 3 >= mCameraPaths[camIndex].size()) {
			idx = mCameraPaths[camIndex].size() - 4;
			t = 1.0f;
		}
		Vec3f off = CarmullRomCurve::evalCatmullRom(
			mCameraPaths[camIndex][idx],
			mCameraPaths[camIndex][idx + 1],
			mCameraPaths[camIndex][idx + 2],
			mCameraPaths[camIndex][idx + 3], t);
		return off;
	}

	Vec3f TessellationTestScene::getCameraForward() {
		int camIndex = FWSync::cameraIndex;
		float ribbonT = FWSync::ribbonEnd;
		int startRibbon = clamp(int(ribbonT), 1, int(mRibbonCurves[camIndex].size())-1);
		float t = ribbonT;
		t -= int(ribbonT);
		Vec3f ribbonHead;
		Vec3f prevRibbonHead;
		if (startRibbon < mRibbonCurves[camIndex].size()) {
			ribbonHead = mRibbonCurves[camIndex][startRibbon].V;
			prevRibbonHead = mRibbonCurves[camIndex][startRibbon - 1].V;
		}
		else
		{
			ribbonHead = mRibbonCurves[camIndex].back().V;
			prevRibbonHead = mRibbonCurves[camIndex][mRibbonCurves[camIndex].size()-2].V;
		}

		Vec3f interpolatedPoint = FW::lerp(prevRibbonHead, ribbonHead, t);

		return normalize(interpolatedPoint -mCamPtr->getPosition());
	}

	void TessellationTestScene::renderSurfaces(GLContext * gl, const Mat4f & toScreen, const Vec3f & lightPosition, const Vec3f & lightDirection, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & camPos)
	{
		
		mMeshCurveRenderProgram->use();

		Mat4f toWorld;
		toWorld.setIdentity();

		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("normalToWorld"), toWorld.inverted().transposed());
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightPos"), lightPosition);
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightDirection"), lightDirection);
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("cameraPosition"), camPos);
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightColor"), lightColor);
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("seaColor"), seaColor);
		for (size_t i = 0; i < mCurveSurfacesVAO.size(); ++i)
		{
			glBindVertexArray(mCurveSurfacesVAO[i]);
			
			int s = int(FWSync::ribbonStart)*mProfileNumIndices*6;
			int e = int(FWSync::ribbonEnd)*mProfileNumIndices*6;
			glDrawElements(GL_TRIANGLES, e-s, GL_UNSIGNED_INT, NULL + (char*)(s*sizeof(GL_UNSIGNED_INT)));

		}
		glBindVertexArray(0);
	}

	void TessellationTestScene::renderSurfacesLight(GLContext * gl, const Mat4f & toScreen)
	{

		mMeshCurveRenderLightProgram->use();

		gl->setUniform(mMeshCurveRenderLightProgram->getUniformLoc("toScreen"), toScreen);

		for (size_t i = 0; i < mCurveSurfacesVAO.size(); ++i)
		{
			glBindVertexArray(mCurveSurfacesVAO[i]);

			int s = int(FWSync::ribbonStart)*mProfileNumIndices * 6;
			int e = int(FWSync::ribbonEnd)*mProfileNumIndices * 6;
			glDrawElements(GL_TRIANGLES, e - s, GL_UNSIGNED_INT, NULL + (char*)(s * sizeof(GL_UNSIGNED_INT)));

		}
		glBindVertexArray(0);
	}

	void TessellationTestScene::renderAuthorLogos(GLContext * gl, const Mat4f & toScreen, const Vec3f & lightPosition, const Vec3f & lightDirection, const Vec3f & lightColor, const Vec3f & seaColor, const Vec3f & camPos)
	{

		Mat4f cceToWorld = Mat4f::translate(Vec3f(3899, 5751, 1117) + m_knobs[5].getXYZ()) * Mat4f::scale(Vec3f(40)) *rot(Vec3f(0, 2.24, 0));

		mAuthorRenderProgram->use();
		gl->setUniform(mAuthorRenderProgram->getUniformLoc("toWorld"), cceToWorld);
		gl->setUniform(mAuthorRenderProgram->getUniformLoc("normalToWorld"), cceToWorld.transposed().inverted());

		gl->setUniform(mAuthorRenderProgram->getUniformLoc("toScreen"), toScreen*cceToWorld);
		gl->setUniform(mAuthorRenderProgram->getUniformLoc("seaColor"), seaColor);
		gl->setUniform(mAuthorRenderProgram->getUniformLoc("lightColor"), lightColor);
		gl->setUniform(mAuthorRenderProgram->getUniformLoc("lightDirection"), lightDirection);
		gl->setUniform(mAuthorRenderProgram->getUniformLoc("lightPosition"), lightPosition);
		gl->setUniform(mAuthorRenderProgram->getUniformLoc("cameraPosition"), camPos);
		renderMesh(gl, mCceMesh.get(), mAuthorRenderProgram);

		
	}

	void TessellationTestScene::renderFlares(GLContext * gl, const Mat4f & toScreen, const Vec3f & cameraPosition, const Vec3f & cameraUp, const Vec3f & cameraHorizontal)
	{
		mBillboardLensProgram->use();

		//glDisable(GL_DEPTH_TEST);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mLensFlareParticleTex);
		gl->setUniform(mBillboardLensProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mBillboardLensProgram->getUniformLoc("flareTex"), 0);
		Vec4f flarePositions[4];

		int camIndex = FWSync::cameraIndex;
		float ribbonT = FWSync::ribbonEnd+1.0f;
		for (size_t i = 0; i < 4; ++i)
		{
			int startRibbon = clamp(int(ribbonT), 1, int(mRibbonCurves[i].size()) - 1);
			float t = ribbonT;
			t -= int(ribbonT);
			Vec3f ribbonHead;
			Vec3f prevRibbonHead;

			if (startRibbon < mRibbonCurves[i].size()) {
				ribbonHead = mRibbonCurves[i][startRibbon].V;
				prevRibbonHead = mRibbonCurves[i][startRibbon - 1].V;
			}
			else
			{
				ribbonHead = mRibbonCurves[i].back().V;
				prevRibbonHead = mRibbonCurves[i][mRibbonCurves[i].size() - 2].V;
			}

			Vec3f interpolatedPoint = FW::lerp(prevRibbonHead, ribbonHead, t);
			flarePositions[i] = Vec4f(interpolatedPoint, 1);

			
		}
		glBindVertexArray(mFlareParticlesVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mFlareParticlesVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vec4f) * 4, flarePositions[0].getPtr(), GL_DYNAMIC_DRAW);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
		glEnable(GL_POINT_SPRITE);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		
		glDrawArrays(GL_POINTS, 0, 4);
		
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDisable(GL_BLEND);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		//glEnable(GL_DEPTH_TEST);
	}

};