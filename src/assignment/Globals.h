#pragma once

#include "TexturePool.h"
#include "FBOManager.h"

#include "base/Timer.hpp"

namespace FW {

	extern TexturePool * TEXTURE_POOL;
	extern FW::Timer GLOBAL_TIMER;
	extern float GLOBAL_DT;
	extern float GLOBAL_RATIO;
	//extern FBOPool * FBO_POOL;
}