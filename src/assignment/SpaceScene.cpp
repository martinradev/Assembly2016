#include "SpaceScene.h"
#include "gui/Image.hpp"
#include "ParticleModelNBody.h"
#include "Util.h"
#include "Globals.h"
#include "SyncVars.h"
#include "IntersectionTest.h"
#include "TessellationTestScene.h"
#include <fstream>
#include <algorithm>
#include <unordered_map>

namespace FW {

	SpaceScene::SpaceScene(GLContext * ctx, int width, int height, FBO * lastPass, CameraControls * camPtr)
		:
		mLastFBO(lastPass),
		mRenderNefertitiProgramName("render_nefertiti_head"),
		mRenderParticleCloudProgramName("render_particle_cloud_nefertiti"),
		mCombineProgramName("combine_nefertiti"),
		mDisplayProgramName("nefertiti_display"),
		mBackgroundProgramName("background_nefertiti"),
		mParticleCloudMoveProgramName("particle_cloud_nefertiti_move"),
		mNefertitiMoveProgramName("nefertiti_move_program"),
		mMeshObjectProgramName("nefertiti_object_mesh"),
		mCamPtr(camPtr),
		mDebugRenderCurve(false),
		mSelectedControlPointIndex(-1),
		mSelectedCurveIndex(-1),
		mWndSize(Vec2i(width, height))
	{

		loadShaders(ctx);
		
		GLuint fboDepthTex = TEXTURE_POOL->request(TextureDescriptor(GL_DEPTH_COMPONENT32F, width, height, GL_DEPTH_COMPONENT, GL_FLOAT))->m_texture;

		GLuint nefertitiColorTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;
		GLuint nefertitiNormalTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;
		GLuint nefertitiPositionTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;

		mNefertitiFBO.reset(new FBO(fboDepthTex));
		mNefertitiFBO->attachTexture(0, nefertitiColorTex);
		mNefertitiFBO->attachTexture(1, nefertitiNormalTex);
		mNefertitiFBO->attachTexture(2, nefertitiPositionTex);

		//mSkyBoxFBO.reset(new FBO(fboDepthTex));
		//mSkyBoxFBO->attachTexture(0, nefertitiColorTex);

		setupGLBuffers();

		loadNefertiti();

		mMesher.reset(new Mesher(ctx, Vec4f(-20.0f, -20.0f, -20.0f, 0.625f), "shaders/nefertiti_scene/sdf.glsl"));

		loadRibbons();

		

		
		mGodrayFBO.reset(new FBO(fboDepthTex));
		mGodrayFBO->attachTexture(0, nefertitiColorTex);
		GLuint godrayColorTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;
		mGodrayBlurFBO.reset(new FBO(fboDepthTex));
		mGodrayBlurFBO->attachTexture(0, godrayColorTex);

		mGodrayBlurTex = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT))->m_texture;

		mGaussiaFilter.reset(new GaussianFilter(ctx, mWndSize));

		loadSkybox();
	}


	void SpaceScene::render(Window & wnd, const CameraControls & camera) {
		mCamPtr->setNear(30.0f);
		mCamPtr->setFar(42000.0f);
		GLContext * gl = wnd.getGL();


		moveParticleCloud(gl);

		if (FWSync::nefertitiParticleStep > 0.0f) {
		moveNefertiti(gl);
		}

		GLContext::Program * mesherProg = mMesher->getProgram();

		mCamPtr->setPosition(getCameraPosition());
		mCamPtr->setForward(getCameraForward());

		if (FWSync::nefertitiParticleStep == 0.0f)
		{
			mesherProg->use();
			gl->setUniform(mesherProg->getUniformLoc("knob0"), Vec4f(FWSync::sdf1, FWSync::sdf2, FWSync::sdf3, FWSync::sdf4));
			gl->setUniform(mesherProg->getUniformLoc("knob1"), Vec4f(FWSync::sdf5, FWSync::sdf6, FWSync::sdf7, FWSync::sdf8));
			gl->setUniform(mesherProg->getUniformLoc("knob2"), Vec4f(FWSync::sdf9, FWSync::sdf10, FWSync::sdf11, FWSync::sdf12));
			mMesher->update(gl);
		}
		Vec3f cameraPosition = camera.getPosition();
		Mat4f toScreen = camera.getWorldToClip();
		Mat4f orientation = camera.getWorldToCamera();
		orientation.setCol(3, Vec4f(0, 0, 0, 1));
		Mat4f cameraToClip = camera.getCameraToClip();

		Mat4f nefertitiToWorld = Mat4f::translate(Vec3f(800, 0, -4500)) * Mat4f::scale(Vec3f(450, 400, 450));
		Mat4f nefertitiToScreen = toScreen * nefertitiToWorld;
		Mat4f meshToWorld = Mat4f::translate(Vec3f(-1000, 0, 2000)) * Mat4f::scale(Vec3f(90.0f));
		Mat4f meshToScreen = toScreen * meshToWorld;
		Mat4f cloudToWorld = Mat4f::translate(Vec3f(800, 0, -4500)) * Mat4f::scale(Vec3f(160, 110, 160));
		Mat4f cloudToScreen = toScreen * cloudToWorld;

		Mat4f ribbonsToScreen = toScreen;

		if (FWSync::nefertitiParticleStep == 0.0f)
		{
			godrayPass(gl, nefertitiToScreen, cloudToScreen, meshToScreen, ribbonsToScreen);
		}

		renderBackground(gl);

		
		
		renderNefertiti(gl, nefertitiToScreen, nefertitiToWorld, nefertitiToWorld.inverted().transposed(), cameraPosition);

		renderRibbons(gl, ribbonsToScreen, cameraPosition);

		
		renderStatueParticles(gl, cloudToScreen, cloudToWorld, cloudToWorld.inverted().transposed(), cameraPosition);

		if (FWSync::nefertitiParticleStep == 0.0f)
		{
			renderMeshObject(gl, meshToScreen, meshToWorld, meshToWorld.inverted().transposed(), cameraPosition);
		}
		

		mLastFBO->bind();

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mCombineProgram->use();

		gl->setUniform(mCombineProgram->getUniformLoc("meshColorTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mNefertitiFBO->getTexture(0));


		gl->setUniform(mCombineProgram->getUniformLoc("godrayColorTex"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, mGodrayBlurTex);

		glBindVertexArray(mQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		
		
		mLastFBO->unbind();

	}

	void SpaceScene::renderNefertiti(GLContext *gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition)
	{

		mNefertitiFBO->bind();

		//glClearColor(0, 0, 0, 1);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mRenderNefertitiProgram->use();

		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("normalToWorld"), normalToWorld);
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("cameraPos"), cameraPosition);
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("lightPos[0]"), getLightPosition(0));
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("lightColor[0]"), getLightColor(0));
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("lightDirection[0]"), getLightDirection(0));
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("lightPos[1]"), getLightPosition(1));
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("lightColor[1]"), getLightColor(1));
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("lightDirection[1]"), getLightDirection(1));
		gl->setUniform(mRenderNefertitiProgram->getUniformLoc("AlphaUniform"), FWSync::nefertitiAlpha);
		//gl->setUniform(mRenderNefertitiProgram->getUniformLoc("skyBox"), 0);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBoxTex);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleMaterialSSBO);
		glBindVertexArray(mNeferittiVAO);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glEnable(GL_POINT_SMOOTH);
		if (FWSync::nefertitiAlpha>0.0f)glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, mNumNefertitiParticles);
		if (FWSync::nefertitiAlpha>0.0f)glDisable(GL_BLEND);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDisable(GL_POINT_SMOOTH);
		glBindVertexArray(0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);

		if (mDebugRenderCurve)
		{
			debugRenderRibbons();
		}

		mNefertitiFBO->unbind();
	}

	void SpaceScene::setupGLBuffers() {
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

		/*GLfloat skyboxVertices[] = {
			// Positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			1.0f,  1.0f, -1.0f,
			1.0f,  1.0f,  1.0f,
			1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f, -1.0f,
			1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			1.0f, -1.0f,  1.0f
		};

		glGenVertexArrays(1, &mSkyBoxVAO);
		glGenBuffers(1, &mSkyBoxVBO);
		glBindVertexArray(mSkyBoxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, mSkyBoxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 18 * 6, skyboxVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		*/
		glGenTextures(1, &mDynamicSkyBoxTex);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mDynamicSkyBoxTex);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, false);

		for (int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		Image * img = importImage("assets/skybox/nebula.png");
		mBackgroundTex = img->createGLTexture();
		delete img;

		generateParticleCloud();
	}

	void SpaceScene::loadShaders(GLContext * ctx) {

		if (mMesher != nullptr) mMesher->loadShaders(ctx);

		const char meshObjectVertex[] = "shaders/nefertiti_scene/mesh_object_vert.glsl";
		const char meshObjectFragment[] = "shaders/nefertiti_scene/mesh_object_frag.glsl";

		mMeshObjectProgram = loadShader(ctx, meshObjectVertex, meshObjectFragment, mMeshObjectProgramName);

		const char nefertitiHeadVertex[] = "shaders/nefertiti_scene/render_nefertiti_vert.glsl";
		const char nefertitiHeadFragment[] = "shaders/nefertiti_scene/render_nefertiti_frag.glsl";

		mRenderNefertitiProgram = loadShader(ctx, nefertitiHeadVertex, nefertitiHeadFragment, mRenderNefertitiProgramName);

		const char nefertitiParticleCloudVert[] = "shaders/nefertiti_scene/render_particle_cloud_vert.glsl";
		const char nefertitiParticleCloudFrag[] = "shaders/nefertiti_scene/render_particle_cloud_frag.glsl";

		mRenderParticleCloudProgram = loadShader(ctx, nefertitiParticleCloudVert, nefertitiParticleCloudFrag, mRenderParticleCloudProgramName);

		const char displayVertex[] = "shaders/common/display_vertex.glsl";
		const char displayFragment[] = "shaders/common/display_fragment.glsl";

		mDisplayProgram = loadShader(ctx, displayVertex, displayFragment, mDisplayProgramName);

		const char backgroundVertex[] = "shaders/nefertiti_scene/background_vertex.glsl";
		const char backgroundFragment[] = "shaders/nefertiti_scene/background_fragment.glsl";

		mBackgroundProgram = loadShader(ctx, backgroundVertex, backgroundFragment, mBackgroundProgramName);

		const char combineVertex[] = "shaders/nefertiti_scene/combine_vertex.glsl";
		const char combineFragment[] = "shaders/nefertiti_scene/combine_fragment.glsl";

		mCombineProgram = loadShader(ctx, combineVertex, combineFragment, mCombineProgramName);

		mParticleCloudMoveProgram = loadShader(ctx, "shaders/nefertiti_scene/particle_cloud_cs.glsl", mParticleCloudMoveProgramName);

		mNefertitiMoveProgram = loadShader(ctx, "shaders/nefertiti_scene/particle_nefertiti_cs.glsl", mNefertitiMoveProgramName);

		/*const char skyboxVertex[] = "shaders/skybox/render_vert.glsl";
		const char skyboxFragment[] = "shaders/skybox/render_frag.glsl";

		mSkyboxProgram = loadShader(ctx, skyboxVertex, skyboxFragment, mSkyboxProgramName);*/

		mMeshCurveRenderProgram = loadShader(ctx, "shaders/nefertiti_scene/surface_vert.glsl", 
			"shaders/nefertiti_scene/surface_frag.glsl", "nefertiti_surface_program_asd");

		mGodrayProgram = loadShader(ctx, "shaders/nefertiti_scene/godray_pass_vert.glsl",
			"shaders/nefertiti_scene/godray_pass_frag.glsl", "nefertiti_godray_geometry");

		mParticleGodrayProgram = loadShader(ctx, "shaders/nefertiti_scene/godray_pass_particle_vert.glsl",
			"shaders/nefertiti_scene/godray_pass_frag.glsl", "nefertiti_godray_particle_geometry");

		mGodrayBlurProgram = loadShader(ctx, "shaders/common/display_vertex.glsl",
			"shaders/nefertiti_scene/godray_blur_frag.glsl", "godray_blur_nefertiti");
	}

	


	void SpaceScene::loadNefertiti()
	{
		mNeferittiMesh = (Mesh<VertexPNTC>*)FW::importMesh("assets/nefertiti/nefertiti5.obj");


		std::vector<ParticleMaterial> materials(mNeferittiMesh->numSubmeshes());

		// texture handle, location
		std::unordered_map<GLuint, int> textureMap;
		std::unordered_map<GLuint, int>::iterator textureMapIterator;

		int numTriangles = mNeferittiMesh->numTriangles();
		int particlesPerTriangle = 30;
		int cParticle = 0;

		mNumNefertitiParticles = particlesPerTriangle * numTriangles;
		Random rnd;

		std::vector<ParticleInfo> particleData(mNumNefertitiParticles);


		for (int i = 0; i < mNeferittiMesh->numSubmeshes(); ++i)
		{

			// copy material
			const auto & cMaterial = mNeferittiMesh->material(i);;

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

			const Array<Vec3i>& idx = mNeferittiMesh->indices(i);
			for (int j = 0; j < idx.getSize(); ++j)
			{

				const VertexPNTC &v0 = mNeferittiMesh->vertex(idx[j][0]),
					&v1 = mNeferittiMesh->vertex(idx[j][1]),
					&v2 = mNeferittiMesh->vertex(idx[j][2]);



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



		glGenVertexArrays(1, &mNeferittiVAO);
		glBindVertexArray(mNeferittiVAO);

		glGenBuffers(1, &mNefertitiVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mNefertitiVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleInfo) * mNumNefertitiParticles, particleData.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + sizeof(Vec4f)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + 2 * sizeof(Vec4f)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);


		std::vector<Vec4f> attractors;
		attractors.push_back(Vec4f(0, 0, 0, 50));
		attractors.push_back(Vec4f(1.0, 1.0, -1, -50));
		attractors.push_back(Vec4f(3, 2.5, -2, -50));
		attractors.push_back(Vec4f(-3, -2, 1, 50));
		attractors.push_back(Vec4f(5, -4, 1, 50));
		attractors.push_back(Vec4f(4, 4, 3, -50));
		attractors.push_back(Vec4f(4, 0, 4, 50));
		mNumAttractors = attractors.size();
		glGenBuffers(1, &mAttractorSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mAttractorSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vec4f) * attractors.size(), attractors.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void SpaceScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		case Action::Action_RightButton:
			selectControlPoint(wnd.getMousePos());
			break;
		case Action::Action_SaveRibbonPath:
			saveRibbonPaths();
			break;
		default:
			break;
		}

	}
	
	void SpaceScene::activate(Window & wnd, CommonControls & controls) {

		updateGUI(wnd, controls);

	}

	void SpaceScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		controls.removeControl(&actionExt);
	}

	void SpaceScene::updateGUI(Window & wnd, CommonControls & controls) {

		cleanUpGUI(wnd, controls);

		controls.addSeparator();


		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(-20.0f), Vec4f(20.0f)), // knob1
			std::make_pair(Vec4f(-20.0f), Vec4f(20.0f)), // knob2
			std::make_pair(Vec4f(-1.0f), Vec4f(1.0f)), // knob3
			std::make_pair(Vec4f(0.0f), Vec4f(1.0f)), // knob4
			std::make_pair(Vec4f(-10.0f), Vec4f(10.0f)), // knob5
			std::make_pair(Vec4f(-1200.0f), Vec4f(1200.0f)), // knob6
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
		controls.addButton((S32*)&actionExt, (S32)Action::Action_SaveRibbonPath, FW_KEY_NONE, "Save p");
	}

	Vec3f SpaceScene::getLightPosition(int i)
	{
		switch (i)
		{
		case 0:
			return Vec3f(-30, 10, 12);
		case 1:
			return Vec3f(30.0, -10, -40);
		}
		return Vec3f(0.0f);
	}

	Mat4f SpaceScene::getLightMatrix(int i) {
		Mat4f mat;
		switch (i)
		{
		case 0:
			
			break;
		case 1:
			break;
			
		}
		return mat;
	}

	Vec3f SpaceScene::getLightColor(int i)
	{
		switch (i)
		{
		case 0:
			return Vec3f(0.0669, 0.01255, 0.15);
		case 1:
			return Vec3f(0.0334, 0.0167, 0.0794);
		}
		return Vec3f(0.0f);
	}

	Vec3f SpaceScene::getLightDirection(int i) {
		return (Vec3f(0) - getLightPosition(i)).normalized();
	}

	
	void SpaceScene::loadSkybox() {

		glGenTextures(1, &mSkyBoxTex);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBoxTex);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, true);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, FW_S32_MAX);
		for (int i = 0; i < 6; ++i)
		{
			std::string file = "assets/skybox/" + std::to_string(i) + ".png";
			Image * img = FW::importImage(file.c_str());
			ImageFormat format = img->getFormat();
			ImageFormat::ID formatID = format.getGLFormat();
			const ImageFormat::StaticFormat* sf = ImageFormat(formatID).getStaticFormat();
			Vec2i sz = img->getSize();
			
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, sf->glInternalFormat,
				sz.x, sz.y,
				0, sf->glFormat, sf->glType, img->getPtr());
		}
		
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


	}

	void SpaceScene::renderBackground(GLContext * gl) {

		glDisable(GL_DEPTH_TEST);
		mNefertitiFBO->bind();

		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mBackgroundProgram->use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mBackgroundTex);

		gl->setUniform(mBackgroundProgram->getUniformLoc("inTex"), 0);

		glBindVertexArray(mQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		mNefertitiFBO->unbind();
		glEnable(GL_DEPTH_TEST);
	}

	void SpaceScene::generateParticleCloud() {

		mParticleCloudNum = 10e+5;

		Random rnd;

		std::vector<ParticleInfo> particles(mParticleCloudNum);
		for (size_t i = 0; i < mParticleCloudNum; ++i)
		{
			float phi = rnd.getF32(0.0f, 1.0) * FW_PI * 2.0f;
			float alpha = rnd.getF32(-0.2, 0.2);
			float r = rnd.getF32(10.0f, 15.0f);
			
			float x = r * cos(alpha) * sin(phi);
			float y = r * sin(alpha) * sin(phi);
			float z = r * cos(phi);

			x += 0.5*rnd.getF32(-1.0f, 1.0);
			y += 0.5*rnd.getF32(-1.0f, 1.0);
			z += 0.5*rnd.getF32(-1.0f, 1.0);

			particles[i] = ParticleInfo(Vec4f(x, y, z, 1), Vec4f(Vec3f(x, y, z).normalized(), 0.0), 0);

		}

		glGenVertexArrays(1, &mParticleCloudVAO);
		glBindVertexArray(mParticleCloudVAO);

		glGenBuffers(1, &mParticleCloudVBO);
		glBindBuffer(GL_ARRAY_BUFFER, mParticleCloudVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(ParticleInfo) * mParticleCloudNum, particles.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)NULL);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + sizeof(Vec4f)));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(ParticleInfo), (void*)(NULL + 2 * sizeof(Vec4f)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	void SpaceScene::renderStatueParticles(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition)
	{
		mNefertitiFBO->bind();

		//glClearColor(0, 0, 0, 1);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mRenderParticleCloudProgram->use();

		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("normalToWorld"), normalToWorld);
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("cameraPos"), cameraPosition);
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("lightPos[0]"), getLightPosition(0));
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("lightColor[0]"), getLightColor(0));
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("lightDirection[0]"), getLightDirection(0));
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("lightPos[1]"), getLightPosition(1));
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("lightColor[1]"), getLightColor(1));
		gl->setUniform(mRenderParticleCloudProgram->getUniformLoc("lightDirection[1]"), getLightDirection(1));

		glBindVertexArray(mParticleCloudVAO);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);
		glDrawArrays(GL_POINTS, 0, mNumNefertitiParticles);
		glDisable(GL_BLEND);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glDisable(GL_POINT_SMOOTH);
		glBindVertexArray(0);

		mNefertitiFBO->unbind();
	}

	void SpaceScene::moveParticleCloud(GLContext * gl)
	{
		if (FWSync::cloudParticleStep <= 0.0f) return;
		mParticleCloudMoveProgram->use();
		gl->setUniform(mParticleCloudMoveProgram->getUniformLoc("numParticles"), mParticleCloudNum);

		gl->setUniform(mParticleCloudMoveProgram->getUniformLoc("dtUniform"), GLOBAL_DT);
		gl->setUniform(mParticleCloudMoveProgram->getUniformLoc("curlStep"), FWSync::cloudParticleCurlStep);
		gl->setUniform(mParticleCloudMoveProgram->getUniformLoc("integrationStep"), FWSync::cloudParticleStep);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mParticleCloudVBO);

		int localSizeX = 128;

		int groupSizeX = (mParticleCloudNum + localSizeX - 1) / localSizeX;

		glDispatchCompute(groupSizeX, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
	}

	void SpaceScene::moveNefertiti(GLContext * gl)
	{
		if (FWSync::nefertitiParticleStep <= 0.0f) return;
		mNefertitiMoveProgram->use();
		gl->setUniform(mNefertitiMoveProgram->getUniformLoc("numParticles"), mNumNefertitiParticles);
		gl->setUniform(mNefertitiMoveProgram->getUniformLoc("curlStep"), FWSync::nefertitiParticleCurlStep);
		gl->setUniform(mNefertitiMoveProgram->getUniformLoc("integrationStep"), FWSync::nefertitiParticleStep);
		gl->setUniform(mNefertitiMoveProgram->getUniformLoc("dtUniform"), GLOBAL_DT);
		gl->setUniform(mNefertitiMoveProgram->getUniformLoc("numAttractors"), mNumAttractors);
		gl->setUniform(mNefertitiMoveProgram->getUniformLoc("attractorPower"), FWSync::attractorPower);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mNefertitiVBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mAttractorSSBO);
		int localSizeX = 128;

		int groupSizeX = (mNumNefertitiParticles + localSizeX - 1) / localSizeX;

		glDispatchCompute(groupSizeX, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	}

	void SpaceScene::renderMeshObject(GLContext * gl, const Mat4f & toScreen, const Mat4f & toWorld, const Mat4f & normalToWorld, const Vec3f & cameraPosition) {

		mNefertitiFBO->bind();

		mMeshObjectProgram->use();

		gl->setUniform(mMeshObjectProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mMeshObjectProgram->getUniformLoc("posToWorld"), toWorld);
		gl->setUniform(mMeshObjectProgram->getUniformLoc("normalToWorld"), normalToWorld);
		gl->setUniform(mMeshObjectProgram->getUniformLoc("cameraPos"), cameraPosition);
		gl->setUniform(mMeshObjectProgram->getUniformLoc("lightPos[0]"), getLightPosition(0));
		gl->setUniform(mMeshObjectProgram->getUniformLoc("lightColor[0]"), getLightColor(0));
		gl->setUniform(mMeshObjectProgram->getUniformLoc("lightDirection[0]"), getLightDirection(0));
		gl->setUniform(mMeshObjectProgram->getUniformLoc("lightPos[1]"), getLightPosition(1));
		gl->setUniform(mMeshObjectProgram->getUniformLoc("lightColor[1]"), getLightColor(1));
		gl->setUniform(mMeshObjectProgram->getUniformLoc("lightDirection[1]"), getLightDirection(1));

		gl->setUniform(mMeshObjectProgram->getUniformLoc("envMapTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mSkyBoxTex);

		glBindBuffer(GL_ARRAY_BUFFER, mMesher->getTriangleVBO());

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), 0);

		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), (GLvoid*)((char*)NULL + sizeof(float) * 8));
		int numTrigs = mMesher->getNumTriangles();
		if (0<numTrigs)glDrawArrays(GL_TRIANGLES, 0, numTrigs);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		mNefertitiFBO->unbind();
	}

	Vec3f SpaceScene::getCameraForward()
	{
		return normalize(Vec3f(0) - getCameraPosition());
	}

	Vec3f SpaceScene::getCameraPosition()
	{
		return Vec3f(-931,5,6189);
	}

	void SpaceScene::loadRibbonPath(const std::string & fileName, std::vector<Vec3f> & vec) {

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

	void SpaceScene::loadRibbons()
	{
		static const int ribbonsCount = 2;

		mControlPoints.resize(ribbonsCount);

		for (size_t i = 0; i < ribbonsCount; ++i) {
			std::string filePath = "assets/space_ribbon_path_" + std::to_string(i + 1) + ".txt";
			loadRibbonPath(filePath, mControlPoints[i]);
		}

		std::vector<Vec3f> starControlPoints;
		loadRibbonPath("assets/star_curve.txt", starControlPoints);

		for (size_t i = 0; i < starControlPoints.size(); ++i)
		{
			starControlPoints[i] *= 68.0;
		}

		Curve starCurve = evalCatmullRomspline(starControlPoints, 30, false, 0.0, 0.0);
		mProfileNumIndices = starCurve.size();

		mRibbonVAOs.resize(mControlPoints.size());
		mRibbonVBOs.resize(mControlPoints.size());
		mRibbonIBOs.resize(mControlPoints.size());
		mRibbonNumTriangles.resize(mControlPoints.size());

		glGenVertexArrays(mRibbonVAOs.size(), mRibbonVAOs.data());
		glGenBuffers(mRibbonVBOs.size(), mRibbonVBOs.data());
		glGenBuffers(mRibbonIBOs.size(), mRibbonIBOs.data());
		for (size_t i = 0; i < mControlPoints.size(); ++i)
		{
			Curve ribbonCurve = evalCatmullRomspline(mControlPoints[i], 80, false, 0, 0);
			Surface surf = makeGenCyl(starCurve, ribbonCurve);
			mRibbonNumTriangles[i] = surf.VF.size();

			std::vector<MissileMeshVertex> vertexData(surf.VV.size());

			for (size_t k = 0; k < vertexData.size(); ++k)
			{
				vertexData[k] = MissileMeshVertex(surf.VV[k], surf.VN[k], surf.VT[k].x, surf.VT[k].y);
			}
			glBindVertexArray(mRibbonVAOs[i]);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRibbonIBOs[i]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, surf.VF.size() * sizeof(Vec3i), surf.VF.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, mRibbonVBOs[i]);
			glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(MissileMeshVertex), vertexData.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), NULL);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(MissileMeshVertex), (char*)NULL + sizeof(Vec4f));

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

		}
	}

	void SpaceScene::debugRenderRibbons()
	{
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

		static Vec4f deltaValue = m_knobs[5];
		if (deltaValue != m_knobs[5] && mSelectedCurveIndex != -1 && mSelectedControlPointIndex != -1) {
			deltaValue = m_knobs[5];
			mControlPoints[mSelectedCurveIndex][mSelectedControlPointIndex]
				= mSelectedPointOrigValue + m_knobs[5].getXYZ();
		}

		for (size_t k = 0; k < mControlPoints.size(); ++k) {
			glColor3f(0,0.2,0);
			Curve curve = evalCatmullRomspline(mControlPoints[k], 60, false, 0, 0);
			for (size_t i = 1; i < curve.size(); ++i) {
				CurvePoint cp1 = curve[i - 1];
				CurvePoint cp2 = curve[i];
				glVertex3f(cp1.V.x, cp1.V.y, cp1.V.z);
				glVertex3f(cp2.V.x, cp2.V.y, cp2.V.z);
			}
		}


		glEnd();

		glColor3f(1, 0, 0);
		glPointSize(9.0f);
		glBegin(GL_POINTS);

		for (size_t k = 0; k < mControlPoints.size(); ++k) {
			for (size_t i = 0; i < mControlPoints[k].size(); ++i) {
				Vec3f cp = mControlPoints[k][i];
				glVertex3f(cp.x, cp.y, cp.z);
			}
		}

		glEnd();
		glPopMatrix();
	}

	void SpaceScene::selectControlPoint(Vec2i mousePos) {

		if (mSelectedControlPointIndex != -1) {
			mControlPoints[mSelectedCurveIndex][mSelectedControlPointIndex] =
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



		float x = float(mousePos.x) / mWndSize.x *  2.0f - 1.0f;
		float y = float(mousePos.y) / mWndSize.y * -2.0f + 1.0f;

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
		for (size_t k = 0; k < mControlPoints.size(); ++k) {
			for (size_t i = 0; i < mControlPoints[k].size(); ++i) {
				if (intersect_sphere(Ro, Rd, mControlPoints[k][i], 10.0f, t)) {
					mSelectedCurveIndex = k;
					mSelectedControlPointIndex = i;
					mSelectedPointOrigValue = mControlPoints[k][i];
				}
			}
		}
	}

	void SpaceScene::saveRibbonPaths()
	{

		for (size_t i = 0; i < mControlPoints.size(); ++i) {
			std::string filePath = "assets/space_ribbon_path_" + std::to_string(i + 1) + ".txt";
			saveRibbonPath(filePath, mControlPoints[i]);
		}

	}

	void SpaceScene::saveRibbonPath(const std::string & fileName, const std::vector<Vec3f> & vec) {

		std::ofstream out(fileName);

		out << vec.size() << std::endl;

		for (size_t i = 0; i < vec.size(); ++i) {
			out << vec[i].x << " " << vec[i].y << " " << vec[i].z << std::endl;
		}
	}

	void SpaceScene::renderRibbons(GLContext * gl, const Mat4f & toScreen, const Vec3f & cameraPosition)
	{
		mNefertitiFBO->bind();

		mMeshCurveRenderProgram->use();

		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("toScreen"), toScreen);
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightPos[0]"), getLightPosition(0));
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightColor[0]"), getLightColor(0));
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightDirection[0]"), getLightDirection(0));
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightPos[1]"), getLightPosition(1));
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightColor[1]"), getLightColor(1));
		gl->setUniform(mMeshCurveRenderProgram->getUniformLoc("lightDirection[1]"), getLightDirection(1));

		for (size_t i = 0; i < mRibbonVAOs.size(); ++i)
		{
			glBindVertexArray(mRibbonVAOs[i]);
			int s = int(FWSync::ribbonStart)*mProfileNumIndices * 6;
			int e = int(FWSync::ribbonEnd)*mProfileNumIndices * 6;
			glDrawElements(GL_TRIANGLES, e - s, GL_UNSIGNED_INT, NULL + (char*)(s * sizeof(GL_UNSIGNED_INT)));
		}
		glBindVertexArray(0);

		mNefertitiFBO->unbind();
	}

	void SpaceScene::godrayPass(GLContext * gl, const Mat4f & nefertitiToScreen, const Mat4f & cloudToScreen, const Mat4f & sdfToScreen, const Mat4f & ribbonsToScreen)
	{
		mGodrayFBO->bind();

		glClearColor(0.001,0.001,0.001,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		/*mParticleGodrayProgram->use();

		gl->setUniform(mParticleGodrayProgram->getUniformLoc("toScreen"), nefertitiToScreen);
		gl->setUniform(mParticleGodrayProgram->getUniformLoc("color"), Vec3f(0.0f));

		glBindVertexArray(mNeferittiVAO);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, mNumNefertitiParticles);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glBindVertexArray(0);

		glBindVertexArray(mParticleCloudVAO);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArrays(GL_POINTS, 0, mNumNefertitiParticles);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
		glBindVertexArray(0);*/

		mGodrayProgram->use();
		gl->setUniform(mGodrayProgram->getUniformLoc("color"), Vec3f(0.0f));
		gl->setUniform(mGodrayProgram->getUniformLoc("toScreen"), ribbonsToScreen);
		for (size_t i = 0; i < mRibbonVAOs.size(); ++i)
		{
			glBindVertexArray(mRibbonVAOs[i]);
			int s = int(FWSync::ribbonStart)*mProfileNumIndices * 6;
			int e = int(FWSync::ribbonEnd)*mProfileNumIndices * 6;
			glDrawElements(GL_TRIANGLES, e - s, GL_UNSIGNED_INT, NULL + (char*)(s * sizeof(GL_UNSIGNED_INT)));
		}
		glBindVertexArray(0);

		gl->setUniform(mGodrayProgram->getUniformLoc("toScreen"), sdfToScreen);
		gl->setUniform(mGodrayProgram->getUniformLoc("color"), Vec3f(1.0f));
		glBindBuffer(GL_ARRAY_BUFFER, mMesher->getTriangleVBO());

		glEnableVertexAttribArray(0); // position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), 0);

		glEnableVertexAttribArray(1); // normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DMT_TriangleUnit), (GLvoid*)((char*)NULL + sizeof(float) * 8));
		int numTrigs = mMesher->getNumTriangles();
		if (0<numTrigs)glDrawArrays(GL_TRIANGLES, 0, numTrigs);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		mGodrayFBO->unbind();

		mGodrayBlurFBO->bind();

		mGodrayBlurProgram->use();

		Vec4f lightClipSpace = sdfToScreen * Vec4f(0,0,0, 1.0f);
		lightClipSpace /= lightClipSpace.w;
		lightClipSpace.x = lightClipSpace.x*0.5 + 0.5;
		lightClipSpace.y = lightClipSpace.y*0.5 + 0.5;

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mGodrayFBO->getTexture(0));

		gl->setUniform(mGodrayBlurProgram->getUniformLoc("bwSmapler"), 0);
		gl->setUniform(mGodrayBlurProgram->getUniformLoc("lightPos"), lightClipSpace.getXY());
		gl->setUniform(mGodrayBlurProgram->getUniformLoc("WEIGHT"), FWSync::godrayWeight);

		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(mQuadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
		glEnable(GL_DEPTH_TEST);

		mGodrayBlurFBO->unbind();



		mGaussiaFilter->process(gl, mGodrayBlurFBO->getTexture(0), mGodrayBlurTex, 2);
	}

};