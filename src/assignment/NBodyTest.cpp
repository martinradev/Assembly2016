#include "NBodyTest.h"

namespace FW {


	NBodyTest::NBodyTest(GLContext * ctx, unsigned width, unsigned height) :
		m_computeProgramName("nbodyComputeSimple"),
		m_renderProgramName("nbodyRenderSimple"),
		m_motionBlurProgramName("nbodyMotionBlur"),
		m_displayProgramName("nbodyDisplay"),
		m_imageX(120),
		m_imageY(120)
	{

		m_textures[NBODY_POS_TEXTURE] = 0;
		m_textures[NBODY_POS_TEXTURE1] = 0;

		m_gBuffer.reset(new GBuffer(width, height));
		m_gBufferAcc.reset(new GBuffer(width, height));
		// init knobs

		m_knobs[0] = Vec4f(1.0);
		m_knobs[1] = Vec4f(0.5f);
		m_knobs[2] = Vec4f(0.0f, -1.0f, 0.0f, 0.0f);
		m_knobs[3] = Vec4f(0.5f);
		m_knobs[4] = Vec4f(0.2f);

		loadShaders(ctx);
		createBuffers();

		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	}

	void NBodyTest::moveParticles(GLContext * gl) {

		GLContext::Program * prog = gl->getProgram(m_computeProgramName.c_str());

		prog->use();

		Vec2i totalSize = Vec2i(m_imageX, m_imageY);

		gl->setUniform(prog->getUniformLoc("imageSize"), totalSize);

		Vec2i localSize(20, 20);

		Vec2i numGroups = (totalSize + localSize - Vec2i(1)) / localSize;

		glBindImageTexture(0, m_textures[NBODY_POS_TEXTURE], 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(1, m_textures[NBODY_POS_TEXTURE1], 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

		glDispatchCompute(numGroups.x, numGroups.y, 1);

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		std::swap(m_textures[NBODY_POS_TEXTURE], m_textures[NBODY_POS_TEXTURE1]);

	}

	void NBodyTest::render(Window & wnd, const CameraControls & camera) {
		
		GLContext * gl = wnd.getGL();

		// move particles
		moveParticles(gl);

		GLContext::Program * prog;

		// render
		GLuint fbo = m_gBuffer->getFBO();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		prog = gl->getProgram(m_renderProgramName.c_str());
		prog->use();

		glBindFragDataLocation(prog->getHandle(), 0, "diffuseOut");
		glBindFragDataLocation(prog->getHandle(), 2, "positionOut");
		glBindFragDataLocation(prog->getHandle(), 3, "depthOut");

		gl->setUniform(prog->getUniformLoc("toClip"), camera.getWorldToClip());
		gl->setUniform(prog->getUniformLoc("knob2"), m_knobs[1]);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);

		/*glBindVertexArray(m_particleVAO);
		glDrawArrays(GL_POINTS, 0, m_imageX*m_imageY);
		glBindVertexArray(0);*/

		glBindVertexArray(m_cubeVAO);
		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0, m_imageX*m_imageY);
		glBindVertexArray(0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		// finished doing pass 1 to buffer
		
		// accumulate
		fbo = m_gBufferAcc->getFBO();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

		glClear(GL_DEPTH_BUFFER_BIT);

		prog = gl->getProgram(m_motionBlurProgramName.c_str());
		prog->use();

		glBindFragDataLocation(prog->getHandle(), 0, "color");

		gl->setUniform(prog->getUniformLoc("colorMap"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gBuffer->getTexture(GBUFFER_TYPES::GBUFFER_DIFFUSE));

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindVertexArray(m_quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDisable(GL_BLEND);
		// render all
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		prog = gl->getProgram(m_displayProgramName.c_str());
		prog->use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_gBufferAcc->getTexture(GBUFFER_TYPES::GBUFFER_DIFFUSE));
		gl->setUniform(prog->getUniformLoc("inTex"), 0);

		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		
	}

	void NBodyTest::handleAction(const Action & action, Window & wnd, CommonControls & controls) {
		Window::Event ev;

		switch (action) {

		case Action::Action_ReloadShaders:
			loadShaders(wnd.getGL());
			break;
		default:
			break;
		}

	}


	void NBodyTest::loadShaders(GLContext * ctx) {

		const char vertexShader[] = "shaders/nbody/nbody_vertex.glsl";
		const char fragmentShader[] = "shaders/nbody/nbody_fragment.glsl";
		const char computeShader[] = "shaders/nbody/nbody_compute_shared.glsl";

		loadShader(ctx, vertexShader, fragmentShader, m_renderProgramName);

		loadShader(ctx, computeShader, m_computeProgramName);

		const char motionVertexShader[] = "shaders/postprocess/motionBlurAcc_vertex.glsl";
		const char motionFragmentShader[] = "shaders/postprocess/motionBlurAcc_fragment.glsl";
		loadShader(ctx, motionVertexShader, motionFragmentShader, m_motionBlurProgramName);

		const char vertexDisplayShader[] = "shaders/common/display_vertex.glsl";
		const char fragmentDisplayShader[] = "shaders/common/display_fragment.glsl";
		loadShader(ctx, vertexDisplayShader, fragmentDisplayShader, m_displayProgramName);
	}

	void NBodyTest::activate(Window & wnd, CommonControls & controls) {

		updateGUI(wnd, controls);

	}

	void NBodyTest::cleanUpGUI(Window & wnd, CommonControls & controls) {
		// remove all knobs
		for (int i = 0; i < 10; ++i) {
			controls.removeControl(&m_knobs[i].x);
			controls.removeControl(&m_knobs[i].y);
			controls.removeControl(&m_knobs[i].z);
			controls.removeControl(&m_knobs[i].w);
		}


		controls.removeControl(&actionExt);
	}

	void NBodyTest::updateGUI(Window & wnd, CommonControls & controls) {

		cleanUpGUI(wnd, controls);


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

	void NBodyTest::createBuffers() {
		std::vector<Vec4f> particlePositions(m_imageX*m_imageY);
		Random rnd;
		int k = 0;
		for (int i = 0; i < m_imageX; ++i) {

			for (int j = 0; j < m_imageY; ++j) {

				Vec2f rndVec = rnd.getVec2f(0.0f, 1.0f);
				rndVec.x *= 2.0f * FW_PI;
				rndVec.y *= FW_PI;
				float radius = 6.0f;
				float x = radius * cosf(rndVec.x) * sinf(rndVec.y);
				float y = radius * sinf(rndVec.x) * sinf(rndVec.y);
				float z = radius * cosf(rndVec.y);

				float mass = 0.01f;
				if (i == 0 && j == 0) mass = 10000.0;

				particlePositions[k++] = Vec4f(x,y,z, mass);

			}

		}

		std::vector<Vec2i> m_particleIndices(m_imageX * m_imageY);
		k = 0;
		for (int i = 0; i < m_imageX; ++i) {
			for (int j = 0; j < m_imageY; ++j) {
				m_particleIndices[k++] = Vec2i(i,j);
			}
		}

		glGenVertexArrays(1, &m_particleVAO);
		glBindVertexArray(m_particleVAO);

		glGenBuffers(1, &m_particleVBO);

		glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2i) * m_imageX * m_imageY, m_particleIndices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glGenTextures(NBODY_TEX_COUNT, m_textures);

		glBindTexture(GL_TEXTURE_2D, m_textures[NBODY_POS_TEXTURE]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_imageX, m_imageY, 0, GL_RGBA, GL_FLOAT, particlePositions.data());


		glBindTexture(GL_TEXTURE_2D, m_textures[NBODY_POS_TEXTURE1]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_imageX, m_imageY, 0, GL_RGBA, GL_FLOAT, particlePositions.data());
		glBindTexture(GL_TEXTURE_2D, 0);

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

		static const float cubeVertices[] = {
			-1, -1, -1, 0,
			1, -1, -1, 0,
			1, 1, -1, 0,
			-1, 1, -1, 0,
			-1, -1, 1, 0,
			1, -1, 1, 0,
			1, 1, 1, 0,
			-1, 1, 1, 0
		};

		static const uint16_t cubeIndices[] = {
			// front
			0,1,2,
			0,2,3,

			// right
			1,5,6,
			1,6,2,

			// left
			4,0,3,
			4,3,7,

			// back
			5,4,7,
			5,7,6,

			// bottom
			0,1,5,
			0,5,4,

			// top
			3,2,6,
			3,6,7


		};

		glGenVertexArrays(1, &m_cubeVAO);
		glGenBuffers(1, &m_cubeVBO);
		glGenBuffers(1, &m_cubeIBO);
		glBindVertexArray(m_cubeVAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 32, cubeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 36, cubeIndices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

};