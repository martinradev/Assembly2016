#include "SplitMethod.h"

#include <algorithm>
#include <cassert>

namespace FW {

	void SplitMethod::objectMedianSplit(
		std::vector<int> & indices,
		const std::vector<RTTriangle> * triangles,
		int from, int to,
		const BVHNode & node,
		AABB & laabb, AABB & raabb,
		int & splitPosition) {

		float dx = node.m_aabb.max[0] - node.m_aabb.min[0];
		float dy = node.m_aabb.max[1] - node.m_aabb.min[1];
		float dz = node.m_aabb.max[2] - node.m_aabb.min[2];

		int componentPos = 2;
		if (dx >= dy && dx >= dz) {
			componentPos = 0;
		}
		else if (dy >= dx && dy >= dz) {
			componentPos = 1;
		}

		int mid = (to + from - 1) >> 1;
		// find median and store it at position idx
		std::nth_element(indices.begin() + from, indices.begin() + mid, indices.begin() + to,

			[componentPos, triangles](int r1Index, int r2Index) {
			const auto & r1 = triangles->operator[](r1Index);
			const auto & r2 = triangles->operator[](r2Index);
			return (r1.maxComponent(componentPos) + r1.minComponent(componentPos)) <
				(r2.maxComponent(componentPos) + r2.minComponent(componentPos));
		}

		);

		for (int i = from; i < to; ++i) {
			const auto & trig = triangles->operator[](indices[i]);
			if (i <= mid) {
				laabb.extend(trig.min(), trig.max());
			}
			else {
				raabb.extend(trig.min(), trig.max());
			}
		}

		splitPosition = mid;
	}

	void SplitMethod::sahBinnedSplit(
		const int numBuckets,
		std::vector<int> & indices,
		const std::vector<RTTriangle> * triangles,
		int from, int to,
		const BVHNode & node,
		AABB & laabb, AABB & raabb,
		int & splitPosition,
		float & outCost) {

		typedef std::pair<int, AABB> BucketInfo;


		const int len = to - from;
		float bestCost = std::numeric_limits<float>::max();
		int bestBucket = -1;
		int bestComponent = -1;
		int bestSplit = -1;
		std::vector<BucketInfo> buckets(numBuckets);

		// compute bb of the centroids
		AABB centroidBB = AABB(node.m_aabb.max, node.m_aabb.min);

		for (int i = from; i < to; ++i) {
			const auto & trig = (*triangles)[indices[i]];
			const auto & trigCentroid = trig.centroid();
			centroidBB.extend(trigCentroid, trigCentroid);
		}

		std::vector<BucketInfo> backward(numBuckets);

		for (int componentPos = 0; componentPos < 3; ++componentPos) {
			if (centroidBB.max[componentPos] - centroidBB.min[componentPos] == 0.0f) continue;
			// initialize buckets
			for (int i = 0; i < numBuckets; ++i) {
				buckets[i] = std::make_pair(0, AABB(node.m_aabb.max, node.m_aabb.min));
			}

			// distribute the triangles in the buckets

			const float bbLenInv = 1.0f / (centroidBB.max[componentPos] - centroidBB.min[componentPos]);

			for (int i = from; i < to; ++i) {
				const auto & trig = (*triangles)[indices[i]];
				int idx = numBuckets * ((trig.centroid()[componentPos] - centroidBB.min[componentPos]) * bbLenInv);
				if (idx == numBuckets) --idx;
				assert(idx >= 0 && idx < numBuckets);
				++buckets[idx].first;
				buckets[idx].second.extend(trig.min(), trig.max());
			}

			// calculate cost for each split

			backward[numBuckets - 1] = buckets[numBuckets - 1];

			for (int i = 1; i < numBuckets; ++i) {
				backward[numBuckets - i - 1] = backward[numBuckets - i];
				backward[numBuckets - i - 1].second.extend(buckets[numBuckets - i - 1].second);
				backward[numBuckets - i - 1].first += buckets[numBuckets - i - 1].first;
			}


			AABB lbox = buckets[0].second;
			
			for (int i = 0; i < numBuckets - 1; ++i) {
				lbox.extend(buckets[i].second);
				const float larea = lbox.area();
				const float rarea = backward[i + 1].second.area();
				const int rcnt = backward[i + 1].first;
				const int lcnt = len - rcnt;
				if (lcnt == 0) continue;

				const float cost = (lcnt * larea + rcnt * rarea);

				if (cost < bestCost) {
					bestCost = cost;
					bestBucket = i;
					bestComponent = componentPos;
					laabb = lbox;
					raabb = backward[bestBucket + 1].second;
					bestSplit = from + lcnt - 1;
				}
				
			}
		}
		assert(bestBucket != -1);

		// distribute to left and right also computing the bounding boxes
		const float bbLenInv = 1.0f / (centroidBB.max[bestComponent] - centroidBB.min[bestComponent]);
		int off = from;
		for (int i = from; i < to; ++i) {
			const auto & trig = (*triangles)[indices[i]];
			int idx = numBuckets * ((trig.centroid()[bestComponent] - centroidBB.min[bestComponent]) * bbLenInv);
			if (idx == numBuckets) --idx;
			if (idx <= bestBucket) {
				std::swap(indices[i], indices[off++]);
			}
		}

		outCost = 1.0f + bestCost / node.m_aabb.area();
		splitPosition = bestSplit;
	}

};