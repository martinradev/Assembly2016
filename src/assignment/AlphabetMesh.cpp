#include "AlphabetMesh.h"
#include <ctype.h>
namespace FW {

	AlphabetMesh::AlphabetMesh(const std::string & meshDir) {
		Mat3f rot = Mat3f::rotation(Vec3f(1, 0, 0), FW_PI / 2.0f);
		Mat4f rot4f;
		rot4f.setRow(0, Vec4f(rot.getRow(0), 0));
		rot4f.setRow(1, Vec4f(rot.getRow(1), 0));
		rot4f.setRow(2, Vec4f(rot.getRow(2), 0));
		rot4f.setRow(3, Vec4f(0, 0 ,0 , 1));
		Mat4f trans = Mat4f::translate(Vec3f(0, 2.7, 0));

		Mat4f total = trans * rot4f;
		for (char i = 'a'; i <= 'z'; ++i) {
			std::string file = meshDir + "/" + std::string(1, i) + ".obj";
			m_meshes[i] = importMesh(file.c_str());
			m_meshes[i]->xform(total);
		}
		std::string file = meshDir + "/less.obj";
		m_meshes['<'] = importMesh(file.c_str());
		m_meshes['<']->xform(total);

		file = meshDir + "/3.obj";
		m_meshes['3'] = importMesh(file.c_str());
		m_meshes['3']->xform(total);
		file = meshDir + "/!.obj";
		m_meshes['!'] = importMesh(file.c_str());
		m_meshes['!']->xform(total);


	}

	Mesh<VertexPNTC> * AlphabetMesh::stringify(const std::string & text) {

		Mesh<VertexPNTC> * res = new Mesh<VertexPNTC>();

		Vec3f off = Vec3f(2.5, 0, 0);
		
		int i = 0;
		int col = 0;
		for (char c : text) {
			if (c == '\n') {
				++col;
				i = -1;
			} else if (c != ' ') {
				MeshBase * cMesh = m_meshes[tolower(c)];
				MeshBase copy = *cMesh;
				Mat4f tt;
				if (tolower(c) == 'i') {
					tt = Mat4f::translate(off*i - Vec3f(0.0f, col * 3.7, 0)) * Mat4f::scale(Vec3f(4.5));
				}
				else {
					tt = Mat4f::translate(off*i - Vec3f(0.8f, col * 3.7, 0)) * Mat4f::scale(Vec3f(4.5));
				}
				copy.xform(tt);
				res->append(copy);
			}
			++i;
		}
		
		return res;
	}

};