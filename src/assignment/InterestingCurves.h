#pragma once

#include "curve.h"
#include <functional>

std::function<DualParamCoord(float t)> getHeartFunction();
std::function<DualParamCoord(float t)> getVivianiFunction(float sphereRadius);