#include "HeightMapScene.h"
#include <fstream>

namespace FW {

	HeightMapScene::HeightMapScene(const std::string & sceneShader, const std::string & heightMapFile, GLContext * ctx)
		:
		m_sceneShader(sceneShader),
		m_programName(sceneShader),
		m_programCSName(sceneShader + "_triangulate_cs"),
		m_heightMapTex(0),
		m_planeSize(Vec2f(500, 500)),
		m_numTriangles(Vec2i(400,400))
	{
		
		loadShaders(ctx);

		Image * img = importImage(heightMapFile.c_str());

		m_heightMapTex = img->createGLTexture();

		delete img;


	}

	void HeightMapScene::loadShaders(GLContext * ctx) {
		const char vertexShader[] = "shaders/raymarch/quad.vertex";
		const char declShader[] = "shaders/raymarch/decl.glsl"; // header stuff, declerations
		const char hgSdfShader[] = "shaders/raymarch/hg_sdf.glsl"; // mercury's sdf library
		const char noiseShader[] = "shaders/raymarch/noise.glsl"; // raymarch utilities
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms
		const char raymarchShader[] = "shaders/raymarch/raymarch.glsl"; // raymarch stuff
		const char terrainMarchPixelShader[] = "shaders/raymarch/terrain_march_pixel.glsl"; // pixel shader
		const char heightmapTriangulateCsShader[] = "shaders/compute/heightmap_triangulate.glsl"; // terrain triangulate cs shader

		loadShader(ctx, { vertexShader },
		{
			declShader,
			noiseShader,
			hgSdfShader,
			knobsShader,
			raymarchShader,
			m_sceneShader,
			terrainMarchPixelShader
		}, m_programName);

		loadShader(ctx,
		{
			declShader,
			noiseShader,
			hgSdfShader,
			knobsShader,
			raymarchShader,
			m_sceneShader,
			heightmapTriangulateCsShader
		}, m_programCSName);

		glGenBuffers(1, &m_terrainTrigSSBO);
	}

	void HeightMapScene::render(Window & wnd, const CameraControls & camera) {
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

		GLContext::Program * prog = ctx->getProgram(m_programName.c_str());

		updateUniforms(wnd, camera);

		prog->use();

		ctx->setAttrib(prog->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(prog->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttrib);





		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	void HeightMapScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		case Action::Action_TriangulateHeightMap:
			saveModel(toModel(wnd.getGL()), "terrain.obj");
			break;
		default:
			break;
		}

	}

	void HeightMapScene::updateUniforms(Window & wnd, const CameraControls & camera) {

		Vec2i wndSize = wnd.getSize();
		Vec2f fWndSize = Vec2f(wndSize.x, wndSize.y);

		GLContext * ctx = wnd.getGL();

		// update ray march program uniforms
		GLContext::Program * prog = ctx->getProgram(m_programName.c_str());

		prog->use();

		ctx->setUniform(prog->getUniformLoc("windowSize"), fWndSize);
		ctx->setUniform(prog->getUniformLoc("center"), camera.getPosition());
		Vec3f fw = camera.getForward();
		Vec3f up = camera.getUp();
		Vec3f hor = cross(fw, up).normalized();
		up = cross(hor, fw).normalized();

		ctx->setUniform(prog->getUniformLoc("horizontal"), hor);
		ctx->setUniform(prog->getUniformLoc("vertical"), up);
		ctx->setUniform(prog->getUniformLoc("direction"), fw);

		// update knobs
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

		// update textures
		ctx->setUniform(prog->getUniformLoc("terrainSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_heightMapTex);

		ctx->checkErrors();


		// update uniforms for 
		prog = ctx->getProgram(m_programCSName.c_str());

		prog->use();

		ctx->setUniform(prog->getUniformLoc("terrainSampler"), 0);

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

		const unsigned int numThreads = m_numTriangles.x * m_numTriangles.y;
		const Vec2f step = m_planeSize / Vec2f(m_numTriangles.x, m_numTriangles.y);
		const Vec2f planeStart = Vec2f(1.0f);

		ctx->setUniform(prog->getUniformLoc("numThreads"), numThreads);
		ctx->setUniform(prog->getUniformLoc("planeStart"), planeStart);
		ctx->setUniform(prog->getUniformLoc("planeStep"), step);

		ctx->checkErrors();
	}

	void HeightMapScene::activate(Window & wnd, CommonControls & controls) {

		updateGUI(wnd, controls);

	}

	void HeightMapScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		controls.removeControl(&actionExt);
	}

	void HeightMapScene::updateGUI(Window & wnd, CommonControls & controls) {

		cleanUpGUI(wnd, controls);

		controls.addSeparator();

		controls.addButton((S32*)&actionExt, (S32)Action::Action_TriangulateHeightMap, FW_KEY_NONE, "Triangulate ");


		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(0.0f), Vec4f(100.0f)), // knob1
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

	void HeightMapScene::saveModel(const HeightMapModel & model, const std::string & fileName) {
		
		std::ofstream out(fileName.c_str());

		// positions
		for (size_t i = 0; i < model.m_vertices.size(); ++i) {
			out << "v "
				<< model.m_vertices[i].m_position.x << " "
				<< model.m_vertices[i].m_position.y << " "
				<< model.m_vertices[i].m_position.z
				<< std::endl;
		}

		// normals
		for (size_t i = 0; i < model.m_vertices.size(); ++i) {
			out << "vn "
				<< model.m_vertices[i].m_normal.x << " "
				<< model.m_vertices[i].m_normal.y << " "
				<< model.m_vertices[i].m_normal.z
				<< std::endl;
		}

		// faces
		for (size_t i = 1; i <= model.m_vertices.size(); i += 3) {
			out << "f " << i << "//" << i << " " 
				<< (i + 1) << "//" << (i+1) << " " 
				<< (i + 2) << "//" << (i + 2) << std::endl;
		}

		out.close();
	}

	HeightMapModel HeightMapScene::toModel(GLContext * ctx) {

		HeightMapModel model;

		GLContext::Program * triangulateProgram = ctx->getProgram(m_programCSName.c_str());

		triangulateProgram->use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_heightMapTex);

		const unsigned int numThreads = m_numTriangles.x * m_numTriangles.y;
		const unsigned int numVertices = numThreads * 6;

		// initialize ssbo size
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_terrainTrigSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numVertices * sizeof(HeightMapVertex), NULL, GL_DYNAMIC_COPY);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_terrainTrigSSBO);

		const Vec2i numBlocks = Vec2i(
			::ceil(m_numTriangles.x / 8.0f),
			::ceil(m_numTriangles.y / 8.0f)
			);

		glDispatchCompute(numBlocks.x, numBlocks.y, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_terrainTrigSSBO);

		HeightMapVertex * data = (HeightMapVertex*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, numVertices * sizeof(HeightMapVertex), GL_MAP_READ_BIT);


		for (unsigned int i = 0; i < numVertices; ++i) {


			// actually exists
			model.m_vertices.push_back(data[i]);


		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		return model;
	}

};