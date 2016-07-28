#include "Object.h"

CurveObject::CurveObject(const Curve & curve) : numberOfPoints_(curve.size() * 2 - 2) {

	std::vector<FW::Vec3f> linePoints;
	linePoints.reserve(curve.size() * 2 - 2);
	linePoints.push_back(curve[0].V);
	for (size_t i = 1; i < curve.size()-1; ++i) {
		linePoints.push_back(curve[i].V);
		linePoints.push_back(curve[i].V);
	}
	linePoints.push_back(curve.back().V);

	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glBufferData(GL_ARRAY_BUFFER, sizeof(FW::Vec3f)*linePoints.size(), linePoints.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
}

CurveObject::~CurveObject() {
	glDeleteVertexArrays(1, &vao_);
	glDeleteBuffers(1, &vbo_);
}

void CurveObject::render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const {
	program->use();
	glLineWidth(5.0f);
	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_LINES, 0, numberOfPoints_);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}