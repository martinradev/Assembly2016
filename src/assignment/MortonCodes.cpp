#include "MortonCodes.h"
#include "AABB.h"
#include "ParallelSort.h"

namespace FW {

	int MortonCodesHelper::findSplit(const std::vector<MortonPair> & mortonPairs, int from, int to) {

		if (mortonPairs[from].first == mortonPairs[to - 1].first) {
			return (from + to) >> 1;
		}
		DWORD v1, v2;
		_BitScanReverse(&v1, mortonPairs[from].first ^ mortonPairs[to - 1].first);
		v1 = 32 - v1;
		int l = from, r = to - 1;

		int split = from;

		while (l <= r) {
			int mid = (l + r) >> 1;
			_BitScanReverse(&v2, mortonPairs[from].first ^ mortonPairs[mid].first);
			v2 = 32 - v2;
			if (v2 > v1) {
				l = mid + 1;
				split = mid;
			}
			else {
				r = mid - 1;
			}
		}
		return split;
	}

	void MortonCodesHelper::generateMortonPairs(std::vector<MortonPair> & mortonPairs, const std::vector<RTTriangle> * triangles) {
		// compute bounding box of the scene
		Vec3f centroid;
		AABB aabb((*triangles)[0].min(), (*triangles)[0].max());

		for (size_t i = 1; i < triangles->size(); ++i) {
			aabb.extend((*triangles)[i].min(), (*triangles)[i].max());
		}

		// find relative position to the bounding box and generate morton pairs
		for (size_t i = 0; i < triangles->size(); ++i) {
			// express it relatively to the aabb box
			centroid = (*triangles)[i].centroid();

			centroid[0] = (centroid[0] - aabb.min[0]) / (aabb.max[0] - aabb.min[0]);
			centroid[1] = (centroid[1] - aabb.min[1]) / (aabb.max[1] - aabb.min[1]);
			centroid[2] = (centroid[2] - aabb.min[2]) / (aabb.max[2] - aabb.min[2]);

			mortonPairs[i] = std::make_pair(MortonCodesHelper::morton3D(centroid[0], centroid[1], centroid[2]), i);
		}

		parallelMergeSort(mortonPairs.begin(), mortonPairs.end(),
			[](const MortonCodesHelper::MortonPair & a, const MortonCodesHelper::MortonPair & b) {
			return a.first < b.first;
		});
	}

};