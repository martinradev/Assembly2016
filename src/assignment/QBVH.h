#pragma once

#include "AABB.h"
#include "Bvh.hpp"
#include "MemoryAllignedAllocator.h"
#include <vector>
#include <cstdint>

namespace FW {

	class _MM_ALIGN16 QBVH_Node {

		friend class QBVH;

	public:

		QBVH_Node() :
			m_left(0),
			m_segmentStart(0),
			m_mask(0)
		{};

		FORCEINLINE bool isLeaf() const {
			return m_left & (1 << 31);
		}

		FORCEINLINE void makeLeaf() {
			m_left |= (1 << 31);
		}

		FORCEINLINE unsigned getAvxIndex() const {
			// 0x7FFFFFFF we clear off the first 1 bits
			return m_left & (0x7FFFFFFF);
		}

		FORCEINLINE void setAvxIndex(unsigned index) {
			m_left = (1u << 31) | index;
		}

		FORCEINLINE void setNumChildren(unsigned num) {
			m_mask |= (num & (0x7));
		}

		FORCEINLINE unsigned getNumChildren() const {
			return m_mask & 0x7;
		}

		FORCEINLINE unsigned getLeftChild() const {
			return m_left & (0x7FFFFFFF);
		}

	private:

		__m128 m_minx;
		__m128 m_miny;
		__m128 m_minz;
		__m128 m_maxx;
		__m128 m_maxy;
		__m128 m_maxz;

		/*
			most significant tells whether it is a leaf
			index in the avx triangle buffer is stored if it is a leaf
		*/
		unsigned m_left;

		/*
			start of the segment in the actual triangle buffer
		*/
		unsigned m_segmentStart;

		// allignment data
		// first 3 bits is the number of children
		uint64_t m_mask;


	};

	class QBVH {
	public:
		QBVH(const BVH & bvh);

		bool isOccluded(const Vec3f & orig, const Vec3f & dir, float tmin) const;
		bool intersect(const Vec3f & orig, const Vec3f & dir, float & tmin, float & umin, float & vmin, int & idx) const;
	
	private:

		std::vector<AVX_TriangleIntersectionInput, aligned_allocator<AVX_TriangleIntersectionInput, 16> > m_avxTriangleData;
		std::vector<int> m_indices;

		// 4 x minx, 4 x miny, 4 x minz, 4 x maxx, 4 x maxy, 4 x maxz,
		// segment start 1, left 1, seg start 2, left 2, seg start 3, left 3, seg start 4, left 4

		std::vector<QBVH_Node, aligned_allocator<QBVH_Node, 16> > m_nodes;
		const std::vector<RTTriangle> * m_triangles;

		FORCEINLINE void fillSseBBData(const BVH & bvh, unsigned qNodeIndex, int children[4], unsigned numChildren);
		FORCEINLINE void fillChildren(const BVH & bvh, unsigned nodeIndex, int children[4], unsigned & numChildren);

	};

};