#pragma once

#include "BvhNode.hpp"
#include "RTTriangle.hpp"
#include "IntersectionTest.h"
#include "MemoryAllignedAllocator.h"
#include "MortonCodes.h"

#include <vector>
#include <fstream>

namespace FW {
	enum class BVH_BUILD_METHOD {

		OBJECT_MEDIAN = 0,
		BINNED_SAH,
		LINEAR,
		SBVH

	};

	class BVH {

		friend class QBVH;

	public:
		/*
			creates the BVH, generating the BVH using the given split method
		*/
		BVH(std::vector<RTTriangle> * triangles, const BVH_BUILD_METHOD splitMethod);
		
		/*
			loads the BVH from the input stream
		*/
		BVH(std::vector<RTTriangle> * triangles, std::ifstream & input);

		/*
			traverses the bvh searching for an intersection with a triangle
			returns true if a triangle intersection has been found
			orig is the origin of the ray
			dir is the direction of the ray. Dir does not have to be normalized
			The search for an intersection will be done in [orig, orig+dir]
			tmin will contain a t value for the closest intersection. Point of intersection is orig + tmin * dir
			umin and vmin are the barrycentric coefficients of the intersection
			idx is the index of the triangle in the m_triangles list. It is to be used later for Material querying, etc
		*/
		bool intersect(const Vec3f & orig, const Vec3f & dir, float & tmin, float & umin, float & vmin, int & idx) const;
		
		/*
			Returns true if any intersection has been found in [orig, orig+dir]
			Much faster than the intersect(...) method since only a single intersection is fine,
			no need to find the closest.
			Should be used for shadow rays!
		*/
		bool isOccluded(const Vec3f & orig, const Vec3f & dir, float tmin) const;

		void saveToStream(std::ofstream & output) const;

	private:
		// indices used to access the triangles array
		std::vector<int> m_indices;

		// all nodes allocated sequentially
		std::vector<BVHNode> m_nodes;

		// triangle array. Cannot be modified
		const std::vector<RTTriangle> * m_triangles;

		// contains triangle blocks of 4 for intersection tests
		std::vector<AVX_TriangleIntersectionInput, aligned_allocator<AVX_TriangleIntersectionInput, 16> > m_avxTriangleData;

		// build method used to construct the bvh
		BVH_BUILD_METHOD m_buildMethod;

		// generates linear bvh using morton codes
		void buildLBVH();

		// generates the sbvh and converts it to the current BVH structure
		void buildFromSBVH();

		// generates the bvh using the appropriate split method
		void buildStandardBVH();

		// generates the avx triangle blocks. It is to be used when loading the bvh from a file
		void buildAVXTriangleData();

		// build AABBs from the leaves to the top. It is to be used for the LBVH construction
		void buildAABBBottomUp(unsigned index, unsigned segmentEnd);

	};

};