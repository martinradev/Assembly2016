#include "Object.h"
#include "Util.h"
#include "base/Random.hpp"

SurfaceObject::~SurfaceObject() {
	glDeleteVertexArrays(1, &vao_);

	glDeleteBuffers(1, &vbo_);

}

SurfaceObject::SurfaceObject(const float * meshBuf, const int * indBuf, int indBufSz, GLuint texObject)
:numberOfElements_(indBufSz),
textureObject_(texObject)
{
	

	toWorld_.setIdentity();

	std::vector<Vertex> mydata;
	mydata.reserve(indBufSz);

	FW::Random random;
	for (int i = 0; i < indBufSz; ++i) {
		const int k = indBuf[i];
		mydata.push_back(Vertex(FW::Vec3f::fromPtr(meshBuf + k * 8), FW::Vec3f::fromPtr(meshBuf + k * 8 + 3), FW::Vec2f::fromPtr(meshBuf + k * 8 + 6), 0.0f));
	}

	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);


	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glBufferData(GL_ARRAY_BUFFER, mydata.size() * sizeof(Vertex), mydata.data(), GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)(sizeof(FW::Vec3f)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)(sizeof(FW::Vec3f) * 2));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)(sizeof(FW::Vec3f) * 2 + sizeof(FW::Vec2f)));


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

SurfaceObject::SurfaceObject(const Surface & surface, GLuint texObject, const FW::Mat4f & toWorldTransformation) :
SurfaceObject(surface, texObject)
{
	toWorld_ = toWorldTransformation;
}

SurfaceObject::SurfaceObject(const Surface & surface, GLuint texObject) :
numberOfElements_(surface.VF.size() * 3),
textureObject_(texObject)
{

	toWorld_.setIdentity();

	std::vector<Vertex> mydata;
	mydata.reserve(surface.VF.size() * 3);

	FW::Random random;

	for (size_t i = 0; i < surface.VF.size(); ++i) {
		FW::Vec3i face = surface.VF[i];
		float myT = random.getF32(1.0f, 4.5f);
		for (size_t j = 0; j < 3; ++j) {
			mydata.push_back(Vertex(surface.VV[face[j]], surface.VN[face[j]], surface.VT[face[j]], myT));
		}
	}

	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);


	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glBufferData(GL_ARRAY_BUFFER, mydata.size() * sizeof(Vertex), mydata.data(), GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)(sizeof(FW::Vec3f)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)(sizeof(FW::Vec3f) * 2));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)(sizeof(FW::Vec3f) * 2 + sizeof(FW::Vec2f)));

	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}

void SurfaceObject::render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const {

	program->use();

	FW::Mat4f toWorldTrans = posToCamera * toWorld_;
	FW::Mat4f normalToWorldTrans = toWorldTrans.inverted().transposed();

	gl->setUniform(program->getUniformLoc("toWorldUniform"), toWorldTrans);
	gl->setUniform(program->getUniformLoc("normalToWorldUniform"), normalToWorldTrans);

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureObject_);

	glDrawArrays(GL_TRIANGLES, 0, numberOfElements_);

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}