#pragma once

#include "base/Math.hpp"
#include "gpu/GLContext.hpp"
#include "3d/Mesh.hpp"

#include "gui/Window.hpp"
#include "3d/CameraControls.hpp"
#include "GBuffer.h"
#include <string>

namespace FW {

	class DisplacedMesh {

	public:
		DisplacedMesh(
			GLContext * gl,
			Mesh<VertexPNTC> * mesh, 
			const std::string & vertexShader,
			const std::string & tessControlShader,
			const std::string & tessEvalShader,
			const std::string & fragmentShader,
			const std::string & programName);
		void render(GBuffer * gbuffer, Window & wnd, const CameraControls & camera, const Mat4f & toWorld);
		void loadShaders(GLContext * gl);

		void setDispValue(float v1, float v2, float v3) {
			m_disp1 = v1;
			m_disp2 = v2;
			m_disp3 = v3;
		}

	private:

		std::string m_program;
		std::string m_vertexShader;
		std::string m_tessControlShader;
		std::string m_tessEvalShader;
		std::string m_fragmentShader;

		Mesh<VertexPNTC> * m_mesh;

		float m_disp1;
		float m_disp2;
		float m_disp3;

	};

};