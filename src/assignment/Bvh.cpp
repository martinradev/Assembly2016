#include "Bvh.hpp"
#include "IntersectionTest.h"
#include "SplitMethod.h"
#include "SBVH.h"
#include "filesaves.hpp"

namespace FW {

	static const int TRIANGLE_COUNT = 8;

	/* START CONSTRUCTION METHODS */

	static FORCEINLINE AABB computeBBox(const int from, const int to, const std::vector<RTTriangle> * triangles, const std::vector<int> & indices) {
		Vec3f minp = triangles->operator[](indices[from]).min();
		Vec3f maxp = triangles->operator[](indices[from]).max();
		for (int i = from + 1; i < to; ++i) {
			const int k = indices[i];
			const RTTriangle & trig = triangles->operator[](k);
			minp = FW::min(minp, trig.min());
			maxp = FW::max(maxp, trig.max());
		}
		return AABB(minp, maxp);
	}

	static FORCEINLINE SSE_TriangleIntersectionInput generateSSETrIntersectionInput(const int from, const int to, const std::vector<RTTriangle> * triangles, const std::vector<int> & indices) {

		SSE_TriangleIntersectionInput data;

		float _MM_ALIGN16 datax1[4];
		float _MM_ALIGN16 datax2[4];
		float _MM_ALIGN16 datax3[4];

		float _MM_ALIGN16 datay1[4];
		float _MM_ALIGN16 datay2[4];
		float _MM_ALIGN16 datay3[4];

		float _MM_ALIGN16 dataz1[4];
		float _MM_ALIGN16 dataz2[4];
		float _MM_ALIGN16 dataz3[4];

		float _MM_ALIGN16 nx[4];
		float _MM_ALIGN16 ny[4];
		float _MM_ALIGN16 nz[4];

		const int len = to - from;

		for (int i = 0; i < 4; ++i) {
			const int k = (i >= len ? 0 : i) + from;
			datax1[i] = (*triangles)[indices[k]].m_data.M(0, 0);
			datax2[i] = (*triangles)[indices[k]].m_data.M(1, 0);
			datax3[i] = (*triangles)[indices[k]].m_data.M(2, 0);

			datay1[i] = (*triangles)[indices[k]].m_data.M(0, 1);
			datay2[i] = (*triangles)[indices[k]].m_data.M(1, 1);
			datay3[i] = (*triangles)[indices[k]].m_data.M(2, 1);

			dataz1[i] = (*triangles)[indices[k]].m_data.M(0, 2);
			dataz2[i] = (*triangles)[indices[k]].m_data.M(1, 2);
			dataz3[i] = (*triangles)[indices[k]].m_data.M(2, 2);

			nx[i] = (*triangles)[indices[k]].m_data.N[0];
			ny[i] = (*triangles)[indices[k]].m_data.N[1];
			nz[i] = (*triangles)[indices[k]].m_data.N[2];

		}

		data.datax1 = _mm_load_ps(datax1);
		data.datax2 = _mm_load_ps(datax2);
		data.datax3 = _mm_load_ps(datax3);

		data.datay1 = _mm_load_ps(datay1);
		data.datay2 = _mm_load_ps(datay2);
		data.datay3 = _mm_load_ps(datay3);

		data.dataz1 = _mm_load_ps(dataz1);
		data.dataz2 = _mm_load_ps(dataz2);
		data.dataz3 = _mm_load_ps(dataz3);

		data.nx = _mm_load_ps(nx);
		data.ny = _mm_load_ps(ny);
		data.nz = _mm_load_ps(nz);

		return data;
	}

	static FORCEINLINE AVX_TriangleIntersectionInput generateAVXTrIntersectionInput(const int from, const int to, const std::vector<RTTriangle> * triangles, const std::vector<int> & indices) {

		AVX_TriangleIntersectionInput data;

		float _MM_ALIGN16 datax1[8];
		float _MM_ALIGN16 datax2[8];
		float _MM_ALIGN16 datax3[8];

		float _MM_ALIGN16 datay1[8];
		float _MM_ALIGN16 datay2[8];
		float _MM_ALIGN16 datay3[8];

		float _MM_ALIGN16 dataz1[8];
		float _MM_ALIGN16 dataz2[8];
		float _MM_ALIGN16 dataz3[8];

		float _MM_ALIGN16 nx[8];
		float _MM_ALIGN16 ny[8];
		float _MM_ALIGN16 nz[8];

		const int len = to - from;

		for (int i = 0; i < 8; ++i) {
			const int k = (i >= len ? 0 : i) + from;
			datax1[i] = (*triangles)[indices[k]].m_data.M(0, 0);
			datax2[i] = (*triangles)[indices[k]].m_data.M(1, 0);
			datax3[i] = (*triangles)[indices[k]].m_data.M(2, 0);

			datay1[i] = (*triangles)[indices[k]].m_data.M(0, 1);
			datay2[i] = (*triangles)[indices[k]].m_data.M(1, 1);
			datay3[i] = (*triangles)[indices[k]].m_data.M(2, 1);

			dataz1[i] = (*triangles)[indices[k]].m_data.M(0, 2);
			dataz2[i] = (*triangles)[indices[k]].m_data.M(1, 2);
			dataz3[i] = (*triangles)[indices[k]].m_data.M(2, 2);

			nx[i] = (*triangles)[indices[k]].m_data.N[0];
			ny[i] = (*triangles)[indices[k]].m_data.N[1];
			nz[i] = (*triangles)[indices[k]].m_data.N[2];

		}

		data.datax1 = _mm256_load_ps(datax1);
		data.datax2 = _mm256_load_ps(datax2);
		data.datax3 = _mm256_load_ps(datax3);

		data.datay1 = _mm256_load_ps(datay1);
		data.datay2 = _mm256_load_ps(datay2);
		data.datay3 = _mm256_load_ps(datay3);

		data.dataz1 = _mm256_load_ps(dataz1);
		data.dataz2 = _mm256_load_ps(dataz2);
		data.dataz3 = _mm256_load_ps(dataz3);

		data.nx = _mm256_load_ps(nx);
		data.ny = _mm256_load_ps(ny);
		data.nz = _mm256_load_ps(nz);

		return data;
	}

	struct BVHConstructionStackEntry {
		BVHConstructionStackEntry() {};
		BVHConstructionStackEntry(unsigned index, int segmentEnd) :
			m_index(index),
			m_segmentEnd(segmentEnd) {}
		unsigned m_index;
		int m_segmentEnd;
	};

	BVH::BVH(std::vector<RTTriangle> * triangles, const BVH_BUILD_METHOD splitMethod) :
		m_triangles(triangles),
		m_buildMethod(splitMethod) {

		
		if (m_buildMethod == BVH_BUILD_METHOD::LINEAR) {
			// build the bvh based on z curve sorting
			buildLBVH();
		}
		else if (m_buildMethod == BVH_BUILD_METHOD::SBVH) {
			// builds the sbvh and converts it to this structure
			buildFromSBVH();
		}
		else {
			// builds the bvh based on the other splitting methods
			buildStandardBVH();
		}
		

	}

	void BVH::buildFromSBVH() {

		SBVH sbvh(m_triangles, 16, TRIANGLE_COUNT);

		m_indices.resize(sbvh.m_numIndices);
		m_nodes.resize(sbvh.m_numNodes);
		m_avxTriangleData.resize(sbvh.m_numLeaves);

		unsigned st[128];
		int stSize = 1;
		st[0] = 0;

		int cIdx = 0;
		int avxIndex = 0;
		while (stSize != 0) {

			int top = st[--stSize];
			const auto & node = sbvh.m_nodes[top];
			auto & bvhNode = m_nodes[top];

			bvhNode.m_aabb = node.m_aabb.toAABB();
			bvhNode.m_left = node.getFlatBVHLeft();

			if (node.isLeaf()) {
				// if leaf, copy the triangle indices

				bvhNode.m_segmentStart = cIdx;

				for (int i = 0; i < node.getReferencesSize(); ++i) {
					m_indices[cIdx++] = sbvh.m_references[node.m_referenceIndices[i]].m_triangleIndex;
				}

				bvhNode.setAvxIndex(avxIndex);
				m_avxTriangleData[avxIndex] = generateAVXTrIntersectionInput(bvhNode.m_segmentStart, cIdx, m_triangles, m_indices);
				++avxIndex;
			}
			else {


				st[stSize++] = node.getRight();
				st[stSize++] = node.getLeft();

			}


		}
	}

	void BVH::buildStandardBVH() {

		// allocate enough memory
		m_avxTriangleData.resize(m_triangles->size());
		m_indices.resize(m_triangles->size());
		// do 1 to 1 mapping with the triangles initially
		for (int i = 0; i < m_indices.size(); ++i) m_indices[i] = i;

		// 2 * #triangles should be enough
		m_nodes.resize(m_indices.size() * 2);
		m_nodes[0] = BVHNode(computeBBox(0, m_indices.size(), m_triangles, m_indices), 1u << 31, 0);

		// stack and other book-keeping variables
		BVHConstructionStackEntry st[128];
		int stSize = 0;

		int numNodes = 1, numBadSplits = 0, numSplits = 0;
		unsigned numLeaves = 0;

		unsigned curNodeIndex = 0;
		int segmentEnd = m_indices.size();

		while (true) {

			// get the head and discard
			BVHNode & curNode = m_nodes[curNodeIndex];

			int segmentStart = curNode.m_segmentStart;
			if ((segmentEnd - segmentStart) <= TRIANGLE_COUNT) {
				// we have reached minimum number of triangles to create a leaf

				// set avx index in the leaf and generate avx intersection block
				curNode.setAvxIndex(numLeaves);
				m_avxTriangleData[numLeaves++] = generateAVXTrIntersectionInput(segmentStart, segmentEnd, m_triangles, m_indices);

				// check whether the stack is empty and break
				if (stSize == 0) break;

				// pop from stack
				curNodeIndex = st[--stSize].m_index;
				segmentEnd = st[stSize].m_segmentEnd;

				continue;
			}

			// process the node
			// subdivide

			AABB leftBB(curNode.m_aabb.max, curNode.m_aabb.min), rightBB(curNode.m_aabb.max, curNode.m_aabb.min);

			// do a split based on criterion
			int mid;
			float dummyValue; // we do not care for the cost here as long as it is minimum
			switch (m_buildMethod) {
			case BVH_BUILD_METHOD::OBJECT_MEDIAN:
				SplitMethod::objectMedianSplit(m_indices, m_triangles, segmentStart, segmentEnd, curNode, leftBB, rightBB, mid);
				break;
			case BVH_BUILD_METHOD::BINNED_SAH:
				SplitMethod::sahBinnedSplit(12, m_indices, m_triangles, segmentStart, segmentEnd, curNode, leftBB, rightBB, mid, dummyValue);
				break;
			default:
				mid = -1;
				std::cerr << "METHOD NOT SUPPORTED" << std::endl;
			}

			// just in case check for an invalid split and report error
			if (mid < 0) {
				// the split selection has decided that there is no reason to continue splitting
				std::cerr << "ERROR: Split was not chosen." << std::endl;
				continue;
			}
			++numSplits;

			// a bad split has occurred for some reason -> record it
			if (mid < segmentStart || mid >= segmentEnd) {
				mid = (segmentStart + segmentEnd) >> 1;
				leftBB = computeBBox(segmentStart, mid + 1, m_triangles, m_indices);
				rightBB = computeBBox(mid + 1, segmentEnd, m_triangles, m_indices);
				++numBadSplits;
			}

			

			// elements are reordered
			int leftIndex = numNodes++;
			int rightIndex = numNodes++;


			// generate left and right nodes
			m_nodes[leftIndex] = BVHNode(leftBB, 1u << 31, segmentStart);
			m_nodes[rightIndex] = BVHNode(rightBB, 1u << 31, mid + 1);

			curNode.m_left = leftIndex;

			st[stSize].m_index = rightIndex;
			st[stSize++].m_segmentEnd = segmentEnd;

			curNodeIndex = leftIndex;
			segmentEnd = mid + 1;

		}
	}

	void BVH::buildLBVH() {

		// allocated needed buffers
		m_avxTriangleData.resize(m_triangles->size());
		m_indices.resize(m_triangles->size());
		m_nodes.resize(m_indices.size() * 2);
		m_nodes[0] = BVHNode(AABB(), 1u << 31, 0);

		// morton pairs buffer
		std::vector<MortonCodesHelper::MortonPair> mortonPairs;
		mortonPairs.resize(m_triangles->size());

		// sort the triangles along the z curve
		MortonCodesHelper::generateMortonPairs(mortonPairs, m_triangles);

		// map index array
		for (size_t i = 0; i < mortonPairs.size(); ++i) m_indices[i] = mortonPairs[i].second;

		// allocate stack and book-keeping variables
		BVHConstructionStackEntry st[128];
		int stSize = 0;

		int numNodes = 1, numBadSplits = 0, numSplits = 0;
		unsigned numLeaves = 0;

		unsigned curNodeIndex = 0;
		int segmentEnd = m_indices.size();

		while (true) {

			// get the head and discard
			BVHNode & curNode = m_nodes[curNodeIndex];

			int segmentStart = curNode.m_segmentStart;
			if ((segmentEnd - segmentStart) <= TRIANGLE_COUNT) {
				// we have enough triangles

				curNode.setAvxIndex(numLeaves);
				m_avxTriangleData[numLeaves++] = generateAVXTrIntersectionInput(segmentStart, segmentEnd, m_triangles, m_indices);

				if (stSize == 0) break;
				curNodeIndex = st[--stSize].m_index;
				segmentEnd = st[stSize].m_segmentEnd;

				continue;
			}

			// process the node
			// subdivide
			AABB leftBB(curNode.m_aabb.max, curNode.m_aabb.min), rightBB(curNode.m_aabb.max, curNode.m_aabb.min);

			// find the position with highest plane split
			int mid = MortonCodesHelper::findSplit(mortonPairs, curNode.m_segmentStart, segmentEnd);

			if (mid < 0) {
				// the split selection has decided that there is no reason to continue splitting
				continue;
			}

			if (mid < segmentStart || mid >= segmentEnd) {
				// select mid since split is bad
				mid = (segmentStart + segmentEnd) >> 1;
				++numBadSplits;
			}
			++numSplits;



			// elements are reordered
			int leftIndex = numNodes++;
			int rightIndex = numNodes++;

			m_nodes[leftIndex] = BVHNode(leftBB, 1u << 31, segmentStart);
			m_nodes[rightIndex] = BVHNode(rightBB, 1u << 31, mid + 1);
			curNode.m_left = leftIndex;

			st[stSize].m_index = rightIndex;
			st[stSize++].m_segmentEnd = segmentEnd;

			curNodeIndex = leftIndex;
			segmentEnd = mid + 1;

		}

		// during the split, we do not build the AABBs since it would be more costly
		// it is faster to build them from the bottom up!
		buildAABBBottomUp(0, m_indices.size());


		std::cout << numNodes << " nodes out of " << m_nodes.size() << " allocated" << std::endl;
		std::cout << numBadSplits << " bad splits out of " << numSplits << std::endl;
	}

	void BVH::buildAABBBottomUp(unsigned index, unsigned segmentEnd) {
		BVHNode & node = m_nodes[index];
		if (node.isLeaf()) {
			// it is a leaf node
			node.m_aabb = computeBBox(node.m_segmentStart, segmentEnd, m_triangles, m_indices);
			return;
		}

		// otherwise compute for both children
		unsigned lIndex = node.getLeftChild();
		unsigned rIndex = node.getRightChild();
		buildAABBBottomUp(lIndex, m_nodes[rIndex].m_segmentStart);
		buildAABBBottomUp(rIndex, segmentEnd);

		node.m_aabb = m_nodes[lIndex].m_aabb;
		node.m_aabb.extend(m_nodes[rIndex].m_aabb);
	}

	void BVH::buildAVXTriangleData() {
		// allocate enough memory
		m_avxTriangleData.resize(m_indices.size());


		BVHConstructionStackEntry st[128];
		int stSize = 0;

		BVHConstructionStackEntry top(0, m_indices.size());

		while (true) {

			BVHNode & curNode = m_nodes[top.m_index];

			if (curNode.isLeaf()) {

				unsigned arrIndex = curNode.getAvxIndex();
				m_avxTriangleData[arrIndex] =
					generateAVXTrIntersectionInput(curNode.m_segmentStart, top.m_segmentEnd, m_triangles, m_indices);
			}
			else {

				st[stSize].m_index = curNode.getRightChild();
				st[stSize++].m_segmentEnd = top.m_segmentEnd;

				top.m_index = curNode.getLeftChild();
				top.m_segmentEnd = m_nodes[curNode.getRightChild()].m_segmentStart;


				continue;
			}

			if (stSize == 0) break;
			top = st[--stSize];


		}

	}

	/*
		END CONSTRUCTION METHODS
	*/

	/*
		START LOAD/SAVE
	*/
	BVH::BVH(std::vector<RTTriangle> * triangles, std::ifstream & input) :
		m_triangles(triangles)
	{
		size_t cnt;
		int tmp;

		// read indices
		fileload(input, cnt);
		m_indices.resize(cnt);
		for (size_t i = 0; i < cnt; ++i) {
			fileload(input, tmp);
			m_indices[i] = tmp;
		}
		// read nodes
		fileload(input, cnt);
		m_nodes.resize(cnt);
		for (size_t i = 0; i < m_nodes.size(); ++i) {
			auto & node = m_nodes[i];


			fileload(input, node.m_left);
			fileload(input, node.m_segmentStart);


			// also load bb
			fileload(input, node.m_aabb.min[0]);
			fileload(input, node.m_aabb.min[1]);
			fileload(input, node.m_aabb.min[2]);
			fileload(input, node.m_aabb.max[0]);
			fileload(input, node.m_aabb.max[1]);
			fileload(input, node.m_aabb.max[2]);
		}

		buildAVXTriangleData();
	}

	void BVH::saveToStream(std::ofstream & output) const {

		filesave(output, m_indices.size());
		for (size_t i = 0; i < m_indices.size(); ++i) {
			filesave(output, m_indices[i]);

		}

		// save nodes
		filesave(output, m_nodes.size());
		for (size_t i = 0; i < m_nodes.size(); ++i) {
			const auto & node = m_nodes[i];

			filesave(output, node.m_left);
			filesave(output, node.m_segmentStart);

			// also save bb
			filesave(output, node.m_aabb.min[0]);
			filesave(output, node.m_aabb.min[1]);
			filesave(output, node.m_aabb.min[2]);
			filesave(output, node.m_aabb.max[0]);
			filesave(output, node.m_aabb.max[1]);
			filesave(output, node.m_aabb.max[2]);
		}

	}
	/*
		END LOAD/SAVE
	*/

	/* START TRAVERSING METHODS */

	// 
	struct BVHTraverseEntry {

		BVHTraverseEntry() {};
		BVHTraverseEntry(unsigned entry, float t, float t0, float t1) :
			entry_(entry),
			t_(t),
			t0_(t0),
			t1_(t1) {}
		unsigned entry_;
		float t_;
		float t0_;
		float t1_;

	};

	bool BVH::intersect(const Vec3f & orig, const Vec3f & dir, float & tmin, float & umin, float & vmin, int & idx) const {

		BVHTraverseEntry st[128];
		int stSize = 0;
		const BVHNode* curNode = &m_nodes[0];
		float ct = 0;
		float cts = 0, cte = 0;

		FW::Vec3f invDir = Vec3f(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);
		int sign[3] = { invDir[0] < 0.0f, invDir[1] < 0.0f, invDir[2] < 0.0f };

		if (!intersect_aabb(orig.getPtr(), sign, invDir.getPtr(), curNode->m_aabb.min.getPtr(), tmin, cts, cte)) return false;


		AVX_RayInput rayData(orig, dir, tmin);


		while (true) {


			const auto & bbox = curNode->m_aabb;

			if (ct <= tmin) {
				if (curNode->isLeaf()) {
					// leaf

					float t = tmin, u, v;
					int i = -1;

					bool res = avx_triangle_intersect(m_avxTriangleData[curNode->getAvxIndex()], rayData, t, u, v, i);


					if (res && t < tmin) {
						idx = m_indices[i + curNode->m_segmentStart];
						tmin = t;
						umin = u;
						vmin = v;
						rayData.tmin = _mm256_set1_ps(tmin);
					}



				}
				else {
					// continue with left and right child
					float t0, t1;
					float t0s, t0e, t1s, t1e;
					int left = curNode->getLeftChild();
					int right = curNode->getRightChild();
					const BVHNode * leftNode = &m_nodes[left];
					const BVHNode * rightNode = &m_nodes[right];

					bool r0 = intersect_aabb(orig.getPtr(), sign, invDir.getPtr(), rightNode->m_aabb.min.getPtr(), tmin, t0s, t0e);
					bool r1 = intersect_aabb(orig.getPtr(), sign, invDir.getPtr(), leftNode->m_aabb.min.getPtr(), tmin, t1s, t1e);

					t0 = t0s >= 0.0f ? t0s : 0.0f;
					if (t0s > tmin) r0 = false, t0 = std::numeric_limits<float>::max();
					t1 = t1s >= 0.0f ? t1s : 0.0f;
					if (t1s > tmin) r1 = false, t1 = std::numeric_limits<float>::max();
					if (t1 < t0) {
						std::swap(rightNode, leftNode);
						std::swap(t0, t1);
						std::swap(t0s, t1s);
						std::swap(t0e, t1e);
						std::swap(right, left);
						std::swap(r0, r1);
					}

					if (r1) {
						st[stSize++] = BVHTraverseEntry(left, t1, t1s, t1e);
					}

					if (r0) {
						curNode = rightNode;
						ct = t0;
						cts = t0s;
						cte = t0e;
						continue;
					}


				}

			}

			if (stSize == 0) break;
			const auto tmp = st[--stSize];
			curNode = &m_nodes[tmp.entry_];
			ct = tmp.t_;
			cts = tmp.t0_;
			cte = tmp.t1_;
		}
		return idx != -1;
	}

	bool BVH::isOccluded(const Vec3f & orig, const Vec3f & dir, float tmin) const {
		unsigned st[128];
		int stSize = 0;
		const BVHNode * curNode = &m_nodes[0];
		float cts, cte;
		FW::Vec3f invDir = Vec3f(1.0f / dir.x, 1.0f / dir.y, 1.0f / dir.z);
		int sign[3] = { invDir[0] < 0, invDir[1] < 0, invDir[2] < 0 };

		if (!intersect_aabb(orig.getPtr(), sign, invDir.getPtr(), curNode->m_aabb.min.getPtr(), tmin, cts, cte)) return false;

		AVX_RayInput rayData(orig, dir, tmin);

		while (true) {


			const auto & bbox = curNode->m_aabb;


			if (curNode->isLeaf()) {
				// leaf

				float t = tmin, u, v;
				int i = -1;


				bool res = avx_triangle_intersect(m_avxTriangleData[curNode->getAvxIndex()], rayData, t, u, v, i);

				if (res && t < tmin) {
					return true;
				}



			}
			else {
				// continue with left and right child
				float t0, t1;
				float t0s, t0e, t1s, t1e;
				int left = curNode->m_left;
				int right = left + 1;
				const BVHNode *leftNode = &m_nodes[left];
				const BVHNode * rightNode = &m_nodes[right];

				bool r0 = intersect_aabb(orig.getPtr(), sign, invDir.getPtr(), rightNode->m_aabb.min.getPtr(), tmin, t0s, t0e);
				bool r1 = intersect_aabb(orig.getPtr(), sign, invDir.getPtr(), leftNode->m_aabb.min.getPtr(), tmin, t1s, t1e);

				t0 = t0s >= 0.0f ? t0s : 0.0f;
				if (t0s > tmin) r0 = false, t0 = std::numeric_limits<float>::max();
				t1 = t1s >= 0.0f ? t1s : 0.0f;
				if (t1s > tmin) r1 = false, t1 = std::numeric_limits<float>::max();
				if (t1 < t0) {
					std::swap(rightNode, leftNode);
					std::swap(right, left);
					std::swap(r0, r1);
				}


				if (r1) {
					st[stSize++] = left;
				}

				if (r0) {
					curNode = rightNode;
					continue;
				}


			}



			if (stSize == 0) break;
			const auto tmp = st[--stSize];
			curNode = &m_nodes[tmp];
		}

		return false;
	}

	/* END TRAVERSING METHODS */

};