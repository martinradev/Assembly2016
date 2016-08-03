#pragma once

#include "Scene.h"
#include "Action.h"

#include "gui/Window.hpp"
#include "gui/CommonControls.hpp"
#include "3d/CameraControls.hpp"
#include "3d/Texture.hpp"
#include "gpu/Buffer.hpp"
#include "gui/Image.hpp"
#include "DisplacedMesh.h"
#include "LightParticles.h"
#include "ForwardShading.h"
#include "bass.h"
#include "sync.h"
#include "AlphabetMesh.h"
#include "DynamicMarchingTetrahedra.h"
#include "CameraPath.h"
#include "ToneMapper.h"
#include "FBO.h"
#include "SpaceScene.h"
#include "NewBloomFilter.h"
#include <vector>
#include <memory>

namespace FW
{
//------------------------------------------------------------------------

class App : public Window::Listener, public CommonControls::StateObject
{

public:
                    App             (void);
    virtual         ~App            (void);

    virtual bool    handleEvent     (const Window::Event& ev);
	virtual void    readState(StateDump& d);
	virtual void    writeState(StateDump& d) const;

private:
    void            waitKey         (void);
    void            renderFrame     (GLContext* gl);
	void			renderSceneOptions(GLContext * gl);

private:
                    App             (const App&); // forbidden
    App&            operator=       (const App&); // forbidden

private:
    Window          m_window;
    CommonControls  m_commonCtrl;
    CameraControls  m_cameraCtrl;

    Action          m_action;
	Knob			m_activeKnob;
	Knob			m_prevKnob;
	
	std::unique_ptr<FBO> mLastFBO;
	std::unique_ptr<ToneMapper> mToneMapper;

	bool m_displaySceneTabs;
	Scene * m_scene;
	
	std::vector<SceneDescriptor> m_allScenes;
	void setupScenes();
	void setupMusic();
	void setupRocket();
	float mExposure;
	HSTREAM m_stream;
	sync_device * m_rocket;

	Scene * mLogoScene;
	Scene * mWaterScene;
	Scene * mSpaceScene;
	Scene * mTunnelScene;
	Scene * mFinalScene;
	
	std::unique_ptr<NewBloomFilter> mBloomFilter;
	GLuint mBloomTexture;

	GLuint mLastColorTexture;

	float mLuminance;
	float mOffset;
	float mThreshold;

	
};

}
