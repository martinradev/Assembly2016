#include "Object.h"

using namespace FW;

SimpleObject::SimpleObject(FW::MeshBase * mesh) : mesh_(mesh) {

}

SimpleObject::~SimpleObject() {
	delete mesh_;
}

SimpleObject & SimpleObject::operator = (const SimpleObject & rht) {
	if (this != &rht) {
		
		delete mesh_;
		mesh_ = new FW::MeshBase(*rht.mesh_);

	}
	return *this;
}

SimpleObject::SimpleObject(const SimpleObject & rht) {
	
	mesh_ = new FW::MeshBase(*rht.mesh_);

}

SimpleObject::SimpleObject(SimpleObject && rht) {

	mesh_ = rht.mesh_;
	rht.mesh_ = nullptr;

}

SimpleObject & SimpleObject::operator = (SimpleObject && rht) {
	
	if (this != &rht) {

		mesh_ = rht.mesh_;
		rht.mesh_ = nullptr;

	}

	return *this;
}

void SimpleObject::render(GLContext* gl, FW::GLContext::Program * program, const Mat4f& posToCamera, const Mat4f& projection) const {
	
	program->use();

	int posAttrib = mesh_->findAttrib(MeshBase::AttribType_Position);
	int normalAttrib = mesh_->findAttrib(MeshBase::AttribType_Normal);
	int vcolorAttrib = mesh_->findAttrib(MeshBase::AttribType_Color);
	int texCoordAttrib = mesh_->findAttrib(MeshBase::AttribType_TexCoord);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_->getVBO().getGLBuffer());

	mesh_->setGLAttrib(gl, posAttrib, program->getAttribLoc("positionAttrib"));

	if (normalAttrib != -1)
		mesh_->setGLAttrib(gl, normalAttrib, program->getAttribLoc("normalAttrib"));
	else if (program->getAttribLoc("normalAttrib") != -1)
		glVertexAttrib3f(program->getAttribLoc("normalAttrib"), 0.0f, 0.0f, 0.0f);

	if (vcolorAttrib != -1)
		mesh_->setGLAttrib(gl, vcolorAttrib, program->getAttribLoc("vcolorAttrib"));
	else if (program->getAttribLoc("vcolorAttrib") != -1)
		glVertexAttrib4f(program->getAttribLoc("vcolorAttrib"), 1.0f, 1.0f, 1.0f, 1.0f);

	if (texCoordAttrib != -1)
		mesh_->setGLAttrib(gl, texCoordAttrib, program->getAttribLoc("texCoordAttrib"));
	else if (program->getAttribLoc("texCoordAttrib") != -1)
		glVertexAttrib2f(program->getAttribLoc("texCoordAttrib"), 0.0f, 0.0f);

	for (int i = 0; i < mesh_->numSubmeshes(); i++)
	{
		const MeshBase::Material& mat = mesh_->material(i);
		gl->setUniform(program->getUniformLoc("diffuseUniform"), mat.diffuse);
		gl->setUniform(program->getUniformLoc("specularUniform"), mat.specular * 0.5f);
		gl->setUniform(program->getUniformLoc("glossiness"), mat.glossiness);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mat.textures[MeshBase::TextureType_Diffuse].getGLTexture());
		gl->setUniform(program->getUniformLoc("useDiffuseTexture"), mat.textures[MeshBase::TextureType_Diffuse].exists());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mat.textures[MeshBase::TextureType_Normal].getGLTexture());
		gl->setUniform(program->getUniformLoc("useNormalMap"), mat.textures[MeshBase::TextureType_Normal].exists());

		glDrawElements(GL_TRIANGLES, mesh_->vboIndexSize(i), GL_UNSIGNED_INT, (void*)(UPTR)mesh_->vboIndexOffset(i));


	}

}

PlaneTestObject::PlaneTestObject() {
	static float TriangleVertices[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f
	};


	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 12, TriangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
}

PlaneTestObject::~PlaneTestObject() {

	glDeleteVertexArrays(1, &vao_);

	glDeleteBuffers(1, &vbo_);

}

void PlaneTestObject::render(FW::GLContext* gl, FW::GLContext::Program * program, const FW::Mat4f& posToCamera, const FW::Mat4f& projection) const {
	
	program->use();

	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}