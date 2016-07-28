#include "QBVH.h"
#include <cassert>

namespace FW {

	static struct QBVH_StackEntry {

		QBVH_StackEntry() {};
		QBVH_StackEntry(unsigned qbvhNode, unsigned BVHNode) :
			m_qbvhNode(qbvhNode),
			m_bvhNode(BVHNode) {};

		unsigned m_qbvhNode;
		unsigned m_bvhNode;

	};

	QBVH::QBVH(const BVH & bvh) : m_triangles(bvh.m_triangles) {

		// triangle and index buffers are the same
		// we just have to compact the 2 way bvh into a 4 way bvh
		m_avxTriangleData.assign(bvh.m_avxTriangleData.begin(), bvh.m_avxTriangleData.end());
		m_indices.assign(bvh.m_indices.begin(), bvh.m_indices.end());
		m_nodes.resize(bvh.m_nodes.size());

		// stack and some bookkeeping
		QBVH_StackEntry st[256];
		st[0] = QBVH_StackEntry(0, 0);
		unsigned stSize = 1;
		unsigned nodesAdded = 1;

		int children[4];
		unsigned numChildren;

		unsigned numNodes = 1;

		while (stSize != 0) {

			QBVH_StackEntry top = st[--stSize];
			const BVHNode & curNode = bvh.m_nodes[top.m_bvhNode];
			QBVH_Node & qbvhNode = m_nodes[top.m_qbvhNode];
			qbvhNode.m_segmentStart = curNode.m_segmentStart;
			// go one more level down and fill children information
			fillChildren(bvh, top.m_bvhNode, children, numChildren);

			// if there aren't any children, we are at a leaf
			if (numChildren == 0) {
				// it is a leaf
				qbvhNode.makeLeaf();
				qbvhNode.setAvxIndex(curNode.getAvxIndex());
				continue;
			}

			// fill the child bb data
			fillSseBBData(bvh, top.m_qbvhNode, children, numChildren);

			qbvhNode.m_left = numNodes;
			qbvhNode.setNumChildren(numChildren);

			// push all children to the stack
			for (int i = 0; i < numChildren; ++i) {
				st[stSize++] = QBVH_StackEntry(numNodes, children[i]);
				++numNodes;
			}

		}


	}

	void QBVH::fillSseBBData(const BVH & bvh, unsigned qNodeIndex, int children[4], unsigned numChildren) {

		// go over each child and store the boundaries of its aabb
		QBVH_Node & qNode = m_nodes[qNodeIndex];

		for (int i = 0; i < numChildren; ++i) {
			const BVHNode & bvhNode = bvh.m_nodes[children[i]];

			qNode.m_maxx.m128_f32[i] = bvhNode.m_aabb.max.x;
			qNode.m_maxy.m128_f32[i] = bvhNode.m_aabb.max.y;
			qNode.m_maxz.m128_f32[i] = bvhNode.m_aabb.max.z;

			qNode.m_minx.m128_f32[i] = bvhNode.m_aabb.min.x;
			qNode.m_miny.m128_f32[i] = bvhNode.m_aabb.min.y;
			qNode.m_minz.m128_f32[i] = bvhNode.m_aabb.min.z;


		}
		// set 0s for every missing child
		while (numChildren < 4) {
			qNode.m_maxx.m128_f32[numChildren] = qNode.m_maxy.m128_f32[numChildren] = qNode.m_maxz.m128_f32[numChildren] = 0.0f;
			qNode.m_minx.m128_f32[numChildren] = qNode.m_miny.m128_f32[numChildren] = qNode.m_minz.m128_f32[numChildren] = 0.0f;
			++numChildren;
		}
	}

	void QBVH::fillChildren(const BVH & bvh, unsigned nodeIndex, int children[4], unsigned & numChildren) {

		const BVHNode & node = bvh.m_nodes[nodeIndex];

		// invalidate children
		children[0] = children[1] = children[2] = children[3] = -1;
		numChildren = 0;

		if (node.isLeaf()) {
			// no children
			return;
		}


		const BVHNode & lNode = bvh.m_nodes[node.getLeftChild()];
		
		if (lNode.isLeaf()) {
			// cannot go down any futher -> left and right
			// add only this one
			children[numChildren++] = node.getLeftChild();

		}
		else {
			// left child has 2 children, add both of them
			children[numChildren++] = lNode.getLeftChild();
			children[numChildren++] = lNode.getRightChild();
		}

		const BVHNode & rNode = bvh.m_nodes[node.getRightChild()];

		if (rNode.isLeaf()) {

			// add only this one
			children[numChildren++] = node.getRightChild();

		}
		else {
			// expand again and add both of them
			// right child has 2 children, add both of them
			children[numChildren++] = rNode.getLeftChild();
			children[numChildren++] = rNode.getRightChild();
		}

	}
	
	bool QBVH::isOccluded(const Vec3f & orig, const Vec3f & dir, float tmin) const {
		unsigned st[256];
		unsigned stSize = 1;
		st[0] = 0;

		AVX_RayInput rayData(orig, dir, tmin);
		SSE_RayInput sseRayData(orig, dir, tmin);

		SSE_4AABB_1RAY intersectData(sseRayData.ox, sseRayData.oy, sseRayData.oz, sseRayData.invdx, sseRayData.invdy, sseRayData.invdz, sseRayData.tmin);

		while (stSize != 0) {

			unsigned top = st[--stSize];

			const QBVH_Node * curNode = &m_nodes[top];

			if (curNode->isLeaf()) {

				float t = tmin, u, v;
				int i = -1;

				bool res = avx_triangle_intersect(m_avxTriangleData[curNode->getAvxIndex()], rayData, t, u, v, i);

				if (res && t < tmin) {
					return true;
				}

			}
			else {

				unsigned numChildren = curNode->getNumChildren();

				__m128 t0, t0s, t0e, result;
				intersectData.setupBoxIntersection(
					curNode->m_minx,
					curNode->m_miny,
					curNode->m_minz,
					curNode->m_maxx,
					curNode->m_maxy,
					curNode->m_maxz
					);
				int res = intersect4AABB1ray(
					intersectData
					);

				if (res) {

					for (int i = 0; i < numChildren; ++i) {

						if ((res & (1 << i))) {
							st[stSize++] = curNode->m_left + i;

						}

					}

				}



			}

		}
		return false;
	}

	static struct QBVH_IntersectEntry {

		FORCEINLINE QBVH_IntersectEntry() {};
		FORCEINLINE QBVH_IntersectEntry(unsigned qbvhNodeIndex, float t) :
			qbvhNodeIndex_(qbvhNodeIndex),
			t_(t) {};

		unsigned qbvhNodeIndex_;
		float t_;

	};

#define SORT_SWAP(stack, i1,i2) if (stack[(i1)].t_ > stack[(i2)].t_) std::swap(stack[(i1)], stack[(i2)]);

	bool QBVH::intersect(const Vec3f & orig, const Vec3f & dir, float & tmin, float & umin, float & vmin, int & idx) const {

		QBVH_IntersectEntry st[256];
		unsigned stSize = 1;
		st[0] = QBVH_IntersectEntry(0, 0);

		AVX_RayInput rayData(orig, dir, tmin);
		SSE_RayInput sseRayData(orig, dir, tmin);

		SSE_4AABB_1RAY intersectData(sseRayData.ox, sseRayData.oy, sseRayData.oz, sseRayData.invdx, sseRayData.invdy, sseRayData.invdz, sseRayData.tmin);

		while (stSize != 0) {

			unsigned top = st[--stSize].qbvhNodeIndex_;
			float ct = st[stSize].t_;


			if (ct <= tmin) {

				const QBVH_Node * curNode = &m_nodes[top];

				if (curNode->isLeaf()) {

					float t = tmin, u, v;
					int i = -1;

					bool res = avx_triangle_intersect(m_avxTriangleData[curNode->getAvxIndex()], rayData, t, u, v, i);

					if (res && t <= tmin) {
						idx = m_indices[i + curNode->m_segmentStart];
						tmin = t;
						umin = u;
						vmin = v;
						rayData.tmin = _mm256_set1_ps(tmin);
						intersectData.tmin = _mm_set1_ps(tmin);
					}

				}
				else {

					unsigned numChildren = curNode->getNumChildren();

					// before doing the intersection tests, preload memory
					for (unsigned i = 0; i < numChildren; ++i) {
						unsigned childIndex = curNode->m_left + i;
						_mm_prefetch((char*)&m_nodes[childIndex], _MM_HINT_T0);
						_mm_prefetch((char*)&m_nodes[childIndex] + 32, _MM_HINT_T0);
						_mm_prefetch((char*)&m_nodes[childIndex] + 64, _MM_HINT_T0);
						_mm_prefetch((char*)&m_nodes[childIndex] + 96, _MM_HINT_T0);
					}
					
					intersectData.setupBoxIntersection(
						curNode->m_minx,
						curNode->m_miny,
						curNode->m_minz,
						curNode->m_maxx,
						curNode->m_maxy,
						curNode->m_maxz
						);


					int res = intersect4AABB1ray(
						intersectData
						);

					if (res) {
						int found = 0;
						for (unsigned i = 0; i < numChildren; ++i) {

							if ((res & (1 << i))) {
								assert(intersectData.tbegin.m128_f32[i] <= intersectData.tend.m128_f32[i]);
								st[stSize++] = QBVH_IntersectEntry(curNode->m_left + i, intersectData.t.m128_f32[i]);
								++found;
							}

						}

					}



				}
			}


		}

		return idx != -1;
	}

};