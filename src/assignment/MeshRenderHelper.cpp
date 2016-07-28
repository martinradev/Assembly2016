#include "MeshRenderHelper.h"

namespace FW {

	void renderMesh(GLContext * gl, MeshBase * mesh, GLContext::Program * program) {

		
		gl->setUniform(program->getUniformLoc("diffuseSampler"), 0);
		gl->setUniform(program->getUniformLoc("normalSampler"), 1);
		gl->setUniform(program->getUniformLoc("specularSampler"), 2);

		int posAttrib = mesh->findAttrib(MeshBase::AttribType_Position);
		int normalAttrib = mesh->findAttrib(MeshBase::AttribType_Normal);
		int vcolorAttrib = mesh->findAttrib(MeshBase::AttribType_Color);
		int texCoordAttrib = mesh->findAttrib(MeshBase::AttribType_TexCoord);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->getVBO().getGLBuffer());
		mesh->setGLAttrib(gl, posAttrib, program->getAttribLoc("positionAttrib"));

		if (normalAttrib != -1) {
			mesh->setGLAttrib(gl, normalAttrib, program->getAttribLoc("normalAttrib"));
		}
		else if (program->getAttribLoc("normalAttrib") != -1) {
			glVertexAttrib3f(program->getAttribLoc("normalAttrib"), 0.0f, 0.0f, 0.0f);
		}

		if (vcolorAttrib != -1) {
			mesh->setGLAttrib(gl, vcolorAttrib, program->getAttribLoc("vcolorAttrib"));
		} else if (program->getAttribLoc("vcolorAttrib") != -1) {
			glVertexAttrib4f(program->getAttribLoc("vcolorAttrib"), 1.0f, 1.0f, 1.0f, 1.0f);
		}

		if (texCoordAttrib != -1) {
			mesh->setGLAttrib(gl, texCoordAttrib, program->getAttribLoc("texCoordAttrib"));
		}
		else if (program->getAttribLoc("texCoordAttrib") != -1) {
			glVertexAttrib2f(program->getAttribLoc("texCoordAttrib"), 0.0f, 0.0f);
		}
		
		for (int i = 0; i < mesh->numSubmeshes(); i++)
		{
			const MeshBase::Material& mat = mesh->material(i);
			gl->setUniform(program->getUniformLoc("diffuseUniform"), mat.diffuse);
			gl->setUniform(program->getUniformLoc("specularUniform"), mat.specular * 0.5f);
			gl->setUniform(program->getUniformLoc("glossiness"), mat.glossiness);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mat.textures[MeshBase::TextureType_Diffuse].getGLTexture());
			gl->setUniform(program->getUniformLoc("useDiffuseTexture"), mat.textures[MeshBase::TextureType_Diffuse].exists());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mat.textures[MeshBase::TextureType_Normal].getGLTexture());
			gl->setUniform(program->getUniformLoc("useNormalMap"), mat.textures[MeshBase::TextureType_Normal].exists());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, mat.textures[MeshBase::TextureType_Specular].getGLTexture());
			gl->setUniform(program->getUniformLoc("useSpecularMap"), mat.textures[MeshBase::TextureType_Specular].exists());

			glDrawElements(GL_TRIANGLES, mesh->vboIndexSize(i), GL_UNSIGNED_INT, (void*)(UPTR)mesh->vboIndexOffset(i));


		}
	}

	void renderMeshCheap(GLContext * gl, MeshBase * mesh, GLContext::Program * program) {


		int posAttrib = mesh->findAttrib(MeshBase::AttribType_Position);
		int normalAttrib = mesh->findAttrib(MeshBase::AttribType_Normal);
		int vcolorAttrib = mesh->findAttrib(MeshBase::AttribType_Color);
		int texCoordAttrib = mesh->findAttrib(MeshBase::AttribType_TexCoord);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->getVBO().getGLBuffer());
		mesh->setGLAttrib(gl, posAttrib, program->getAttribLoc("positionAttrib"));

		if (normalAttrib != -1) {
			mesh->setGLAttrib(gl, normalAttrib, program->getAttribLoc("normalAttrib"));
		}
		else if (program->getAttribLoc("normalAttrib") != -1) {
			glVertexAttrib3f(program->getAttribLoc("normalAttrib"), 0.0f, 0.0f, 0.0f);
		}

		if (vcolorAttrib != -1) {
			mesh->setGLAttrib(gl, vcolorAttrib, program->getAttribLoc("vcolorAttrib"));
		}
		else if (program->getAttribLoc("vcolorAttrib") != -1) {
			glVertexAttrib4f(program->getAttribLoc("vcolorAttrib"), 1.0f, 1.0f, 1.0f, 1.0f);
		}

		if (texCoordAttrib != -1) {
			mesh->setGLAttrib(gl, texCoordAttrib, program->getAttribLoc("texCoordAttrib"));
		}
		else if (program->getAttribLoc("texCoordAttrib") != -1) {
			glVertexAttrib2f(program->getAttribLoc("texCoordAttrib"), 0.0f, 0.0f);
		}

		for (int i = 0; i < mesh->numSubmeshes(); i++)
		{
			const MeshBase::Material& mat = mesh->material(i);

			glDrawElements(GL_TRIANGLES, mesh->vboIndexSize(i), GL_UNSIGNED_INT, (void*)(UPTR)mesh->vboIndexOffset(i));


		}
	}

	void renderMeshTessellation(GLContext * gl, MeshBase * mesh, GLContext::Program * program, int numPatches) {

		glPatchParameteri(GL_PATCH_VERTICES, numPatches);
		gl->setUniform(program->getUniformLoc("diffuseSampler"), 0);
		gl->setUniform(program->getUniformLoc("normalSampler"), 1);
		gl->setUniform(program->getUniformLoc("specularSampler"), 2);

		int posAttrib = mesh->findAttrib(MeshBase::AttribType_Position);
		int normalAttrib = mesh->findAttrib(MeshBase::AttribType_Normal);
		int vcolorAttrib = mesh->findAttrib(MeshBase::AttribType_Color);
		int texCoordAttrib = mesh->findAttrib(MeshBase::AttribType_TexCoord);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->getVBO().getGLBuffer());
		mesh->setGLAttrib(gl, posAttrib, program->getAttribLoc("positionAttrib"));

		if (normalAttrib != -1) {
			mesh->setGLAttrib(gl, normalAttrib, program->getAttribLoc("normalAttrib"));
		}
		else if (program->getAttribLoc("normalAttrib") != -1) {
			glVertexAttrib3f(program->getAttribLoc("normalAttrib"), 0.0f, 0.0f, 0.0f);
		}

		if (vcolorAttrib != -1) {
			mesh->setGLAttrib(gl, vcolorAttrib, program->getAttribLoc("vcolorAttrib"));
		}
		else if (program->getAttribLoc("vcolorAttrib") != -1) {
			glVertexAttrib4f(program->getAttribLoc("vcolorAttrib"), 1.0f, 1.0f, 1.0f, 1.0f);
		}

		if (texCoordAttrib != -1) {
			mesh->setGLAttrib(gl, texCoordAttrib, program->getAttribLoc("texCoordAttrib"));
		}
		else if (program->getAttribLoc("texCoordAttrib") != -1) {
			glVertexAttrib2f(program->getAttribLoc("texCoordAttrib"), 0.0f, 0.0f);
		}

		for (int i = 0; i < mesh->numSubmeshes(); i++)
		{
			const MeshBase::Material& mat = mesh->material(i);
			gl->setUniform(program->getUniformLoc("diffuseUniform"), mat.diffuse);
			gl->setUniform(program->getUniformLoc("specularUniform"), mat.specular * 0.5f);
			gl->setUniform(program->getUniformLoc("glossiness"), mat.glossiness);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mat.textures[MeshBase::TextureType_Diffuse].getGLTexture());
			gl->setUniform(program->getUniformLoc("useDiffuseTexture"), mat.textures[MeshBase::TextureType_Diffuse].exists());

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, mat.textures[MeshBase::TextureType_Normal].getGLTexture());
			gl->setUniform(program->getUniformLoc("useNormalMap"), mat.textures[MeshBase::TextureType_Normal].exists());

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, mat.textures[MeshBase::TextureType_Specular].getGLTexture());
			gl->setUniform(program->getUniformLoc("useSpecularMap"), mat.textures[MeshBase::TextureType_Specular].exists());

			glDrawElements(GL_PATCHES, mesh->vboIndexSize(i), GL_UNSIGNED_INT, (void*)(UPTR)mesh->vboIndexOffset(i));


		}

	}

};