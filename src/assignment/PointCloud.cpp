#include "Object.h"

#include <vector>

using namespace FW;

PointCloud::~PointCloud() {
	glDeleteVertexArrays(1, &vao_);

	glDeleteBuffers(1, &vbo_);

}

void PointCloud::setupMarchingCubes(const FW::Vec3f & f, const FW::Vec3f & t, float cubeLen) {
	
	// generate actual grid
	Vec3f from = FW::min(f, t);
	Vec3f to = FW::max(f, t);
	Vec3f delta = (to - from) / cubeLen;
	Vec3i steps = Vec3i((FW::floor(delta.x) + 1.0f), (FW::floor(delta.y) + 1.0f), (FW::floor(delta.z) + 1.0f));

	numberOfElements_ = steps.x * steps.y * steps.z;

	// generate point cloud
	std::vector<Vec3f> points(numberOfElements_);

#pragma omp parallel for 
	for (int i = 0; i < steps.x; ++i) {
		for (int j = 0; j < steps.y; ++j) {
			for (int k = 0; k < steps.z; ++k) {

				const int index = (i * steps.y + j) * steps.z + k;
				points[index] = from + cubeLen * Vec3f(i,j,k);

			}
		}
	}

	// setup opengl data
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f)* points.size(), points.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void PointCloud::render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const {

	program->use();

	FW::Mat4f toWorldTrans = posToCamera * toWorld_;
	FW::Mat4f normalToWorldTrans = toWorldTrans.inverted().transposed();

	gl->setUniform(program->getUniformLoc("toWorldUniform"), toWorldTrans);
	gl->setUniform(program->getUniformLoc("normalToWorldUniform"), normalToWorldTrans);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glEnableVertexAttribArray(0);

	glDrawArrays(GL_POINTS, 0, numberOfElements_);

	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}