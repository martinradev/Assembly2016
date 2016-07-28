#pragma once

#include "base/Defs.hpp"



namespace FW {


	enum class Action : S32 {

		Action_None = 0,
		Action_RayMarchScene,
		Action_RM_MarchRetrahedraMesh, // march the ray marching scene to generate the mesh
		Action_ReloadShaders,
		Action_DisplaySceneTabs,
		Action_SaveTexture,
		Action_TriangulateHeightMap, // triangulates the height map
		Action_AddCameraPoint,
		Action_FollowPath,
		Action_DeletePrevPoint,
		Action_SavePath,
		Action_RenderPath,

		Action_AddRibbonPoint,
		Action_DeleteRibbonPoint,
		Action_DebugRenderRibbonPath,
		Action_SaveRibbonPath,
		Action_LeftButton,
		Action_RightButton
	};

	

	enum class Knob : S32 {


		Knob1 = 0,
		Knob2,
		Knob3,
		Knob4,
		Knob5,
		Knob6,
		Knob7,
		Knob8,
		Knob9,
		Knob10

	};

};

extern unsigned activeKnob;
extern bool updateGUIExt;
extern FW::Action actionExt;