#pragma once

#include "3d/Mesh.hpp"
#include "LightParticles.h"

namespace FW {

	void renderMesh(GLContext * ctx, MeshBase * mesh, GLContext::Program * program);
	void renderMeshCheap(GLContext * ctx, MeshBase * mesh, GLContext::Program * program);
	void renderMeshTessellation(GLContext * ctx, MeshBase * mesh, GLContext::Program * program, int numPatches);
	
}