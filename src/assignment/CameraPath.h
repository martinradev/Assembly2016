#pragma once
#include "base/Math.hpp"
#include "curve.h"
#include <vector>
#include <string>

namespace FW {

	class CameraPath {

	public:

		// load from file
		CameraPath(const std::string & file);
		CameraPath();

		void addControState(const Mat4f & cameraState);
		void popState();
		void savePath(const std::string & file) const;
		Mat4f getTransformation(float t) const;
		Curve getDebugCurve() const;
	private:
		void loadPath(const std::string & file);
		std::vector<Vec3f> m_controlPositions;
		std::vector<Vec4f> m_controlOrientations;
	};

}