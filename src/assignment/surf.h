#pragma once

#include "curve.h"

#include <iostream>

// Surface is just a struct that contains vertices, normals, and
// faces.  VV[i] is the position of vertex i, and VN[i] is the normal
// of vertex i.  A face is a triple i,j,k corresponding to a triangle
// with (vertex i, normal i), (vertex j, normal j), ...
// VT texture coordinates
struct Surface
{
    std::vector<FW::Vec3f> VV;
    std::vector<FW::Vec3f> VN;
	std::vector<FW::Vec3f> VC;
	std::vector<FW::Vec2f> VT;
	std::vector<FW::Vec3i> VF;
	
	void append(const Surface & surface);
	void applyTransformation(const FW::Mat4f & transformation);

};

// This draws the surface.  Draws the surfaces with smooth shading or wireframe
void drawSurface(const Surface& surface, bool wireframe, bool color);

// This draws normals to the surface at each vertex of length len.
void drawNormals(const Surface& surface, float len);

// Sweep a profile curve that lies flat on the xy-plane around the
// y-axis.  The number of divisions is given by steps.
Surface makeSurfRev(const Curve& profile, unsigned steps);

Surface makeGenCyl(const Curve& profile, const Curve& sweep);

void outputObjFile(std::ostream& out, const Surface& surface);

