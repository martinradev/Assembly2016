#pragma once

#include "gpu/GLContext.hpp"
#include "DynamicMarchingTetrahedra.h"
#include <string>

namespace FW {

	class Mesher {

	public:

		Mesher(GLContext * gl, const Vec4f & cubeInfo, const std::string & sceneShader);

		void update(GLContext * gl);

		GLuint getTriangleVBO() const {
			return m_buffers[DMT_Buffer_Types::MESH_BUFFER];
		}

		void loadShaders(GLContext * gl);

		int getNumTriangles() const {
			return m_numTriangles;
		}

		GLContext::Program * getProgram() {
			return mTetraProgram;
		}

	private:
		GLuint m_buffers[DMT_Buffer_Types::BUFFERS_COUNT];
		Vec4f m_cubeInfo;

		GLContext::Program *mTetraProgram;

		int m_numTriangles;
		std::string m_tetraCSProgram;
		std::string m_sceneShader;
	};

};