#pragma once

#include "gpu/GLContext.hpp"
#include <string>

namespace FW {

	enum DMT_Buffer_Types : int {
		INDEX_BUFFER=0, 
		BLOCK_BUFFER,
		MESH_BUFFER,
		BUFFERS_COUNT
	};

	struct DMT_TriangleUnit {
		Vec4f position;
		Vec4f direction;
		Vec4f normal;
		Vec4f orig_position;
	};

	class DynamicMarchingTetrahedra {

	public:

		DynamicMarchingTetrahedra(GLContext * gl, const Vec4f & cubeInfo, const std::string & sceneShader);

		void update(GLContext * gl);

		GLuint getTriangleVBO() const {
			return m_buffers[DMT_Buffer_Types::MESH_BUFFER];
		}

		void loadShaders(GLContext * gl);

		void renderMesh(GLContext * gl, GLContext::Program * prog);
		void renderMeshCheap(GLContext * gl, GLContext::Program * prog);

		void setDispValues(
			const Vec4f & disp1,
			const Vec4f & disp2,
			const Vec4f & disp3,
			const Vec4f & disp4) {
			m_disp1 = disp1;
			m_disp2 = disp2;
			m_disp3 = disp3;
			m_disp4 = disp4;
		}
		
		int getNumTriangles() const {
			return m_numTriangles;
		}

	private:
		GLuint m_buffers[DMT_Buffer_Types::BUFFERS_COUNT];
		Vec4f m_cubeInfo;

		std::string m_tetraCSProgram;
		std::string m_sceneShader;
		std::string m_renderProgram;

		Vec4f m_disp1;
		Vec4f m_disp2;
		Vec4f m_disp3;
		Vec4f m_disp4;

		int m_numTriangles;
	};
	
};