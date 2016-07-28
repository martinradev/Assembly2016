#pragma once

#include "AABB.h"
#include "RTTriangle.hpp"
#include "MemoryAllignedAllocator.h"
#include <vector>

namespace FW {

	/*
		keeps information for each triangle
	*/
	struct Reference {

		Reference() {};
		Reference(const SSE_AABB & box, unsigned trigIndex) :
			m_aabb(box),
			m_triangleIndex(trigIndex) {}

		SSE_AABB m_aabb;

		unsigned m_triangleIndex;

	};

	/*
		Bin for binned sah and spatial cost computation
	*/
	struct Bin {

		Bin()
			:
			m_aabb(Vec3f(std::numeric_limits<float>::max()), Vec3f(std::numeric_limits<float>::lowest())),
			m_accumulated(m_aabb),
			m_start(0),
			m_end(0)
		{

		}

		// bbox of bin
		SSE_AABB m_aabb;

		// accumulated bbox of bin left/right
		SSE_AABB m_accumulated;

		// references starting here
		int m_start;

		// references ending here
		int m_end;

	};


	/*
		Node which keeps reference list
	*/
	class SBVHNode {

		friend class RayTracer;
		friend class SBVH;
		friend class GPUBvh;
		friend class BVH;

	public:

		SBVHNode() {};

		/*
			we move the reference indices since they are no longer needed by the parent
		*/
		SBVHNode(const std::vector<unsigned> && indices, const SSE_AABB & box, unsigned left) :
			m_referenceIndices(indices),
			m_aabb(box),
			m_left(left)
		{}

		FORCEINLINE unsigned getReferenceIndex(const int pos) const {
			return m_referenceIndices[pos];
		}

		FORCEINLINE size_t getReferencesSize() const {
			return m_referenceIndices.size();
		}

		FORCEINLINE void addReferenceIndex(const unsigned index) {
			m_referenceIndices.push_back(index);
		}

		// no longer needed, so deallocate
		FORCEINLINE void deallocateIndices() {
			m_referenceIndices.clear();
		}

		FORCEINLINE bool isLeaf() const {
			return (m_left & (1u << 31));
		}

		FORCEINLINE unsigned getLeft() const {
			return m_left & 0x7FFFFFFF;
		}

		FORCEINLINE unsigned getRight() const {
			return getLeft() + 1;
		}

		FORCEINLINE void setLeft(int index) {
			m_left &= 0x7FFFFFFF; // remove leaf index
			m_left |= index;
		}

		FORCEINLINE unsigned getFlatBVHLeft() const {
			return m_left;
		}

	private:
		SSE_AABB m_aabb;
		std::vector<unsigned> m_referenceIndices;

		// left child
		unsigned m_left;


	};

	class SBVH {

		friend class RayTracer;
		friend class GPUBvh;
		friend class BVH;

	public:

		/*
			triangles ia vector of the triangle representing the scene
			numBins is the number of bins which will be used for binned sah and spatial cost computation
			maxTriangles is the maximum triangles per list.
			should be between 4 and 8 if you are planning to convert it to a standard BVH
		*/
		SBVH(const std::vector<RTTriangle> * triangles, int numBins = 16, int maxTriangles = 4);

	private:

		/*
			list of references, normally there should be at most 1.5 more references than triangles
		*/
		std::vector<Reference, aligned_allocator<Reference, 16> > m_references;

		// list of nodes
		std::vector<SBVHNode, aligned_allocator<SBVHNode, 16> > m_nodes;

		// triangle list
		const std::vector<RTTriangle> * m_triangles;

		// number of nodes in the tree
		size_t m_numNodes;

		// number of indices in the tree
		size_t m_numIndices;
		
		// number of leaves in the tree
		size_t m_numLeaves;

		// number of bins
		const int m_numBins;

		// maximum # triangles per leaf
		const int m_maxTriangles;

		// construct the sbvh
		void construct();

		/*
			finds a POTENTIAL object split using binned sah, but it does not do the split
			nodeIndex is the node which we want to split
			cost will contain the cost of the plist
			leftBB and rightBB will have the AABBs of the potential left and right child nodes
			splitBin will be the index of the bin which was used for splitting
			component is the dimension (xyz) along which the split was done
		*/
		FORCEINLINE void findObjectSplit(unsigned nodeIndex, float & cost, SSE_AABB & leftBB, SSE_AABB & rightBB, int & splitbin, int & component);
		
		/*
		finds a POTENTIAL spatial split, but it does not do the split
		nodeIndex is the node which we want to split
		cost will contain the cost of the plist
		leftCount and rightCount are the number of references in the left and right child nodes
		Note that leftCount+rightCount >= count(parent node)
		leftBB and rightBB will have the AABBs of the potential left and right child nodes
		component is the dimension (xyz) along which the split was done
		splitPlane value of the component along which the split was done - same idea as the bin index from the findObjectSplit method
		*/
		FORCEINLINE void findSpatialSplit(unsigned nodeIndex, float & cost, int & leftCount, int & rightCount, SSE_AABB & leftBB, SSE_AABB & rightBB, int & component, float & splitPlane);

		/*
			actually does the binned sah split
			nodeIndex is the index of the node which we are splitting
			splitBin is the bin which is the middle point
			component is the x|y|z dimension along which we are doing the split
			leftList and rightList are the reference lists for the left and right children
		*/
		FORCEINLINE void objectSort(unsigned nodeIndex, int splitBin, int component, std::vector<unsigned> & leftList, std::vector<unsigned> & rightList);
		
		/*
		actually does the binned sah split
		nodeIndex is the index of the node which we are splitting
		splitPlane is the value of the plane along which the split happens (same idea as with the bin split)
		component is the x|y|z dimension along which we are doing the split
		splitCost is the cost of the split - it is used to improve a bit on memory usage/bvh quality
		lcnt and rcnt is the count of the references in the left and right children nodes
		leftBB and rightBB will have the AABBs of the left and right child nodes
		leftList and rightList are the reference lists for the left and right children
		*/
		FORCEINLINE void spatialSort(unsigned nodeIndex, float splitPlane, int component, float splitCost, int lcnt, int rcnt, SSE_AABB & leftBB, SSE_AABB & rightBB, std::vector<unsigned> & leftList, std::vector<unsigned> & rightList);

		/*
			evaluates the spatial split cost
		*/
		FORCEINLINE float evalPreSplit(float leftBoxArea, int numLeft, float rightBoxArea, int numRight) const {
			return leftBoxArea * numLeft + rightBoxArea * numRight;
		}

		/*
			evaluates the full cost (sah cost)
		*/
		FORCEINLINE float evalFullSplit(float parentBoxArea, float preCost) const {
			return 1.0f + preCost / parentBoxArea;
		}

	};

};