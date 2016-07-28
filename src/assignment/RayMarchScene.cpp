#include "RayMarchScene.h"
#include "gui/Image.hpp"
#include <fstream>
#include <algorithm>

namespace FW {

	RayMarchScene::RayMarchScene(const std::string & sceneShader, GLContext * ctx) :
		
		m_sceneShader(sceneShader),
		m_programName(sceneShader + "_raymarch"),
		m_tmProgramName(sceneShader + "_tetrahedra_cs"),
		m_tmNormalProgramName(sceneShader + "_tetrahedra_normal_cs"),
		m_showVolumeSliders(false)
	
	{

		// init knobs

		m_knobs[0] = Vec4f(1.0);
		m_knobs[1] = Vec4f(0.5f);
		m_knobs[2] = Vec4f(0.0f, -1.0f, 0.0f, 0.0f);
		m_knobs[3] = Vec4f(0.5f);
		m_knobs[4] = Vec4f(0.2f);

		m_volumeStart = Vec4f(-10.2, -6.849, -6.62, 0.087);
		m_volumeNumBlocks = Vec4f(191,241,145, 0);

		glGenBuffers(1, &m_csVertexBufferSSBO);
		glGenBuffers(1, &m_csPrefixSumSSBO);

		loadShaders(ctx);
		loadTextures();
	}


	void RayMarchScene::render(Window & wnd, const CameraControls & camera) {

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

		// render procedural textures
		for (unsigned int i = 0; i < 10; ++i) {
			if (m_proceduralTextures[i] != nullptr) {
				m_proceduralTextures[i]->renderToTexture(wnd, camera);
			}
		}

		GLContext::Program * prog = ctx->getProgram(m_programName.c_str());

		updateUniforms(wnd, camera);

		prog->use();

		ctx->setAttrib(prog->getAttribLoc("posAttrib"), 4, GL_FLOAT, 0, posAttrib);
		ctx->setAttrib(prog->getAttribLoc("texAttrib"), 2, GL_FLOAT, 0, texAttrib);



		

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	}

	void RayMarchScene::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

			case Action::Action_ReloadShaders:
				loadShaders(wnd.getGL());
				break;
			case Action::Action_RM_MarchRetrahedraMesh:
				saveMesh(toModel(wnd.getGL()), "out.obj");
				break;
			default:
				break;
		}
		
	}

	void RayMarchScene::saveMesh(const RayMarchScene_TriangulatedModel & mesh, const std::string & fileName) {

		std::ofstream ofs(fileName);

		// output vertices
		size_t numVertices = mesh.m_vertices.size();
		size_t numFaces = numVertices / 3;
		// sort faces according types

		std::vector<RayMarchScene_Face> faces(numFaces);
		for (size_t i = 0, cFace = 0; i < numVertices; i += 3, ++cFace) {
			faces[cFace] = RayMarchScene_Face(mesh.m_vertices[i], mesh.m_vertices[i+1], mesh.m_vertices[i+2]);
		}
		std::sort(faces.begin(), faces.end(), [](const RayMarchScene_Face & f1, const RayMarchScene_Face & f2) {
			return f1.getType() < f2.getType();
		});


		ofs << "mtllib buzludja.mtl" << std::endl;

		// save vertex positions
		
		for (size_t i = 0; i < numFaces; ++i) {

			for (size_t j = 0; j < 3; ++j) {
				ofs << "v "
					<< faces[i].m_vertices[j].m_position.x
					<< " "
					<< faces[i].m_vertices[j].m_position.y
					<< " " << faces[i].m_vertices[j].m_position.z
					<< std::endl;
			}

		}


		// save normals
		for (size_t i = 0; i < numFaces; ++i) {

			for (size_t j = 0; j < 3; ++j) {
				ofs << "vn "
					<< faces[i].m_vertices[j].m_normal.x
					<< " "
					<< faces[i].m_vertices[j].m_normal.y
					<< " " << faces[i].m_vertices[j].m_normal.z
					<< std::endl;
			}

		}

		// save texture coordinates
		
		for (size_t i = 0; i < numFaces; ++i) {

			for (size_t j = 0; j < 3; ++j) {
				ofs << "vt "
					<< faces[i].m_vertices[j].m_data.x
					<< " "
					<< faces[i].m_vertices[j].m_data.y
					<< std::endl;
			}

		}

		// output faces
		int cFace = -1;

		for (size_t i = 0, f = 1; i < numFaces; ++i) {

			if (cFace != faces[i].getType()) {
				// new material found
				cFace = faces[i].getType();

				// print material type
				ofs << "usemtl buzludja" << cFace << std::endl;
			}

			

			ofs << "f " << f << "/" << f << "/" << f << " ";
			++f;
			ofs << f << "/" << f << "/" << f << " ";
			++f;
			ofs << f << "/" << f << "/" << f << std::endl;
			++f;

		}

		ofs.close();

	}

	void RayMarchScene::loadShaders(GLContext * ctx) {

		const char vertexShader[] = "shaders/raymarch/quad.vertex";

		const char declShader[] = "shaders/raymarch/decl.glsl"; // header stuff, declerations
		const char hgSdfShader[] = "shaders/raymarch/hg_sdf.glsl"; // mercury's sdf library
		const char raymarchShader[] = "shaders/raymarch/raymarch.glsl"; // raymarch utilities
		const char noiseShader[] = "shaders/raymarch/noise.glsl"; // noise utilities
		const char pixelShader[] = "shaders/raymarch/test.pixel"; // pixel shader
		const char knobsShader[] = "shaders/raymarch/knobs.glsl"; // knob uniforms
		loadShader(ctx, { vertexShader }, 
		{ 
			declShader,
			hgSdfShader,
			knobsShader,
			noiseShader,
			m_sceneShader,
			raymarchShader,
			pixelShader
		}, m_programName);

		
		loadShader(ctx,
		{
			declShader, 
			hgSdfShader, 
			knobsShader,
			raymarchShader,
			noiseShader,
			m_sceneShader,
			"shaders/compute/tetrahedra_march.glsl"
		}, m_tmProgramName);

		loadShader(ctx,
		{
			declShader,
			hgSdfShader,
			knobsShader,
			raymarchShader,
			noiseShader,
			m_sceneShader,
			"shaders/compute/tetrahedra_march_normals.glsl"
		}, m_tmNormalProgramName);
		
	}

	void RayMarchScene::updateUniforms(Window & wnd, const CameraControls & camera) {

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

		// update volume

		// actually adjust the values

		Vec4f volumeSizeCopy = Vec4f(m_volumeNumBlocks) * m_volumeStart.w;
	
		ctx->setUniform(prog->getUniformLoc("volumeStart"), m_volumeStart);
		volumeSizeCopy.w = (m_showVolumeSliders ? 1.0f : 0.0f);
		ctx->setUniform(prog->getUniformLoc("volumeSize"), volumeSizeCopy);


		// update textures
		ctx->setUniform(prog->getUniformLoc("hullSampler"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_brickTexture);

		ctx->setUniform(prog->getUniformLoc("brokenGlass"), 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_concreteTexture);

		int textureOffset = 2;

		for (unsigned int i = 0; i < 10; ++i) {
			if (m_proceduralTextures[i] != nullptr) {

				String texUniformName = sprintf("procTex[%u]", i);
				ctx->setUniform(prog->getUniformLoc(texUniformName), textureOffset);
				glActiveTexture(GL_TEXTURE0 + textureOffset);
				glBindTexture(GL_TEXTURE_2D, m_proceduralTextures[i]->getTextureHandle());
				++textureOffset;
			}
		}
		ctx->checkErrors();
	}

	void RayMarchScene::activate(Window & wnd, CommonControls & controls) {

		updateGUI(wnd, controls);
		
	}

	void RayMarchScene::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}

		// remove other sliders
		controls.removeControl(&m_volumeStart.x);
		controls.removeControl(&m_volumeStart.y);
		controls.removeControl(&m_volumeStart.z);
		controls.removeControl(&m_volumeStart.w);

		controls.removeControl(&m_volumeNumBlocks.x);
		controls.removeControl(&m_volumeNumBlocks.y);
		controls.removeControl(&m_volumeNumBlocks.z);

		// remove toggles
		controls.removeControl(&m_showVolumeSliders);
		controls.removeControl(&actionExt);
	}

	void RayMarchScene::updateGUI(Window & wnd, CommonControls & controls) {

		cleanUpGUI(wnd, controls);

		controls.addSeparator();

		controls.addToggle(&m_showVolumeSliders, FW_KEY_NONE, "Show volume sliders", &updateGUIExt);
		controls.addButton((S32*)&actionExt, (S32)Action::Action_RM_MarchRetrahedraMesh, FW_KEY_NONE, "March ");
		

		static const std::pair<Vec4f, Vec4f> KNOB_SLIDE_DATA[10] = {
			std::make_pair(Vec4f(-40.0f), Vec4f(40.0f)), // knob1
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


		String xStr = sprintf("Knob%u.x = %%f", activeKnob+1);
		String yStr = sprintf("Knob%u.y = %%f", activeKnob + 1);
		String zStr = sprintf("Knob%u.z = %%f", activeKnob + 1);
		String wStr = sprintf("Knob%u.w = %%f", activeKnob + 1);

		controls.beginSliderStack();
		controls.addSlider(&m_knobs[activeKnob].x, KNOB_SLIDE_DATA[activeKnob].first.x, KNOB_SLIDE_DATA[activeKnob].second.x, false, FW_KEY_NONE, FW_KEY_NONE, xStr);
		controls.addSlider(&m_knobs[activeKnob].y, KNOB_SLIDE_DATA[activeKnob].first.y, KNOB_SLIDE_DATA[activeKnob].second.y, false, FW_KEY_NONE, FW_KEY_NONE, yStr);
		controls.addSlider(&m_knobs[activeKnob].z, KNOB_SLIDE_DATA[activeKnob].first.z, KNOB_SLIDE_DATA[activeKnob].second.z, false, FW_KEY_NONE, FW_KEY_NONE, zStr);
		controls.addSlider(&m_knobs[activeKnob].w, KNOB_SLIDE_DATA[activeKnob].first.w, KNOB_SLIDE_DATA[activeKnob].second.w, false, FW_KEY_NONE, FW_KEY_NONE, wStr);
		controls.endSliderStack();

		if (m_showVolumeSliders) {
			controls.beginSliderStack();
			controls.addSlider(&m_volumeStart.x, -30.0, 30.0, false, FW_KEY_NONE, FW_KEY_NONE, "Vol.x = %f");
			controls.addSlider(&m_volumeStart.y, -30.0, 30.0, false, FW_KEY_NONE, FW_KEY_NONE, "Vol.y = %f");
			controls.addSlider(&m_volumeStart.z, -30.0, 30.0, false, FW_KEY_NONE, FW_KEY_NONE, "Vol.z = %f");
			controls.addSlider(&m_volumeStart.w, 0.0, 1.0, false, FW_KEY_NONE, FW_KEY_NONE, "Cube length = %f");
			controls.endSliderStack();

			controls.beginSliderStack();
			controls.addSlider(&m_volumeNumBlocks.x, 0, 500, false, FW_KEY_NONE, FW_KEY_NONE, "Vol size.x = %d");
			controls.addSlider(&m_volumeNumBlocks.y, 0, 500, false, FW_KEY_NONE, FW_KEY_NONE, "Vol size.y = %d");
			controls.addSlider(&m_volumeNumBlocks.z, 0, 500, false, FW_KEY_NONE, FW_KEY_NONE, "Vol size.z = %d");
			controls.endSliderStack();
		}
	}

	RayMarchScene_TriangulatedModel RayMarchScene::toModel(GLContext * ctx) {
		
		GLContext::Program * tetrahedraMarchProgram = ctx->getProgram(m_tmProgramName.c_str());
		GLContext::Program * tetrahedraMarchNormalProgram = ctx->getProgram(m_tmNormalProgramName.c_str());

		RayMarchScene_TriangulatedModel mesh;

		// find longest axis and split along it.

		Vec4f cubeInfo = m_volumeStart;

		Vec3i totalGroupSize = m_volumeNumBlocks.getXYZ();

		for (unsigned int k = 0; k < totalGroupSize.y; k += 4) {
			tetrahedraMarchProgram->use();

			Vec3i globalGroupSize = Vec3i(totalGroupSize.x, 4, totalGroupSize.z);

			ctx->setUniform(tetrahedraMarchProgram->getUniformLoc("cubeInfo"), cubeInfo);

			Vec3i numBlocks =
				Vec3i(
					::ceil(globalGroupSize.x / 4.0f),
					::ceil(globalGroupSize.y / 4.0f),
					::ceil(globalGroupSize.z / 4.0f)
					);

			const unsigned int totalNumCubes = numBlocks.x * numBlocks.y * numBlocks.z * 64;
			
			// run prefix pass
			ctx->setUniform(tetrahedraMarchProgram->getUniformLoc("isPrefixSumPass"), true);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_csPrefixSumSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, totalNumCubes * sizeof(unsigned int), NULL, GL_DYNAMIC_COPY);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_csPrefixSumSSBO);
			glDispatchCompute(numBlocks.x, numBlocks.y, numBlocks.z);

			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_csPrefixSumSSBO);

			unsigned int * dt = (unsigned int*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, totalNumCubes * sizeof(unsigned int), GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);


			unsigned int totalVertices = 0;
			for (unsigned int i = 0; i < totalNumCubes; ++i) {
				unsigned int dtCopy = dt[i];
				dt[i] = totalVertices;
				totalVertices += dtCopy;
			}
			
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

			
			if (totalVertices > 0) {

				ctx->setUniform(tetrahedraMarchProgram->getUniformLoc("isPrefixSumPass"), false);

				// allocates enough data for the current run
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_csVertexBufferSSBO);
				glBufferData(GL_SHADER_STORAGE_BUFFER, totalVertices * sizeof(RayMarchScene_Vertex), NULL, GL_DYNAMIC_COPY);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

				// bind ssbo's and start computing

				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_csVertexBufferSSBO);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_csPrefixSumSSBO);

				glDispatchCompute(numBlocks.x, numBlocks.y, numBlocks.z);

				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

				tetrahedraMarchNormalProgram->use();

				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_csVertexBufferSSBO);
				ctx->setUniform(tetrahedraMarchNormalProgram->getUniformLoc("numVertices"), totalVertices);

				unsigned requiredNumBlocks = ::ceil(totalVertices / 64.0f);

				unsigned cbBlocks = ::ceil(cbrtf(totalVertices / 64.0f));

				glDispatchCompute(cbBlocks, cbBlocks, cbBlocks);

				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

				// done computing

				// bind ssbo's and retrieve data

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_csVertexBufferSSBO);

				RayMarchScene_Vertex * data = (RayMarchScene_Vertex*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, totalVertices * sizeof(RayMarchScene_Vertex), GL_MAP_READ_BIT);

				
				for (unsigned int i = 0; i < totalVertices; ++i) {


					// actually exists
					mesh.m_vertices.push_back(data[i]);


				}

				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
			cubeInfo.y += (4.0f * cubeInfo.w);
			
		}
		
		std::cout << "done " << mesh.m_vertices.size() << std::endl;
		
		return mesh;
	}

	void RayMarchScene::loadTextures() {

		m_brickTexture = m_concreteTexture = 0;

		for (int i = 0; i < 10; ++i) {
			m_proceduralTextures[i] = nullptr;
		}

	}

};