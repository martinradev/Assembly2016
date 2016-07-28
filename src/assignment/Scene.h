#pragma once

#include "Action.h"
#include "ShaderSetup.h"
#include "gpu/GLContext.hpp"
#include "Object.h"
#include <string>

namespace FW {

	

	class Scene {

	public:

		// render the scene
		virtual void render(Window & wnd, const CameraControls & camera) = 0;

		// the scene handles its actions appropriately
		virtual void handleAction(const Action & action, Window & wnd, CommonControls & controls) = 0;

		// makes the scene active
		virtual void activate(Window & wnd, CommonControls & controls) = 0;

		// generates the gui for the scene
		virtual void updateGUI(Window & wnd, CommonControls & controls) = 0;

		// removes the elements for that scene
		virtual void cleanUpGUI(Window & wnd, CommonControls & controls) = 0;

		virtual ~Scene() {};

	private:

	};

	struct SceneDescriptor {

		SceneDescriptor() :
		m_scene(nullptr),
			m_name("NULL")
		{

		}
		SceneDescriptor(Scene * scene, const std::string & name) :
			m_scene(scene),
			m_name(name) {

		}

		Scene * m_scene;
		std::string m_name;

	};

};
