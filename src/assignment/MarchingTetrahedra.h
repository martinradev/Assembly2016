#pragma once

#include "Object.h"
#include "Util.h"
#include "base/Math.hpp"


#include <vector>
#include <functional>
#include <array>
#include <memory>


namespace FW {

	struct Tetrahedra {

		Tetrahedra(){};
		Tetrahedra(const FW::Vec3f & a, const FW::Vec3f & b, const FW::Vec3f & c, const FW::Vec3f & d) {
			vertices_[0] = a;
			vertices_[1] = b;
			vertices_[2] = c;
			vertices_[3] = d;
		}

		FW::Vec3f vertices_[4];

		std::array<Vec3f, 12> getTriangleFaces() const {
			std::array<Vec3f, 12> triangles;

			int k = 3;

			for (int i = 0; i < 3; ++i) {
				const int i1 = i;
				const int i2 = (i1 + 1) % 3;
				const int i3 = (i2 + 1) % 3;

				triangles[i] = vertices_[i];

				triangles[k++] = Vec3f(vertices_[i1]);
				triangles[k++] = Vec3f(vertices_[i2]);
				triangles[k++] = Vec3f(vertices_[3]);

				triangles[k++] = Vec3f(vertices_[i2]);
				triangles[k++] = Vec3f(vertices_[i3]);
				triangles[k++] = Vec3f(vertices_[3]);
			}

			return std::move(triangles);
		}

		Surface toSurface() const {
			Surface surface;

			// copy vertices
			surface.VV.resize(4);
			for (int i = 0; i < 4; ++i) surface.VV[i] = vertices_[i];

			// generate indices
			surface.VF.resize(4);
			surface.VF[0] = Vec3i(0,1,2);
			surface.VF[1] = Vec3i(0, 1, 3);
			surface.VF[2] = Vec3i(1, 2, 3);
			surface.VF[3] = Vec3i(2, 0, 1);

			// geneerate normals
			surface.VN.resize(4);
			for (int i = 0; i < 4; ++i) {
				const auto & face = surface.VF[i];
				surface.VN[i] = getTriangleNormal(surface.VV[face.x], surface.VV[face.y], surface.VV[face.z]);
			}

			// generate empty texture mapping
			surface.VT.resize(4);

			return surface;
		}

		void scale(float factor) {

			FW::Vec3f center = (
				vertices_[0] + vertices_[1] + vertices_[2] + vertices_[3]
				) / 4.0f;
			for (int i = 0; i < 4; ++i) {
				vertices_[i] -= center;
				vertices_[i] *= factor;
				vertices_[i] += center;
			};

		}

	};

	class MarchingTetrahedra {

		/*
			Generates the tetrahedras in the cube given as [from,to]
			tetrahedras is a pointer to an array of 6 elements
		*/

	public:
		static void generateTetrahedra(const Vec3f & from, const float cubeLen, std::array<Tetrahedra,6> & tetrahedras);
		static bool intersects(const Vec3f & edgeA, const Vec3f & edgeB, Vec3f & intersectionPoint, const std::function<float(const Vec3f&)> & fieldFunc);
		static Surface march(const Vec3f & from, const Vec3f & to, float cubeLen, std::function<float(const Vec3f&)> & fieldFunc);

	};
}