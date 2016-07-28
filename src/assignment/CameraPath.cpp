#include "CameraPath.h"
#include "Spline.h"
#include "Util.h"
#include <fstream>
namespace FW {

	CameraPath::CameraPath(const std::string & file) {
		loadPath(file);
	}

	CameraPath::CameraPath() {

	}

	void CameraPath::addControState(const Mat4f & state) {

		Vec4f quat = matToQuaternion(state);
		Vec4f trans = state.getCol(3);
		m_controlPositions.push_back(trans.getXYZ());
		m_controlOrientations.push_back(quat);
	}

	Mat4f CameraPath::getTransformation(float t)const {
		int idx = int(t);
		t -= float(idx);
		idx *= 3;
		if (m_controlPositions.size() < 4) {
			exit(1);
		}
		if (idx + 3 >= m_controlPositions.size()) {
			idx = m_controlPositions.size() - 4;
			t = 1.0f;
		}

		Vec3f position =  
			BezierCurve::evalBezier(
				m_controlPositions[idx],
				m_controlPositions[idx+1],
				m_controlPositions[idx+2],
				m_controlPositions[idx+3], t);

		int qOff = 0;
		if (t <= 0.25f) {
			qOff = 0;
		}
		else if (t <= 0.5f) {
			qOff = 1;
		}
		else if (t <= 0.75f) {
			qOff = 2;
		}
		else {
			qOff = 3;
		}
		
		int i0, i1;
		if (idx + qOff >= m_controlOrientations.size()) {
			i0 = m_controlOrientations.size() - 1;
		}
		else {
			i0 = idx + qOff;
		}

		if (idx + qOff + 1 >= m_controlOrientations.size()) {
			i1 = m_controlOrientations.size() - 1;
		}
		else {
			i1 = idx + qOff + 1;
		}
		i0 = idx;
		i1 = idx + 3;
		const auto & q0 = m_controlOrientations[i0];
		const auto & q1 = m_controlOrientations[i1];
		

		float t2 = t * 4.0f - int(t*4.0f);
		Vec4f qt = slerp(q0, q1, t);

		Mat4f trans = quaternionToMatrix(qt);
		trans.setCol(3, Vec4f(position, 1.0f));

		return trans;
	}

	Curve CameraPath::getDebugCurve() const {
		return evalBezier(m_controlPositions, 100.0f, false, 0.0f, 0.0f);
	}

	void CameraPath::popState() {
		size_t sz = m_controlPositions.size();

		printf("%d\n", int(sz));
		if (sz) {
			m_controlPositions.pop_back();
			m_controlOrientations.pop_back();
		}
	}

	void CameraPath::loadPath(const std::string & file) {
		std::ifstream in(file);

		size_t sz;
		in >> sz;

		m_controlPositions.resize(sz);
		m_controlOrientations.resize(sz);
		for (size_t i = 0; i < sz; ++i) {
			in >> m_controlPositions[i].x >> m_controlPositions[i].y >> m_controlPositions[i].z;
			in >> m_controlOrientations[i].x >> m_controlOrientations[i].y >> m_controlOrientations[i].z >> m_controlOrientations[i].w;
		}

		in.close();
	}

	void CameraPath::savePath(const std::string & filePath) const {
		std::ofstream out(filePath);
		out << m_controlPositions.size() << std::endl;
		for (size_t i = 0; i < m_controlPositions.size(); ++i) {
			out << m_controlPositions[i].x << " " << m_controlPositions[i].y << " " << m_controlPositions[i].z << std::endl;
			out << m_controlOrientations[i].x << " " << m_controlOrientations[i].y << " " << m_controlOrientations[i].z << " " << m_controlOrientations[i].w << std::endl;
		}
		out.close();
	}

};