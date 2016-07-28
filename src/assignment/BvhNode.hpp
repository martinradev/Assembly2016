#pragma once

#include "AABB.h"

namespace FW {

	class BVHNode {

		friend class SplitMethod;
		friend class BVH;
		friend class QBVH;

	public:

		BVHNode() {}
		BVHNode(const AABB & aabb, unsigned left, unsigned segmentStart) :
			m_aabb(aabb),
			m_left(left),
			m_segmentStart(segmentStart)
		{

		}

		FORCEINLINE bool isLeaf() const {
			return (m_left & (1u << 31)) != 0;
		}

		FORCEINLINE unsigned getAvxIndex() const {
			// 0x7FFFFFFF we clear off the first bit
			return m_left & (0x7FFFFFFF);
		}

		FORCEINLINE void setAvxIndex(unsigned index) {
			m_left = ((1u << 31) | index);
		}

		FORCEINLINE unsigned getLeftChild() const {
			return m_left & 0x7FFFFFFF;
		}

		FORCEINLINE unsigned getRightChild() const {
			return getLeftChild() + 1;
		}

	private:
		/*
			bounding box for the node
		*/
		AABB m_aabb;

		/*
			index of the left child
			-	the right child index is m_left+1
			-	if it is a leaf node, the 32th bit is set to 1
			-	if it is a leaf node, 
				the index will point to the index in the sse triangle buffer
		*/
		unsigned m_left;

		/*
			index to the first triangle which is included in this node
		*/
		unsigned m_segmentStart;
	};


};
