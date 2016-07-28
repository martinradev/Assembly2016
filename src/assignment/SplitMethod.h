#pragma once

#include "RTTriangle.hpp"
#include "BvhNode.hpp"
#include <vector>

namespace FW {

	class SplitMethod {

	public:

		/*
			input:
			Does a split according to the object median
			indices contains the indices to the permuted triangles
			triangles is an array of triangles
			from and to are the span in the indices array we are working with
			node is the node we are going to split

			output:
			laabb and raabb are the left and right bounding boxes
			splitPosition is the position. the array partitioned as
			[from,splitPosition] [splitPosition+1,to)
		*/
		static void objectMedianSplit(
			std::vector<int> & indices,
			const std::vector<RTTriangle> * triangles,
			int from, int to,
			const BVHNode & node, 
			AABB & laabb,
			AABB & raabb,
			int & splitPosition);
		
		/*
		input:
		Does a split according to the SAH
		the span is partitioned into bins. No need for sorting,
		but there is a possibility for a lower quality bvh!
		indices contains the indices to the permuted triangles
		triangles is an array of triangles
		from and to are the span in the indices array we are working with
		node is the node we are going to split

		output:
		laabb and raabb are the left and right bounding boxes
		splitPosition is the position. the array partitioned as
		[from,splitPosition] [splitPosition+1,to)
		*/
		static void sahBinnedSplit(
			const int numBuckets,
			std::vector<int> & indices,
			const std::vector<RTTriangle> * triangles,
			int from, int to,
			const BVHNode & node,
			AABB & laabb, AABB & raabb,
			int & splitPosition,
			float & cost);
	};

};