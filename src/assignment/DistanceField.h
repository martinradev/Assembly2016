#pragma once

#include <functional>

#include "base/Math.hpp"

FORCEINLINE std::function<float(const FW::Vec3f&)> getSphereDF(const FW::Vec3f & center, float radius) {

	auto lambda = [center, radius](const FW::Vec3f & point) -> float {

		return (point - center).length() - radius;

	};
	return lambda;
}

FORCEINLINE std::function<float(const FW::Vec3f&)> getPlaneDF() {

	auto lambda = [](const FW::Vec3f & point) -> float {

		return point.y;

	};
	return lambda;
}

FORCEINLINE std::function<float(const FW::Vec3f&)> getTorusDF(const FW::Vec3f & center, float r1, float r2) {

	auto lambda = [center, r1, r2](const FW::Vec3f & point) -> float {

		FW::Vec2f q = FW::Vec2f(FW::Vec2f(point.x, point.z).length() - r1, point.y);
		return q.length() - r2;

	};
	return lambda;
}

FORCEINLINE float smin(float a, float b, float k)
{
	float res = FW::exp(-k*a) + FW::exp(-k*b);
	return -FW::log(res) / k;
}

FORCEINLINE float opS(float a, float b) {
	return FW::max(-b, a);
}

FORCEINLINE std::function<float(const FW::Vec3f&)> getTestDF() {
	static const auto sph1 = getSphereDF(FW::Vec3f(-0.2f), 0.4f);
	static const auto sph2 = getSphereDF(FW::Vec3f(0.2f), 0.4f);
	static const auto sph3 = getSphereDF(FW::Vec3f(0.0f, 0.75f, 0.0f), 0.4f);
	auto lambda = [](const FW::Vec3f & point) -> float {

		return smin(smin(sph1(point), sph2(point), 32), sph3(point), 32);

	};
	return lambda;
}

FORCEINLINE std::function<float(const FW::Vec3f&)> getTestDF2() {
	static const auto sph1 = getSphereDF(FW::Vec3f(0.1f), 0.4f);
	static const auto sph2 = getSphereDF(FW::Vec3f(0.2f), 0.4f);
	auto lambda = [](const FW::Vec3f & point) -> float {

		return opS(sph1(point), sph2(point));

	};
	return lambda;
}

FORCEINLINE std::function<float(const FW::Vec3f&)> getTestDF3() {
	auto lambda = [](const FW::Vec3f & point) -> float {
		
		float f1 = point.x*point.x + point.z * point.z * (9.0f / 4.0f) + point.y * point.y - 1.0f;
		float f2 = point.x * point.x * point.y * point.y * point.y;
		float f3 = point.z * point.z * point.y * point.y * point.y * (9.0f / 200.0f);
		return f1*f1*f1 - f2 - f3;


	};
	return lambda;
}