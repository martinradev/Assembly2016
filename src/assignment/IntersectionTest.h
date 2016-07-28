#pragma once

#include "base/Math.hpp"
#include <stdio.h>
#include <xmmintrin.h>
#include <immintrin.h>

namespace FW {

	/*
		Triangle data for 1 ray 4 triangles intersection test
	*/
	struct SSE_TriangleIntersectionInput {

		// 4 3x3 matrices
		__m128 datax1, datax2, datax3, datay1, datay2, datay3, dataz1, dataz2, dataz3;

		// normals
		__m128 nx, ny, nz;

		// offset to find the position
		int off;
	};

	/*
		Triangle data for 1 ray 8 triangles intersection test
	*/
	struct AVX_TriangleIntersectionInput {

		// 8 3x3 matrices
		__m256 datax1, datax2, datax3, datay1, datay2, datay3, dataz1, dataz2, dataz3;

		// normals
		__m256 nx, ny, nz;

		// offset to find the position
		int off;
	};

	/*
		Ray information for SSE intersection tests
		can be used with 
	*/
	struct SSE_RayInput {

		SSE_RayInput() {};
		SSE_RayInput(const FW::Vec3f & orig, const FW::Vec3f & dir, const float t) {

			ox = _mm_set1_ps(orig.x);
			oy = _mm_set1_ps(orig.y);
			oz = _mm_set1_ps(orig.z);
			dx = _mm_set1_ps(dir.x);
			dy = _mm_set1_ps(dir.y);
			dz = _mm_set1_ps(dir.z);
			invdx = _mm_set1_ps(1.0f / dir.x);
			invdy = _mm_set1_ps(1.0f / dir.y);
			invdz = _mm_set1_ps(1.0f / dir.z);
			tmin = _mm_set1_ps(t);
		}

		__m128 ox, oy, oz, dx, dy, dz, invdx, invdy, invdz, tmin;
	};

	/*
	Ray information for AVX intersection tests
	can be used with
	*/
	struct AVX_RayInput {

		AVX_RayInput() {};
		AVX_RayInput(const FW::Vec3f & orig, const FW::Vec3f & dir, const float t) {

			ox = _mm256_set1_ps(orig.x);
			oy = _mm256_set1_ps(orig.y);
			oz = _mm256_set1_ps(orig.z);
			dx = _mm256_set1_ps(dir.x);
			dy = _mm256_set1_ps(dir.y);
			dz = _mm256_set1_ps(dir.z);
			invdx = _mm256_set1_ps(1.0f / dir.x);
			invdy = _mm256_set1_ps(1.0f / dir.y);
			invdz = _mm256_set1_ps(1.0f / dir.z);
			tmin = _mm256_set1_ps(t);
		}

		__m256 ox, oy, oz, dx, dy, dz, invdx, invdy, invdz, tmin;
	};

	/*
		finds the intersection among all 4 ray triangle intersections
	*/
#define MY_COMPARE_T(index, t, u, v, idx) \
if (t > tmpt[index] && tmpt[index] > 0.0f) {\
t = tmpt[index]; \
u = tmpu[index]; \
v = tmpv[index]; \
idx = index; \
}

	/*
		does 1 ray - 4 triangles intersection tests
	*/
	FORCEINLINE bool sse_triangle_intersect(const SSE_TriangleIntersectionInput & data, const SSE_RayInput & rayData, float & t, float & u, float & v, int & idx) {

		static const __m128 negOne = _mm_set1_ps(-1.0f);
		static const __m128 posOne = _mm_set1_ps(1.0f);
		static const __m128 zerops = _mm_setzero_ps();

		__m128 x = _mm_mul_ps(data.datax1, rayData.ox);
		__m128 y = _mm_mul_ps(data.datay1, rayData.oy);
		__m128 z = _mm_mul_ps(data.dataz1, rayData.oz);
		__m128 tr_origx = _mm_add_ps(_mm_add_ps(x, data.nx), _mm_add_ps(y, z));

		x = _mm_mul_ps(data.datax2, rayData.ox);
		y = _mm_mul_ps(data.datay2, rayData.oy);
		z = _mm_mul_ps(data.dataz2, rayData.oz);
		__m128 tr_origy = _mm_add_ps(_mm_add_ps(x, data.ny), _mm_add_ps(y, z));

		x = _mm_mul_ps(data.datax3, rayData.ox);
		y = _mm_mul_ps(data.datay3, rayData.oy);
		z = _mm_mul_ps(data.dataz3, rayData.oz);
		__m128 tr_origz = _mm_add_ps(_mm_add_ps(x, data.nz), _mm_add_ps(y, z));

		x = _mm_mul_ps(data.datax1, rayData.dx);
		y = _mm_mul_ps(data.datay1, rayData.dy);
		z = _mm_mul_ps(data.dataz1, rayData.dz);
		__m128 tr_dirx = _mm_add_ps(x, _mm_add_ps(y, z));

		x = _mm_mul_ps(data.datax2, rayData.dx);
		y = _mm_mul_ps(data.datay2, rayData.dy);
		z = _mm_mul_ps(data.dataz2, rayData.dz);
		__m128 tr_diry = _mm_add_ps(x, _mm_add_ps(y, z));

		x = _mm_mul_ps(data.datax3, rayData.dx);
		y = _mm_mul_ps(data.datay3, rayData.dy);
		z = _mm_mul_ps(data.dataz3, rayData.dz);
		__m128 tr_dirz = _mm_add_ps(x, _mm_add_ps(y, z));

		__m128 sse_t = _mm_mul_ps(_mm_div_ps(tr_origz, tr_dirz), negOne);

		__m128 mask = _mm_and_ps(_mm_cmpnle_ps(sse_t, zerops), _mm_cmpnge_ps(sse_t, rayData.tmin));
		if (_mm_movemask_ps(mask) == 0x00) {
			return false;
		}

		__m128 sse_u = _mm_add_ps(tr_origx, _mm_mul_ps(sse_t, tr_dirx));
		__m128 sse_v = _mm_add_ps(tr_origy, _mm_mul_ps(sse_t, tr_diry));

		mask = _mm_and_ps(mask, _mm_and_ps(_mm_cmpnle_ps(sse_u, zerops), _mm_cmpnle_ps(sse_v, zerops)));
		if (_mm_movemask_ps(mask) == 0x00) {
			return false;
		}

		mask = _mm_and_ps(mask, _mm_cmpnge_ps(_mm_add_ps(sse_u, sse_v), posOne));
		if (_mm_movemask_ps(mask) == 0x00) {
			return false;
		}

		// make zeros, 2s
		sse_t = _mm_and_ps(mask, sse_t);

		sse_u = _mm_and_ps(mask, sse_u);
		sse_v = _mm_and_ps(mask, sse_v);


		// return min from all values from result

		float _MM_ALIGN16 tmpt[4];
		float _MM_ALIGN16 tmpu[4];
		float _MM_ALIGN16 tmpv[4];

		_mm_store_ps(tmpt, sse_t);
		_mm_store_ps(tmpu, sse_u);
		_mm_store_ps(tmpv, sse_v);

		MY_COMPARE_T(0, t, u, v, idx);
		MY_COMPARE_T(1, t, u, v, idx);
		MY_COMPARE_T(2, t, u, v, idx);
		MY_COMPARE_T(3, t, u, v, idx);


		return idx != -1;

	}

	FORCEINLINE bool avx_triangle_intersect(const AVX_TriangleIntersectionInput & data, const AVX_RayInput & rayData, float & t, float & u, float & v, int & idx) {

		static const __m256 negOne = _mm256_set1_ps(-1.0f);
		static const __m256 posOne = _mm256_set1_ps(1.0f);
		static const __m256 zerops = _mm256_setzero_ps();

		__m256 x = _mm256_mul_ps(data.datax1, rayData.ox);
		__m256 y = _mm256_mul_ps(data.datay1, rayData.oy);
		__m256 z = _mm256_mul_ps(data.dataz1, rayData.oz);
		__m256 tr_origx = _mm256_add_ps(_mm256_add_ps(x, data.nx), _mm256_add_ps(y, z));

		x = _mm256_mul_ps(data.datax2, rayData.ox);
		y = _mm256_mul_ps(data.datay2, rayData.oy);
		z = _mm256_mul_ps(data.dataz2, rayData.oz);
		__m256 tr_origy = _mm256_add_ps(_mm256_add_ps(x, data.ny), _mm256_add_ps(y, z));

		x = _mm256_mul_ps(data.datax3, rayData.ox);
		y = _mm256_mul_ps(data.datay3, rayData.oy);
		z = _mm256_mul_ps(data.dataz3, rayData.oz);
		__m256 tr_origz = _mm256_add_ps(_mm256_add_ps(x, data.nz), _mm256_add_ps(y, z));

		x = _mm256_mul_ps(data.datax1, rayData.dx);
		y = _mm256_mul_ps(data.datay1, rayData.dy);
		z = _mm256_mul_ps(data.dataz1, rayData.dz);
		__m256 tr_dirx = _mm256_add_ps(x, _mm256_add_ps(y, z));

		x = _mm256_mul_ps(data.datax2, rayData.dx);
		y = _mm256_mul_ps(data.datay2, rayData.dy);
		z = _mm256_mul_ps(data.dataz2, rayData.dz);
		__m256 tr_diry = _mm256_add_ps(x, _mm256_add_ps(y, z));

		x = _mm256_mul_ps(data.datax3, rayData.dx);
		y = _mm256_mul_ps(data.datay3, rayData.dy);
		z = _mm256_mul_ps(data.dataz3, rayData.dz);
		__m256 tr_dirz = _mm256_add_ps(x, _mm256_add_ps(y, z));

		__m256 sse_t = _mm256_mul_ps(_mm256_div_ps(tr_origz, tr_dirz), negOne);

		__m256 mask = _mm256_and_ps(_mm256_cmp_ps(sse_t, zerops, _CMP_NLE_US), _mm256_cmp_ps(sse_t, rayData.tmin, _CMP_NGE_US));
		if (_mm256_movemask_ps(mask) == 0x00) {
			return false;
		}

		__m256 sse_u = _mm256_add_ps(tr_origx, _mm256_mul_ps(sse_t, tr_dirx));
		__m256 sse_v = _mm256_add_ps(tr_origy, _mm256_mul_ps(sse_t, tr_diry));
		
		/*	you probably don't support fused instructions ...
		__m256 sse_u = _mm256_fmadd_ps(sse_t, tr_dirx, tr_origx);
		__m256 sse_v = _mm256_fmadd_ps(sse_t, tr_diry, tr_origy);
		*/

		mask = _mm256_and_ps(mask, _mm256_and_ps(_mm256_cmp_ps(sse_u, zerops, _CMP_GE_OS), _mm256_cmp_ps(sse_v, zerops, _CMP_GE_OS)));

		mask = _mm256_and_ps(mask, _mm256_cmp_ps(_mm256_add_ps(sse_u, sse_v), posOne, _CMP_NGE_US));
		if (_mm256_movemask_ps(mask) == 0x00) {
			return false;
		}

		// make zeros, 2s
		sse_t = _mm256_and_ps(mask, sse_t);

		sse_u = _mm256_and_ps(mask, sse_u);
		sse_v = _mm256_and_ps(mask, sse_v);


		// return min from all values from result

		float _MM_ALIGN16 tmpt[8];
		float _MM_ALIGN16 tmpu[8];
		float _MM_ALIGN16 tmpv[8];

		_mm256_store_ps(tmpt, sse_t);
		_mm256_store_ps(tmpu, sse_u);
		_mm256_store_ps(tmpv, sse_v);

		float t0 = t, u0 = u, v0 = v;
		int idx0 = idx;

		MY_COMPARE_T(0, t0, u0, v0, idx0);
		MY_COMPARE_T(1, t, u, v, idx);
		MY_COMPARE_T(2, t0, u0, v0, idx0);
		MY_COMPARE_T(3, t, u, v, idx);
		MY_COMPARE_T(4, t0, u0, v0, idx0);
		MY_COMPARE_T(5, t, u, v, idx);
		MY_COMPARE_T(6, t0, u0, v0, idx0);
		MY_COMPARE_T(7, t, u, v, idx);

		if (t0 > 0.0f && (t0 <= t || t < 0.0f)) {
			// use t0 results
			t = t0;
			u = u0;
			v = v0;
			idx = idx0;
		}

		return idx != -1;

	}

#undef MY_COMPARE(T)

	static const float flt_plus_inf = -logf(0);
	static const float _MM_ALIGN16
		ps_cst_plus_inf[4] = { flt_plus_inf, flt_plus_inf, flt_plus_inf, flt_plus_inf },
		ps_cst_minus_inf[4] = { -flt_plus_inf, -flt_plus_inf, -flt_plus_inf, -flt_plus_inf };


	/*
		structure to keep information for 1 ray 4 aabbs intersection tests
	*/
	struct _MM_ALIGN16 SSE_4AABB_1RAY {

		FORCEINLINE SSE_4AABB_1RAY(__m128 oxIn, __m128 oyIn, __m128 ozIn, __m128 invdxIn, __m128 invdyIn, __m128 invdzIn, __m128 tminIn) :
			ox(oxIn),
			oy(oyIn),
			oz(ozIn),
			invdx(invdxIn),
			invdy(invdyIn),
			invdz(invdzIn),
			tmin(tminIn)
		{

		}

		FORCEINLINE void setupBoxIntersection(__m128 minxIn, __m128 minyIn, __m128 minzIn, __m128 maxxIn, __m128 maxyIn, __m128 maxzIn) {
			minx = minxIn;
			miny = minyIn;
			minz = minzIn;
			maxx = maxxIn;
			maxy = maxyIn;
			maxz = maxzIn;
		}

		__m128 ox;
		__m128 oy;
		__m128 oz;
		__m128 invdx;
		__m128 invdy;
		__m128 invdz;
		__m128 minx;
		__m128 miny;
		__m128 minz;
		__m128 maxx;
		__m128 maxy;
		__m128 maxz;
		__m128 tmin;
		__m128 t;
		__m128 tbegin;
		__m128 tend;
		__m128 result;

	};

	/*
		does 1 ray 4 aabbs intersection test
	*/
	FORCEINLINE static int intersect4AABB1ray(
		SSE_4AABB_1RAY & data
		) {

		static const __m128 plus_inf = _mm_load_ps(ps_cst_plus_inf);
		static const __m128 minus_inf = _mm_load_ps(ps_cst_minus_inf);
		static const __m128 zerops = _mm_setzero_ps();
		static const __m128 oneps = _mm_set1_ps(1.0f);

		// process the x dimension
		__m128 t1 = _mm_mul_ps(_mm_sub_ps(data.minx, data.ox), data.invdx);
		__m128 t2 = _mm_mul_ps(_mm_sub_ps(data.maxx, data.ox), data.invdx);


		__m128 filtered_l1a = _mm_min_ps(t1, plus_inf);
		__m128 filtered_l2a = _mm_min_ps(t2, plus_inf);

		__m128 filtered_l1b = _mm_max_ps(t1, minus_inf);
		__m128 filtered_l2b = _mm_max_ps(t2, minus_inf);


		__m128 ts = _mm_min_ps(filtered_l1b, filtered_l2b);
		__m128 te = _mm_max_ps(filtered_l1a, filtered_l2a);
		// we have ts and te for the x dimension

		// process the y dimension
		t1 = _mm_mul_ps(_mm_sub_ps(data.miny, data.oy), data.invdy);
		t2 = _mm_mul_ps(_mm_sub_ps(data.maxy, data.oy), data.invdy);

		filtered_l1a = _mm_min_ps(t1, plus_inf);
		filtered_l2a = _mm_min_ps(t2, plus_inf);

		filtered_l1b = _mm_max_ps(t1, minus_inf);
		filtered_l2b = _mm_max_ps(t2, minus_inf);
		t1 = _mm_min_ps(filtered_l1b, filtered_l2b);
		t2 = _mm_max_ps(filtered_l1a, filtered_l2a);

		ts = _mm_max_ps(ts, t1);
		te = _mm_min_ps(te, t2);
		// we have ts and te for the y dimension

		// process the z dimension
		t1 = _mm_mul_ps(_mm_sub_ps(data.minz, data.oz), data.invdz);
		t2 = _mm_mul_ps(_mm_sub_ps(data.maxz, data.oz), data.invdz);

		filtered_l1a = _mm_min_ps(t1, plus_inf);
		filtered_l2a = _mm_min_ps(t2, plus_inf);

		filtered_l1b = _mm_max_ps(t1, minus_inf);
		filtered_l2b = _mm_max_ps(t2, minus_inf);
		t1 = _mm_min_ps(filtered_l1b, filtered_l2b);
		t2 = _mm_max_ps(filtered_l1a, filtered_l2a);

		ts = _mm_max_ps(ts, t1);
		te = _mm_min_ps(te, t2);

		data.result = _mm_and_ps(_mm_cmpge_ps(te, ts), _mm_and_ps(_mm_cmpge_ps(te, zerops), _mm_cmple_ps(ts, data.tmin)));

		data.tbegin = ts;
		data.tend = te;
		ts = _mm_max_ps(ts, zerops);
		data.t = ts;

		return _mm_movemask_ps(data.result);
	}


	// yet another box intersection piece of code http://people.csail.mit.edu/amy/papers/box-jgt.pdf
	FORCEINLINE static bool intersect_aabb(const float o[3], const int sign[3], const float invd[3], const float * bounds, float t, float & tbegin, float & tend) {

		float tmin, tmax, tymin, tymax, tzmin, tzmax;

		tmin = (bounds[sign[0] * 3] - o[0]) * invd[0];
		tmax = (bounds[(1 - sign[0]) * 3] - o[0]) * invd[0];
		tymin = (bounds[(sign[1] * 3 + 1)] - o[1]) * invd[1];
		tymax = (bounds[(1 - sign[1]) * 3 + 1] - o[1]) * invd[1];
		if ((tmin > tymax) || (tymin > tmax))
			return false;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;
		tzmin = (bounds[sign[2] * 3 + 2] - o[2]) * invd[2];
		tzmax = (bounds[(1 - sign[2]) * 3 + 2] - o[2]) * invd[2];
		if ((tmin > tzmax) || (tzmin > tmax))
			return false;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;
		tbegin = tmin;
		tend = tmax;
		return ((tmin <= t) && (tmax >= 0.0f));

	}

	FORCEINLINE bool intersect_sphere(const Vec3f & ro, const Vec3f & rd, const Vec3f & so, const float r, float & t) {
		Vec3f tmp = so - ro;

		float A = dot(rd, rd);
		float B = -2 * dot(rd, tmp);
		float C = dot(tmp, tmp) - sqr(r);
		float radical = B*B - 4 * A*C;
		
		if (radical < 0)
			return false;

		radical = sqrtf(radical);
		float t_m = (-B - radical) / (2 * A);
		float t_p = (-B + radical) / (2 * A);
		
		bool flag = false;
		
		if (t_m >= 0.0 && t_m <= t) {
			t = t_m;
			flag = true;
		}
		if (t_p >= 0.0 && t_p <= t) {
			t = t_p;
			flag = true;
		}
		
		return flag;
	}

};