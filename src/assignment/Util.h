#pragma once

#include "surf.h"
#include "RTTriangle.hpp"
#include <3d/Mesh.hpp>
#include <utility>
#include <vector>

struct Vertex {
	FW::Vec3f vertex_;
	FW::Vec3f normal_;
	FW::Vec2f uv_;
	float time_;

	Vertex(){};

	Vertex(const FW::Vec3f & vertex, const FW::Vec3f & normal, const FW::Vec2f & uv, float time) :
		vertex_(vertex),
		normal_(normal),
		uv_(uv),
		time_(time)
	{}

};

struct SimplerVertex {
	FW::Vec3f vertex_;
	FW::Vec3f normal_;
	FW::Vec2f uv_;

	SimplerVertex(){};

	SimplerVertex(const FW::Vec3f & vertex, const FW::Vec3f & normal, const FW::Vec2f & uv) :
		vertex_(vertex),
		normal_(normal),
		uv_(uv)
	{
		
	}
};


std::pair<GLuint, GLuint> get2DPlane();
void render2DPlane(GLuint vao, GLuint vbo);

FORCEINLINE int sign(float value) {
	if (value < 0.0f) return -1;
	if (value > 0.0f) return 1;
	return 0;
}

FORCEINLINE FW::Vec3f getTriangleNormal(const FW::Vec3f & a, const FW::Vec3f & b, const FW::Vec3f & c) {

	return cross(c-a, b-a).normalized();

}

Surface genSurfaceFromVerticesOnly(const std::vector<FW::Vec3f> & vertices);

namespace FW {

	void decomposeMesh(Mesh<VertexPNTC> * mesh, std::vector<RTTriangle> & triangles, std::vector <MeshBase::Material*> & materials);

	template<typename T> T barycentricInterpolation(float u, float v, const T & a, const T & b, const T & c) {
		return (1.0f - u - v) * a + u * b + v * c;
	}


	// from the book "Physicall based rendering"
	FORCEINLINE float VanDerCorput(unsigned n, unsigned scramble) {
		n = (n << 16) | (n >> 16);
		n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
		n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
		n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
		n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
		n ^= scramble;
		return (float)n / (float)0x100000000LL;
	}

	FORCEINLINE float Sobol2(unsigned int n, unsigned int scramble) {
		for (unsigned v = 1 << 31; n != 0; n >>= 1, v ^= v >> 1)
			if (n & 0x1) scramble ^= v;
		return (float)scramble / (float)0x100000000LL;
	}

	FORCEINLINE Vec4f matToQuaternion(const Mat4f & rot) {

		float tr = rot(0, 0) + rot(1, 1) + rot(2, 2);
		float x, y, z, w;
		if (tr > 0) {
			float s = sqrtf(tr + 1.0f) * 2.0f;
			x = (rot(2, 1) - rot(1, 2)) / s;
			y = (rot(0, 2) - rot(2, 0)) / s;
			z = (rot(1, 0) - rot(0, 1)) / s;
			w = 0.25f * s;
		}
		else if ((rot(0, 0) > rot(1, 1)) & (rot(0, 0) > rot(2, 2))) {
			float s = sqrtf(1.0f + rot(0, 0) - rot(1, 1) - rot(2, 2)) * 2.0f;
			w = (rot(2, 1) - rot(1, 2)) / s;
			x = 0.25f * s;
			y = (rot(0, 1) + rot(1, 0)) / s;
			z = (rot(0, 2) + rot(2, 0)) / s;
		}
		else if (rot(1, 1) > rot(2, 2)) {
			float s = sqrtf(1.0f + rot(1, 1) - rot(0, 0) - rot(2, 2)) * 2.0f;
			w = (rot(0, 2) - rot(2, 0)) / s;
			x = (rot(0, 1) + rot(1, 0)) / s;
			y = 0.25f * s;
			z = (rot(1, 2) + rot(2, 1)) / s;
		}
		else {
			float s = sqrtf(1.0f + rot(2, 2) - rot(0, 0) - rot(1, 1)) * 2.0f;
			w = (rot(1, 0) - rot(0, 1)) / s;
			x = (rot(0, 2) + rot(2, 0)) / s;
			y = (rot(1, 2) + rot(2, 1)) / s;
			z = 0.25f * s;
		}
		return Vec4f(x, y, z, w);
	}

	FORCEINLINE FW::Mat4f quaternionToMatrix(const Vec4f & q) {
		FW::Mat4f R;

		R.setRow(0, FW::Vec4f(1.0f - 2.0f * (q.y * q.y + q.z * q.z), 2.0f * (q.x * q.y - q.z * q.w), 2.0f * (q.x * q.z + q.y * q.w), 0.0f));
		R.setRow(1, FW::Vec4f(2.0f * (q.x * q.y + q.z * q.w), 1.0f - 2.0f * (q.x * q.x + q.z * q.z), 2.0f * (q.y * q.z - q.x * q.w), 0.0f));
		R.setRow(2, FW::Vec4f(2.0f * (q.x * q.z - q.y * q.w), 2.0f * (q.y * q.z + q.x * q.w), 1.0f - 2.0f * (q.x * q.x + q.y * q.y), 0.0f));
		R.setRow(3, FW::Vec4f(0, 0, 0, 1));

		return R;
	}

	FORCEINLINE Vec4f slerp(const Vec4f & q0, const Vec4f & q1, float t) {
		static const float EPS = 1e-9;

		Vec4f q0_ = q0;

		float cCos = dot(q0, q1);
		float angle = acosf(cCos);
		if (cCos < 0.0f) {
			q0_ = -q0;
		}
		if (cCos >= 1.0f - EPS && cCos <= 1.0f + EPS) return q1;
		Vec4f result = (q0_ * sinf((1.0f - t) * angle) + q1 * sinf(t*angle)) / sinf(angle);
		return result.normalized();

	}

	FORCEINLINE Mat4f rot(const Vec3f & rotAngle) {
		Mat3f res3 = Mat3f::rotation(Vec3f(1, 0, 0), rotAngle.x)
			*Mat3f::rotation(Vec3f(0, 1, 0), rotAngle.y) * Mat3f::rotation(Vec3f(0, 0, 1), rotAngle.z);
		Mat4f res4;
		res4.setRow(0, Vec4f(res3.getRow(0), 0.0f));
		res4.setRow(1, Vec4f(res3.getRow(1), 0.0f));
		res4.setRow(2, Vec4f(res3.getRow(2), 0.0f));
		res4.setRow(3, Vec4f(0.0, 0.0, 0.0f, 1.0f));
		return res4;
	}

	FORCEINLINE Mat4f toMat4f(const Mat3f & m) {
		Mat4f mat;
		mat.setRow(0, Vec4f(m.getRow(0), 0));
		mat.setRow(1, Vec4f(m.getRow(1), 0));
		mat.setRow(2, Vec4f(m.getRow(2), 0));
		mat.setRow(3, Vec4f(m.getRow(3), 1));
		return mat;
	}

};