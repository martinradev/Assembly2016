#pragma once

#include "base/Math.hpp"

namespace FW {

	class BezierCurve {

	public:
		static Vec3f evalBezier(
			const Vec3f & p0,
			const Vec3f & p1,
			const Vec3f & p2,
			const Vec3f & p3,
			const float t) {

			float a = 1.0f - t;

			return
				a*a*a*p0 +
				3.0f*a*a*t*p1 +
				3.0f*a*t*t*p2 +
				t*t*t*p3;

		}
	private:

	};
	
	class CarmullRomCurve {

	public:
		static Vec3f evalCatmullRom(
			const Vec3f & p0,
			const Vec3f & p1,
			const Vec3f & p2,
			const Vec3f & p3,
			const float t) {


			return
				0.5f * (
					2.0f * p1 + (p2 - p0)*t +
					(2.0f * p0 - 5.0f*p1 + 4.0f*p2 - p3) *t*t +
					(-p0 + 3.0f*p1 - 3.0f*p2 + p3)*t*t*t
					);

		}
	private:


	};

};