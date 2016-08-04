
#include "App.hpp"

#include "RayMarchScene.h"
#include "HeightMapScene.h"
#include "BrightPassFilter.h"
#include "BloomFilter.h"
#include "ToneMapper.h"
#include "base/Main.hpp"
#include "gpu/GLContext.hpp"
#include "3d/Mesh.hpp"
#include "3d/Texture.hpp"
#include "io/File.hpp"
#include "io/StateDump.hpp"
#include "base/Random.hpp"
#include "ShaderSetup.h"
#include "TessellationTestScene.h"
#include "ForwardShading.h"
#include "LightParticles.h"
#include "TexturePool.h"
#include "GlobalSyncVars.h"
#include "GPUPrefixScan.h"
#include "Util.h"
#include <stdio.h>
#include "PrefixScanTests.h"
#include "NBodyTest.h"
#include "ParticleModelNBody.h"
#include "TunnelScene.h"
#include "FinalScene.h"
#include "SyncVars.h"
#include "gui/Image.hpp"
#include <iostream>
#include <conio.h>
#include <fstream>
#include <string>
#include <chrono>
#include <omp.h>
using namespace FW;

// extern to be used from all scenes
unsigned activeKnob;
bool updateGUIExt;
FW::Action actionExt = FW::Action::Action_None; // scene action

namespace FW {
	FW::Timer GLOBAL_TIMER;
	float GLOBAL_DT;
	float GLOBAL_RATIO;
}

namespace FW {
	TexturePool * TEXTURE_POOL;
};

//------------------------------------------------------------------------

App::App(void)
: m_commonCtrl(CommonControls::Feature_Default & ~CommonControls::Feature_RepaintOnF5),
m_cameraCtrl(&m_commonCtrl, CameraControls::Feature_Default | CameraControls::Feature_StereoControls),
m_activeKnob(Knob::Knob1),
m_prevKnob(Knob::Knob4),
m_action(Action::Action_None),
m_scene(nullptr),
m_displaySceneTabs(false),
mExposure(2.5f),
mLuminance(0.0f),
mOffset(0.0f),
mThreshold(0.0f)
{
	
    m_commonCtrl.showFPS(true);
    m_commonCtrl.addStateObject(this);
    m_cameraCtrl.setKeepAligned(true);
    m_window.addListener(&m_cameraCtrl);
	m_cameraCtrl.setSpeed(100.0f);
	m_commonCtrl.addSeparator();
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_ReloadShaders,			FW_KEY_ENTER,   "Reload shaders (ENTER)");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_AddCameraPoint, FW_KEY_SPACE, "Add cam point (SPACE)");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_DeletePrevPoint, FW_KEY_DELETE, "Rem cam point (Del)");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_SavePath, FW_KEY_NONE, "Save cam points");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_RenderPath, FW_KEY_NONE, "Render path");
	m_commonCtrl.addButton((S32*)&m_action, (S32)Action::Action_FollowPath, FW_KEY_NONE, "Follow path");
	m_commonCtrl.addToggle(&m_displaySceneTabs, FW_KEY_Q, "Show scene tabs (Q)");
	m_commonCtrl.addSeparator();
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob1, FW_KEY_1, "Knob1");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob2, FW_KEY_2, "Knob2");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob3, FW_KEY_3, "Knob3");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob4, FW_KEY_4, "Knob4");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob5, FW_KEY_5, "Knob5");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob6, FW_KEY_6, "Knob6");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob7, FW_KEY_7, "Knob7");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob8, FW_KEY_8, "Knob8");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob9, FW_KEY_9, "Knob9");
	m_commonCtrl.addToggle((S32*)&m_activeKnob, (S32)Knob::Knob10, FW_KEY_0, "Knob10");
	activeKnob = 0;
	
	m_commonCtrl.beginSliderStack();
	m_commonCtrl.addSlider(&mExposure, 0.1, 16.0, false, FW_KEY_NONE, FW_KEY_NONE, "Exp = %f");
	m_commonCtrl.endSliderStack();

	m_commonCtrl.beginSliderStack();
	m_commonCtrl.addSlider(&mLuminance, 0.0, 1.0, false, FW_KEY_NONE, FW_KEY_NONE, "Lum = %f");
	m_commonCtrl.addSlider(&mThreshold, 0.0, 1.0, false, FW_KEY_NONE, FW_KEY_NONE, "thre = %f");
	m_commonCtrl.addSlider(&mOffset, 0.0, 10.0, false, FW_KEY_NONE, FW_KEY_NONE, "off = %f");
	m_commonCtrl.endSliderStack();
	m_window.setTitle("Effects galore by Super Grand");
    m_window.addListener(this);
	//m_window.setSize(Vec2i(1280, 720));
	m_window.setSize(Vec2i(1280/2, 720/2));
	m_window.setFullScreen(false);
    m_window.addListener(&m_commonCtrl);

	m_commonCtrl.setStateFilePrefix( "Effects galore by Super Grand" );
	
	//m_commonCtrl.showControls(false);
	//m_commonCtrl.showFPS(false);
	//ShowCursor(false);
	
	GLContext * gl = m_window.getGL(); // grab the appropriate gl context to be able to setup()
	
	m_cameraCtrl.setNear(0.5f);
	m_cameraCtrl.setFar(14950.0f);
	m_cameraCtrl.setFOV(70.0f);

	TEXTURE_POOL = new TexturePool();
	Vec2i size = m_window.getSize();
	GLuint fboDepthTex = TEXTURE_POOL->request(TextureDescriptor(GL_DEPTH_COMPONENT32F, size.x, size.y, GL_DEPTH_COMPONENT, GL_FLOAT))->m_texture;
	mLastColorTexture = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, size.x, size.y, GL_RGBA, GL_FLOAT))->m_texture;
	mLastFBO.reset(new FBO(fboDepthTex));
	mLastFBO->attachTexture(0, mLastColorTexture);
	
	mToneMapper.reset(new ToneMapper(gl, size));
	mToneMapper->setTexture(mLastColorTexture);

	mBloomTexture = TEXTURE_POOL->request(TextureDescriptor(GL_RGBA32F, 0.5*size.x, 0.5*size.y, GL_RGBA, GL_FLOAT))->m_texture;
	mBloomFilter.reset(new NewBloomFilter(gl, mBloomTexture, 0.5*size.x, 0.5*size.y));

	mToneMapper->setBloomTexture(mBloomTexture);

	setupMusic();

	setupRocket();

	setupScenes();

	BASS_Start();
	BASS_ChannelPlay(m_stream, false);

	GLOBAL_TIMER = FW::Timer();
	GLOBAL_TIMER.start();
	GLOBAL_DT = 0.0f;
	GLOBAL_RATIO = float(size.y) / float(size.x);
	m_cameraCtrl.mAspectRatio = GLOBAL_RATIO;
	
}

//------------------------------------------------------------------------

App::~App(void)
{
	
}

//------------------------------------------------------------------------

bool App::handleEvent(const Window::Event& ev)
{
	if (ev.type == Window::EventType_Close)
	{

		
		m_window.showModalMessage("Exiting...");
		delete this;
		return true;
	}

	if (ev.type == Window::EventType_KeyUp) {

		if (ev.key == FW_KEY_MOUSE_LEFT && m_displaySceneTabs) {

			// get mouse pixel position and see what we clicked

			Vec2i pos = ev.mousePos;

			Vec2i wndSize = m_window.getSize();

			const unsigned int sceneBoxSide = wndSize.x / m_allScenes.size();
			const unsigned int height = 80;

			if (pos.y > wndSize.y - height) {

				// definitely clicked a scene

				if (m_scene != nullptr) {
					m_scene->cleanUpGUI(m_window, m_commonCtrl);
				}

				m_scene = m_allScenes[pos.x / sceneBoxSide].m_scene;
				m_scene->activate(m_window, m_commonCtrl);
			}

		}

	}

    Action action = actionExt;
	actionExt = Action::Action_None;

	if (action != Action::Action_None) {
		// handle action
		m_scene->handleAction(action, m_window, m_commonCtrl);
		
	}

	

	action = m_action;
	m_action = Action::Action_None;
	if (action != Action::Action_None) {
		// handle action
		m_scene->handleAction(action, m_window, m_commonCtrl);
		mToneMapper->handleAction(action, m_window, m_commonCtrl);
	}

	if (ev.type == Window::EventType_KeyUp) {

		if (ev.key == FW_KEY_MOUSE_RIGHT) {
			m_scene->handleAction(Action::Action_RightButton, m_window, m_commonCtrl);
		}
	}

	/*
	if (action == Action::Action_AddCameraPoint) {
		m_cameraPath->addControState(m_cameraCtrl.getCameraToWorld());
	} else if (action == Action::Action_DeletePrevPoint) {
		m_cameraPath->popState();
	} else if (action == Action::Action_SavePath) {
		m_cameraPath->savePath("assets/cam_path.txt");
	}
	else if (action == Action::Action_FollowPath) {
		m_followPath = !m_followPath;
	}
	else if (action == Action::Action_RenderPath) {
		m_renderPath = !m_renderPath;
	}*/
	
	if (m_activeKnob != m_prevKnob) {
		activeKnob = (unsigned)m_activeKnob;
		m_prevKnob = m_activeKnob;
		if (m_scene != nullptr) updateGUIExt = true;
	}

	if (updateGUIExt) m_scene->updateGUI(m_window, m_commonCtrl);
	updateGUIExt = false;
    m_window.setVisible(true);

	if (ev.type == Window::EventType_Paint) {
		renderFrame(m_window.getGL());
	}
    m_window.repaint();
    return false;
}

//-----------------------------------------.-------------------------------

//------------------------------------------------------------------------

void App::waitKey(void)
{
    printf("Press any key to continue . . . ");
    _getch();
    printf("\n\n");
}

void App::renderFrame(GLContext* gl)
{
	static float prevT = GLOBAL_TIMER.getElapsed();
	float dt = GLOBAL_TIMER.getElapsed();
	GLOBAL_DT = dt - prevT;
	prevT = dt;
	FWSync::updateValues(m_rocket, m_stream);
	if (FWSync::sceneIndex < 1.0f)
	{
		m_scene = mLogoScene;
	}
	else if (FWSync::sceneIndex < 2.0f) {
		m_scene = mWaterScene;
	}
	else if (FWSync::sceneIndex < 3.0f) {
		m_scene = mSpaceScene;
	}
	else if (FWSync::sceneIndex < 4.0f) {
		m_scene = mTunnelScene;
	}
	else if (FWSync::sceneIndex < 5.0f) {
		m_scene = mFinalScene;
	}
	else if (FWSync::sceneIndex > 5.0f) {
		exit(0);
	}

	m_cameraCtrl.setFOV(FWSync::fov);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.3, 0.4, 1.0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	if (m_scene) {
		m_scene->render(m_window, m_cameraCtrl);

		mBloomFilter->bloom(gl, mLastColorTexture, 4, FWSync::bloom_lumi, FWSync::bloom_thre, FWSync::bloom_offset);

		mToneMapper->setExposure(FWSync::exposure);
		mToneMapper->render(m_window, m_cameraCtrl);
	}
	
	if (m_displaySceneTabs) {
		renderSceneOptions(gl);
	}
	
	// debug cam path
	/*if (m_renderPath) {
		glUseProgram(0);
		glMatrixMode(GL_PROJECTION);
		Mat4f worldToCamera = m_cameraCtrl.getWorldToCamera();
		Mat4f projection = gl->xformFitToView(Vec2f(-1.0f, -1.0f), Vec2f(2.0f, 2.0f)) * m_cameraCtrl.getCameraToClip();

		glLoadMatrixf(&projection(0, 0));
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(&worldToCamera(0, 0));

		Curve curve = m_cameraPath->getDebugCurve();
		glBegin(GL_LINES);
		for (int i = 0; i < curve.size(); ++i) {
			CurvePoint cp = curve[i];
			glVertex3f(cp.V.x, cp.V.y, cp.V.z);
		}
		glEnd();
	}*/
	gl->checkErrors();
}

void App::renderSceneOptions(GLContext * gl) {

	static const Vec3f BOX_COLORS[4] = {

		Vec3f(0.952f, 0.968f, 0.149f),
		Vec3f(0.772f, 0.117f, 0.007f),
		Vec3f(0.207f, 0.101f, 0.858f),
		Vec3f(0.101f, 0.858f, 0.352f)
	};
	

	Vec2i wndSize = m_window.getSize();
	
	const float sceneBoxSide = float(wndSize.x) / m_allScenes.size();
	
	glDisable(GL_DEPTH_TEST);
	glUseProgram(0);
	glBegin(GL_QUADS);
	float yPos = 1.0f - 2.0f * (float(wndSize.y) - 80.0f) / wndSize.y;
	for (size_t i = 0; i < m_allScenes.size(); ++i) {
		float xPos = 2.0f * float(i) * sceneBoxSide / wndSize.x - 1.0f;
		float xNextPos = 2.0f * float(i+1) * sceneBoxSide / wndSize.x - 1.0f;
		Vec3f color = BOX_COLORS[i % 4];
		glColor3f(color.x, color.y, color.z);
		glVertex2f(xPos, -1.0f);
		glVertex2f(xNextPos, -1.0f);
		glVertex2f(xNextPos, yPos);
		glVertex2f(xPos, yPos);
		
	}
	glEnd();
	
	for (size_t i = 0; i < m_allScenes.size(); ++i) {
		float xPos = 2.0f * float(i) * sceneBoxSide / wndSize.x - 1.0f;
		gl->drawLabel(m_allScenes[i].m_name.c_str(), Vec2f(xPos + 0.1f, yPos-0.05f), 0xFFFFFFFF);
	}

	gl->checkErrors();
}

//------------------------------------------------------------------------

void FW::init(void)
{
    new App;
}

//------------------------------------------------------------------------


void App::readState(StateDump& d)
{

	d.pushOwner("App");
	
	d.popOwner();

	
}

//------------------------------------------------------------------------

void App::writeState(StateDump& d) const
{
	d.pushOwner("App");
	
	d.popOwner();
}

void App::setupScenes() {

	GLContext * gl = m_window.getGL();
	Vec2i sz = m_window.getSize();

	mLogoScene = new ParticleLogoSDF(gl, 110, mLastFBO.get(), sz.x, sz.y, m_cameraCtrl);
	m_allScenes.push_back(SceneDescriptor(mLogoScene, "Particle logo"));

	mWaterScene = new TessellationTestScene(&m_cameraCtrl,"tess_test", gl, sz.x, sz.y, mLastFBO.get());
	m_allScenes.push_back(SceneDescriptor(mWaterScene, "Tess"));
	
	mSpaceScene = new SpaceScene(gl, sz.x, sz.y, mLastFBO.get(), &m_cameraCtrl);
	m_allScenes.push_back(SceneDescriptor(mSpaceScene, "Space"));

	mTunnelScene = new TunnelScene(gl, sz.x, sz.y, mLastFBO.get(), &m_cameraCtrl);
	m_allScenes.push_back(SceneDescriptor(mSpaceScene, "Tunnel"));

	mFinalScene = new FinalScene(gl, sz.x, sz.y, mLastFBO.get(), &m_cameraCtrl, (TunnelScene*)mTunnelScene, (TessellationTestScene*)mWaterScene, (ParticleLogoSDF*)mLogoScene);
	
	m_scene = mLogoScene;
}


static void die(const char * message) {
	::printf("%s\n", message);
	exit(1);
}

void App::setupMusic() {
	HWND hwnd = m_window.getHandle();
	if (!BASS_Init(-1, 44100, 0, hwnd, 0)) {
		die("coult not init bass");
	}
	m_stream = BASS_StreamCreateFile(false, "assets/trance_12.mp3", 0, 0, 0);

	if (!m_stream) {
		die("coult not load mp3");
	}

}

void App::setupRocket() {

	m_rocket = sync_create_device("rocket/sync");

	if (!m_rocket) {
		die("out of memory");
	}

	FWSync::bindTracks(m_rocket, m_stream);

}