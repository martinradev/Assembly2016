#include "Util.h"

std::pair<GLuint, GLuint> get2DPlane() {
	static float TriangleVertices[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, -1.0f,
		1.0f, 1.0f,
		-1.0f, 1.0f
	};

	static GLuint vao = 0, vbo = 0;
	if (vao == 0) {

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float)* 12, TriangleVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	return std::make_pair(vao, vbo);
}

void render2DPlane(GLuint vao, GLuint vbo) {
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

Surface genSurfaceFromVerticesOnly(const std::vector<FW::Vec3f> & vertices) {
	using namespace FW;
	Surface surface;

	// copy vertices
	surface.VV = vertices;

	// faces
	surface.VF.resize(vertices.size() / 3);

	// normals
	surface.VN.resize(vertices.size());

	for (size_t i = 0; i < vertices.size(); i += 3) {

		surface.VF[i / 3] = Vec3i(i, i + 1, i + 2);

		Vec3f n = getTriangleNormal(surface.VV[i], surface.VV[i + 1], surface.VV[i + 2]);

		surface.VN[i] = surface.VN[i + 1] = surface.VN[i + 2] = n;

	}

	surface.VT.resize(vertices.size());

	return surface;

}

namespace FW {

	void decomposeMesh(Mesh<VertexPNTC> * mesh, std::vector<RTTriangle> & triangles, std::vector <MeshBase::Material*> & materials) {
		triangles.clear();
		triangles.reserve(mesh->numTriangles());
		materials.resize(mesh->numSubmeshes());
		for (int i = 0; i < mesh->numSubmeshes(); ++i)
		{
			const Array<Vec3i>& idx = mesh->indices(i);
			for (int j = 0; j < idx.getSize(); ++j)
			{

				const VertexPNTC &v0 = mesh->vertex(idx[j][0]),
					&v1 = mesh->vertex(idx[j][1]),
					&v2 = mesh->vertex(idx[j][2]);

				RTTriangle t = RTTriangle(v0, v1, v2);

				t.m_data.vertex_indices = idx[j];
				t.m_material = &(mesh->material(i));
				triangles.push_back(t);

			}
			materials[i] = &(mesh->material(i));
		}
	}
	
};