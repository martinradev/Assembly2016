#pragma once

#include "base/Math.hpp"
#include "gpu/GLContext.hpp"
#include "3d/Mesh.hpp"

#include <string>

namespace FW {

	class AlphabetMesh {

	public:

		AlphabetMesh(const std::string & meshDir);
		Mesh<VertexPNTC> * stringify(const std::string & text);

	private:

		MeshBase * m_meshes[256];

	};

};