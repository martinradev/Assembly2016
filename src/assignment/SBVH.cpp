#include "SBVH.h"

#include "ParallelSort.h"
#include <cassert>

namespace FW {
	
	/*
		Special AABB accumulation for the SSE AABBs
	*/
	static SSE_AABB computeAABB(const std::vector<Reference, aligned_allocator<Reference, 16> > & references, const std::vector<unsigned> indices) {
		SSE_AABB box(references[indices[0]].m_aabb.min, references[indices[0]].m_aabb.max);

		for (size_t i = 1; i < indices.size(); ++i) box.extend(references[indices[i]].m_aabb);

		return box;
	}

	/*
		computes the overlapping area of the aab1 and aabb2
	*/
	FORCEINLINE bool aabbOverlap(const SSE_AABB & aabb1, const SSE_AABB & aabb2, float & overlap) {

		SSE_AABB delta = SSE_AABB(
			_mm_max_ps(aabb1.min, aabb2.min),
			_mm_min_ps(aabb1.max, aabb2.max)
			);

		if (delta.isValid()) {
			overlap = delta.area();
			return true;
		}
		return false;

	}

	SBVH::SBVH(const std::vector<RTTriangle> * triangles, int numBins, int maxTriangles) :
		m_triangles(triangles),
		m_numNodes(0),
		m_numIndices(0),
		m_numLeaves(0),
		m_numBins(numBins),
		m_maxTriangles(maxTriangles)
	{
		construct();
	};

	static struct ConstructEntry {

		ConstructEntry() {};
		ConstructEntry(unsigned nodeIndex) :
			nodeIndex_(nodeIndex) {}

		unsigned nodeIndex_;
	};

	void SBVH::construct() {

		int numNodes = 1;

		// allocate buffers and generate references
		m_references.reserve(2 * m_triangles->size());
		m_references.resize(m_triangles->size());
		for (size_t i = 0; i < m_triangles->size(); ++i) {
			const auto & trig = (*m_triangles)[i];
			m_references[i].m_triangleIndex = i;
			m_references[i].m_aabb = SSE_AABB(trig.min(), trig.max());
		}

		// preallocate nodes, maybe 3 times, more is not needed
		m_nodes.resize(m_references.size() * 3);
		ConstructEntry st[128];
		int stSize = 1;
		st[0] = ConstructEntry(0);

		ConstructEntry top;

		// temporary indices and 1 to 1 mapping
		std::vector<unsigned> allIndices(m_references.size());
		for (int i = 0; i < allIndices.size(); ++i) allIndices[i] = i;

		SSE_AABB rootBox = computeAABB(m_references, allIndices);
		float rootSurfaceArea = rootBox.area();
		// to be used to consider whether spatial should be computed, as in paper
		const float Alpha = 1e-5;
		m_nodes[0] = SBVHNode(std::move(allIndices), rootBox, 1u << 31);
		while (stSize) {

			top = st[--stSize];
			auto & node = m_nodes[top.nodeIndex_];

			if (node.getReferencesSize() <= m_maxTriangles) {
				// enough triangles so far
				m_numIndices += node.getReferencesSize();
				++m_numLeaves;
				continue;
			}


			// compute the potential object split cost
			float objCost;
			SSE_AABB objLeftBB, objRightBB;
			int sahBin, sahComponent;
			findObjectSplit(top.nodeIndex_, objCost, objLeftBB, objRightBB, sahBin, sahComponent);


			// check whether the object split produces overlapping nodes
			// if so, check whether we are close enough to the root so that the spatial split makes sense
			float overlapValue;
			bool computeSpatial;
			if (aabbOverlap(objLeftBB, objRightBB, overlapValue)) {
				computeSpatial = (overlapValue / rootSurfaceArea) >= Alpha;
			}
			else {
				computeSpatial = false;
			}

			// compute the spatial split if required (computeSpatial == true)
			float spatialCost;
			SSE_AABB spatialLeftBB, spatialRightBB;
			float spatialSplitPlane;
			int spatialDimension;
			int lcnt, rcnt;
			if (computeSpatial) {
				findSpatialSplit(top.nodeIndex_, spatialCost, lcnt, rcnt, spatialLeftBB, spatialRightBB, spatialDimension, spatialSplitPlane);
			}

			std::vector<unsigned> lList, rList;

			/*
				if we have compute the spatial cost and it is better than the binned sah cost,
				then do the split
			*/
			int usedDimension;
			if (computeSpatial && spatialCost <= objCost) {
				// use spatial split
				spatialSort(top.nodeIndex_, spatialSplitPlane, spatialDimension, spatialCost, lcnt, rcnt, spatialLeftBB, spatialRightBB, lList, rList);
				objLeftBB = spatialLeftBB;
				objRightBB = spatialRightBB;
				usedDimension = spatialDimension;
			}
			else {
				// use object split

				// check whether the binned sah has failed
				// if so, we have to do the object median split
				// it happens for some scenes, but it happens very close to the leaves
				// i.e. when we are left with 8-16 tightly packed references which end up in the same bin
				if (sahBin == -1) {
					Vec3f axisDelta = node.m_aabb.getAxisLength();

					int bestAxis = 0;
					if (axisDelta.y >= axisDelta.x && axisDelta.y >= axisDelta.z) {
						bestAxis = 1;
					}
					else if (axisDelta.z >= axisDelta.x && axisDelta.z >= axisDelta.y) {
						bestAxis = 2;
					}
					usedDimension = bestAxis;
					// sort fast
					parallelMergeSort(node.m_referenceIndices.begin(), node.m_referenceIndices.end(),
						[bestAxis, this](const unsigned i1, const unsigned i2) {

						return m_references[i1].m_aabb.getCenter().m128_f32[bestAxis] < m_references[i2].m_aabb.getCenter().m128_f32[bestAxis];

					});
					// distribute in left and right child evenly
					objLeftBB = SSE_AABB(Vec3f(std::numeric_limits<float>::max()), Vec3f(std::numeric_limits<float>::lowest()));
					objRightBB = objLeftBB;
					for (int i = 0; i < node.m_referenceIndices.size(); ++i) {
						if (i < node.m_referenceIndices.size() / 2) {
							lList.push_back(node.m_referenceIndices[i]);
							objLeftBB.extend(m_references[node.m_referenceIndices[i]].m_aabb);
						}
						else {
							rList.push_back(node.m_referenceIndices[i]);
							objRightBB.extend(m_references[node.m_referenceIndices[i]].m_aabb);
						}
					}

				}
				else {
					// we can do the binned sah split
					objectSort(top.nodeIndex_, sahBin, sahComponent, lList, rList);
					usedDimension = sahComponent;
				}

			}


			// push left and right

			int leftIndex = numNodes;
			int rightIndex = leftIndex + 1;
			// some sanity check
			assert(lList.size() + rList.size() >= node.getReferencesSize());
			// dont with this object, deallocate memory for the current node
			node.deallocateIndices();

			// copy node data to left and right children
			m_nodes[leftIndex] = SBVHNode(std::move(lList), objLeftBB, 1u << 31);
			m_nodes[rightIndex] = SBVHNode(std::move(rList), objRightBB, 1u << 31);
			node.setLeft(leftIndex);

			st[stSize++] = leftIndex;
			st[stSize++] = rightIndex;

			numNodes += 2;

		}

		m_numNodes = numNodes;
	}

	void SBVH::findObjectSplit(unsigned nodeIndex, float & cost, SSE_AABB & leftBB, SSE_AABB & rightBB, int & splitBin, int & component) {

		std::vector<Bin, aligned_allocator<Bin, 16>> bins(m_numBins);

		const auto & node = m_nodes[nodeIndex];

		// compute the aabb of all centroids
		SSE_AABB bbCentroid = SSE_AABB(node.m_aabb.max, node.m_aabb.min);

		const int len = node.m_referenceIndices.size();
		for (size_t i = 0; i < len; ++i) {
			const int ind = node.m_referenceIndices[i];
			const auto & ref = m_references[ind];
			__m128 centroid = ref.m_aabb.getCenter();
			bbCentroid.extend(centroid);
		}

		cost = std::numeric_limits<float>::max();
		splitBin = -1;
		component = -1;

		// for each dimension check the best splits
		for (int dim = 0; dim < 3; ++dim) {
			if ((bbCentroid.max.m128_f32[dim] - bbCentroid.min.m128_f32[dim]) == 0.0f) continue;
			const float bbLenInv = 1.0f / (bbCentroid.max.m128_f32[dim] - bbCentroid.min.m128_f32[dim]);

			// clear bins
			for (int i = 0; i < m_numBins; ++i) bins[i] = Bin();

			// distribute references in the bins based on the centroids
			for (int i = 0; i < node.m_referenceIndices.size(); ++i) {
				const int ind = node.m_referenceIndices[i];
				const auto & ref = m_references[ind];
				int bIndex = m_numBins * ((ref.m_aabb.getCenter().m128_f32[dim] - bbCentroid.min.m128_f32[dim]) * bbLenInv);
				if (bIndex == m_numBins) --bIndex;
				assert(bIndex >= 0 && bIndex < m_numBins);

				++bins[bIndex].m_start;
				bins[bIndex].m_aabb.extend(ref.m_aabb);
			}

			// backward accumulate using dp
			bins[m_numBins - 1].m_accumulated = bins[m_numBins - 1].m_aabb;
			bins[m_numBins - 1].m_end = bins[m_numBins - 1].m_start;
			for (int i = m_numBins - 2; i >= 0; --i) {
				bins[i].m_accumulated = bins[i + 1].m_accumulated;
				bins[i].m_accumulated.extend(bins[i].m_aabb);
				bins[i].m_end = bins[i].m_start + bins[i + 1].m_end;
			}

			// keep only one variable for forward accumulation
			SSE_AABB lbox = bins[0].m_aabb;

			// find split
			for (int i = 0; i < m_numBins - 1; ++i) {
				lbox.extend(bins[i].m_aabb);

				const float larea = lbox.area();
				const float rarea = bins[i + 1].m_accumulated.area();
				const int rcnt = bins[i + 1].m_end;
				const int lcnt = len - rcnt;
				if (lcnt == 0) continue;

				const float splitCost = evalPreSplit(larea, lcnt, rarea, rcnt);

				if (splitCost < cost) {

					// this split is good
					cost = splitCost;
					splitBin = i;
					component = dim;
					leftBB = lbox;
					rightBB = bins[i + 1].m_accumulated;

				}
			}

		}

	}

	void SBVH::objectSort(unsigned nodeIndex, int splitBin, int component, std::vector<unsigned> & leftList, std::vector<unsigned> & rightList) {
		std::vector<Bin, aligned_allocator<Bin, 16>> bins(m_numBins);

		const auto & node = m_nodes[nodeIndex];

		// compute aabb of all reference centroids
		SSE_AABB bbCentroid = SSE_AABB(node.m_aabb.max, node.m_aabb.min);

		const int len = node.m_referenceIndices.size();
		
		for (size_t i = 0; i < len; ++i) {
			const int ind = node.m_referenceIndices[i];
			const auto & ref = m_references[ind];
			__m128 centroid = ref.m_aabb.getCenter();
			bbCentroid.extend(centroid);
		}

		const float bbLenInv = 1.0f / (bbCentroid.max.m128_f32[component] - bbCentroid.min.m128_f32[component]);

		// distribute to left and right based on the provided split bin
		for (int i = 0; i < node.m_referenceIndices.size(); ++i) {
			const int ind = node.m_referenceIndices[i];
			const auto & ref = m_references[ind];
			// compute the bin index of the current reference and put to EITHER left OR right
			int bIndex = m_numBins * ((ref.m_aabb.getCenter().m128_f32[component] - bbCentroid.min.m128_f32[component]) * bbLenInv);
			if (bIndex == m_numBins) --bIndex;
			assert(bIndex >= 0 && bIndex < m_numBins);
			if (bIndex <= splitBin) {
				// goes left
				leftList.push_back(ind);
			}
			else {
				// goes right
				rightList.push_back(ind);
			}
		}

	}

	void SBVH::spatialSort(unsigned nodeIndex, float splitPlane, int component, float splitCost, int lcnt, int rcnt, SSE_AABB & leftBB, SSE_AABB & rightBB, std::vector<unsigned> & leftList, std::vector<unsigned> & rightList) {

		std::vector<Bin, aligned_allocator<Bin, 16>> bins(m_numBins);

		const auto & node = m_nodes[nodeIndex];
		float rightSurfaceArea = rightBB.area();
		float leftSurfaceArea = leftBB.area();

		/*
			distribute the refenreces to left, right or both children
		*/
		for (int i = 0; i < node.m_referenceIndices.size(); ++i) {

			const auto refIndex = node.m_referenceIndices[i];
			const auto & ref = m_references[refIndex];
			const auto refMin = ref.m_aabb.min.m128_f32[component];
			const auto refMax = ref.m_aabb.max.m128_f32[component];


			if (refMax <= splitPlane) {
				leftList.push_back(refIndex);
			}
			else if (refMin >= splitPlane) {
				rightList.push_back(refIndex);
			}
			else {

				// split the reference

				// check possible unsplit

				SSE_AABB leftUnsplitBB = leftBB;
				leftUnsplitBB.extend(ref.m_aabb);
				const float leftUnsplitBBArea = leftUnsplitBB.area();
				const float unsplitLeftCost = leftUnsplitBBArea * lcnt + (rcnt - 1) * rightSurfaceArea;

				// maybe we can undo the split and push to only one child
				SSE_AABB rightUnsplitBB = rightBB;
				rightUnsplitBB.extend(ref.m_aabb);
				const float rightUnsplitBBArea = rightUnsplitBB.area();
				const float unsplitRightCost = (lcnt - 1) * leftSurfaceArea + rcnt * rightUnsplitBBArea;
				if (unsplitLeftCost < splitCost && unsplitLeftCost <= unsplitRightCost) {
					// put only into left only
					leftList.push_back(refIndex);

					// update params
					leftSurfaceArea = leftUnsplitBBArea;
					leftBB = leftUnsplitBB;
					--rcnt;
					splitCost = unsplitLeftCost;
				}
				else if (unsplitRightCost <= unsplitLeftCost && unsplitRightCost < splitCost) {
					// put only into the right
					rightList.push_back(refIndex);
					// update params
					rightSurfaceArea = leftUnsplitBBArea;
					rightBB = rightUnsplitBB;
					--lcnt;
					splitCost = unsplitRightCost;
				}
				else {
					// push left and right
					Reference leftRef(m_references[refIndex]);
					if (leftRef.m_aabb.max.m128_f32[component] > splitPlane) leftRef.m_aabb.max.m128_f32[component] = splitPlane;

					Reference rightRef(m_references[refIndex]);
					if (rightRef.m_aabb.min.m128_f32[component] < splitPlane) rightRef.m_aabb.min.m128_f32[component] = splitPlane;

					m_references[refIndex] = leftRef;
					m_references.push_back(rightRef);

					leftList.push_back(refIndex);
					rightList.push_back(m_references.size() - 1);
				}

			}

		}

	}

	void SBVH::findSpatialSplit(unsigned nodeIndex, float & cost, int & leftCount, int & rightCount, SSE_AABB & leftBB, SSE_AABB & rightBB, int & bestComponent, float & splitPlane) {
		cost = std::numeric_limits<float>::max();
		splitPlane = -1;
		bestComponent = -1;

		std::vector<Bin, aligned_allocator<Bin, 16> > bins(m_numBins);
		static const float binsInv = 1.0f / m_numBins;
		const auto & node = m_nodes[nodeIndex];
		const auto & pBox = node.m_aabb;

		// check along each dimension
		for (int dim = 0; dim < 3; ++dim) {
			const float segmentLength = (pBox.max.m128_f32[dim] - pBox.min.m128_f32[dim]);
			if (segmentLength == 0.0f) continue;
			const float bbLenInv = 1.0f / segmentLength;

			// clear bins
			for (int i = 0; i < m_numBins; ++i) bins[i] = Bin();

			for (int i = 0; i < node.m_referenceIndices.size(); ++i) {

				const auto & ref = m_references[node.m_referenceIndices[i]];

				const auto refMin = ref.m_aabb.min.m128_f32[dim];
				const auto refMax = ref.m_aabb.max.m128_f32[dim];

				/*
					we split each triangle into references
					each triangle will be recorded into multiple bins
				*/
				const int boxStart = FW::clamp(
					int(m_numBins * ((refMin - pBox.min.m128_f32[dim]) * bbLenInv)),
					0, m_numBins - 1
					);
				const int boxEnd = FW::clamp(
					int(m_numBins * ((refMax - pBox.min.m128_f32[dim]) * bbLenInv)),
					0, m_numBins - 1
					);

				assert(boxStart <= boxEnd);

				for (int j = boxStart; j <= boxEnd; ++j) {
					const float bmin = pBox.min.m128_f32[dim] + (j * segmentLength) * binsInv;
					const float bmax = pBox.min.m128_f32[dim] + ((j + 1.0f) * segmentLength) * binsInv;
					assert(bmin <= bmax);

					bins[j].m_aabb.extend(ref.m_aabb);
					if (bins[j].m_aabb.min.m128_f32[dim] < bmin) bins[j].m_aabb.min.m128_f32[dim] = bmin;
					if (bins[j].m_aabb.max.m128_f32[dim] > bmax) bins[j].m_aabb.max.m128_f32[dim] = bmax;

				}

				++bins[boxStart].m_start;
				++bins[boxEnd].m_end;
			}

			// we have distributed the references already

			// augment the bins from right to left

			bins[m_numBins - 1].m_accumulated = bins[m_numBins - 1].m_aabb;

			for (int j = m_numBins - 2; j >= 0; --j) {
				bins[j].m_accumulated = bins[j + 1].m_accumulated;
				bins[j].m_accumulated.extend(bins[j].m_aabb);
				bins[j].m_end += bins[j + 1].m_end;
			}

			int lcnt = 0;
			SSE_AABB accumulateForward(bins[0].m_aabb.min, bins[0].m_aabb.max);

			// consider each split
			for (int j = 0; j < m_numBins - 1; ++j) {

				const int rcnt = bins[j + 1].m_end;
				lcnt += bins[j].m_start;
				accumulateForward.extend(bins[j].m_aabb);

				assert(accumulateForward.max.m128_f32[dim] <= bins[j + 1].m_accumulated.min.m128_f32[dim]);

				const float splitCost = evalPreSplit(accumulateForward.area(), lcnt, bins[j + 1].m_accumulated.area(), rcnt);

				if (splitCost < cost) {
					cost = splitCost;
					leftBB = accumulateForward;
					rightBB = bins[j + 1].m_accumulated;
					bestComponent = dim;
					splitPlane = bins[j + 1].m_accumulated.min.m128_f32[dim];
					leftCount = lcnt;
					rightCount = rcnt;
				}

			}

		}

	}
	
};