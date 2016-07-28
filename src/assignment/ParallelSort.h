#pragma once

#include <algorithm>
#include <omp.h>

namespace FW {

	template <class RandomAccessIterator, class Compare>
	void parallelMergeSort(RandomAccessIterator first, RandomAccessIterator last, Compare comp) {
		int p = omp_get_max_threads();
		const int n = (last - first);
		const int len = n / p;
		// sort each block using std::sort
		#pragma omp parallel num_threads(p)
		{
			const int i = omp_get_thread_num();
			const int s = i*len;
			const int e = (i + 1 == p) ? n : s + len;

			std::sort(first + s, first + e, comp);
		}

		// combine results
		while (p >= 2) {

			const int len = n / p;

			#pragma omp parallel num_threads(p/2)
			{
				const int i = omp_get_thread_num();
				int s0 = i*len * 2;
				int s1 = s0 + len;
				const int e0 = s1;
				const int e1 = (i + 1 == p / 2 ? n : s1 + len);
				const int total = (e1 - s0);

				std::inplace_merge(first + s0, first + e0, first + e1);
			}

			p = (p + 1) / 2;
		}
	}

};