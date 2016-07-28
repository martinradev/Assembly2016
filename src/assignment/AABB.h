#pragma once

#include "base/Math.hpp"
#include <xmmintrin.h>
#include <immintrin.h>

namespace FW {

	struct AABB {
		Vec3f min, max;

		inline AABB() : min(), max() {}
		inline AABB(const Vec3f& min, const Vec3f& max) : min(min), max(max) {}
		inline float area() const {
			Vec3f d(max - min);
			return 2.0f * (d.x * d.y + d.x * d.z + d.y * d.z);
		}
		inline void extend(const Vec3f & vmin, const Vec3f & vmax) {
			min = FW::min(vmin, min);
			max = FW::max(vmax, max);
		}
		inline void extend(const AABB & aabb) {
			min = FW::min(aabb.min, min);
			max = FW::max(aabb.max, max);
		}
		inline bool isValid() {
			return (min[0] < max[0]) && (min[1] < max[1]) && (min[2] < max[2]);
		}
		inline void enlarge(float delta) {
			min[0] -= delta;
			min[1] -= delta;
			min[2] -= delta;
			max[0] += delta;
			max[1] += delta;
			max[2] += delta;
		}

		inline FW::Vec3f getCenter() const {
			return 0.5f * (min + max);
		}

		inline FW::Vec3f getHalfSize() const {
			return (max - min) * 0.5f;
		}

		inline bool inBox(const FW::Vec3f & v) const {
			static const float eps = 1e-6;
			return (v.x >= min.x - eps && v.y >= min.y - eps && v.z >= min.z - eps) && (v.x <= max.x + eps && v.y <= max.y + eps && v.z <= max.z + eps);
		}
	};



	struct _MM_ALIGN16 SSE_AABB {

		__m128 min;
		__m128 max;

		inline SSE_AABB() :
			min(_mm_setzero_ps()),
			max(_mm_setzero_ps()) {

		};
		inline SSE_AABB(const SSE_AABB & aabb) :
			min(aabb.min),
			max(aabb.max){
		};
		inline SSE_AABB(__m128 min, __m128 max) :
			min(min),
			max(max)
		{
		};
		inline SSE_AABB(const Vec3f& min, const Vec3f& max) :
			min(_mm_set_ps(0.0f, min[2], min[1], min[0])),
			max(_mm_set_ps(0.0f, max[2], max[1], max[0]))
		{

		}
		inline SSE_AABB(const AABB & aabb) :
			min(_mm_set_ps(0.0f, aabb.min[2], aabb.min[1], aabb.min[0])),
			max(_mm_set_ps(0.0f, aabb.max[2], aabb.max[1], aabb.max[0]))
		{

		}
		inline F32 area() const {
			float _MM_ALIGN16 res[4];
			_mm_store_ps(res, _mm_sub_ps(max, min));
			return 2.0f * (res[0] * res[1] + res[0] * res[2] + res[1] * res[2]);
		}
		inline void extend(const FW::Vec3f & vmin, const FW::Vec3f & vmax) {
			min = _mm_min_ps(min, _mm_set_ps(0.0f, vmin[2], vmin[1], vmin[0]));
			max = _mm_max_ps(max, _mm_set_ps(0.0f, vmax[2], vmax[1], vmax[0]));
		}
		inline void extend(__m128 v) {
			min = _mm_min_ps(min, v);
			max = _mm_max_ps(max, v);
		}
		inline void extend(const SSE_AABB & aabb) {
			min = _mm_min_ps(min, aabb.min);
			max = _mm_max_ps(max, aabb.max);
		}
		inline bool isValid() {
			return (_mm_movemask_ps(_mm_cmpge_ps(min, max)) & 0x7) == 0x00;
		}
		inline void enlarge(float delta) {
			min = _mm_sub_ps(min, _mm_set1_ps(delta));
			max = _mm_add_ps(max, _mm_set1_ps(delta));
		}

		inline __m128 getCenter() const {
			return  _mm_mul_ps(_mm_add_ps(min, max), _mm_set1_ps(0.5f));
		}

		inline __m128 getHalfSize() const {
			return _mm_mul_ps(_mm_sub_ps(max, min), _mm_set1_ps(0.5f));
		}

		inline Vec3f getAxisLength() const {
			float _MM_ALIGN16 res[4];
			_mm_store_ps(res, _mm_sub_ps(max, min));
			return Vec3f::fromPtr(res);
		}

		inline AABB toAABB() const {
			float _MM_ALIGN16 resmin[4];
			_mm_store_ps(resmin, min);
			float _MM_ALIGN16 resmax[4];
			_mm_store_ps(resmax, max);
			return AABB(Vec3f::fromPtr(resmin), Vec3f::fromPtr(resmax));
		}

	};

};