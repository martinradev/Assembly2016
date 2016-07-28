#pragma once

#include "base/Math.hpp"
#include "RTTriangle.hpp"
#include <iostream>
#include <vector>

namespace FW {

	class MortonCodesHelper {
	public:

		typedef std::pair<unsigned int, unsigned int> MortonPair;

		// http://devblogs.nvidia.com/parallelforall/thinking-parallel-part-iii-tree-construction-gpu/
		// Expands a 10-bit integer into 30 bits
		// by inserting 2 zeros after each bit.
		static inline unsigned int expandBits(unsigned int v)
		{
			v = (v * 0x00010001u) & 0xFF0000FFu;
			v = (v * 0x00000101u) & 0x0F00F00Fu;
			v = (v * 0x00000011u) & 0xC30C30C3u;
			v = (v * 0x00000005u) & 0x49249249u;
			return v;
		}

		// Calculates a 30-bit Morton code for the
		// given 3D point located within the unit cube [0,1].
		static inline unsigned int morton3D(float x, float y, float z)
		{
			x = FW::min(FW::max(x * 1024.0f, 0.0f), 1023.0f);
			y = FW::min(FW::max(y * 1024.0f, 0.0f), 1023.0f);
			z = FW::min(FW::max(z * 1024.0f, 0.0f), 1023.0f);
			unsigned int xx = expandBits((unsigned int)x);
			unsigned int yy = expandBits((unsigned int)y);
			unsigned int zz = expandBits((unsigned int)z);

			return xx * 4 + yy * 2 + zz;
		}

		static int findSplit(const std::vector<MortonPair> & mortonPairs, int from, int to);
		static void generateMortonPairs(std::vector<MortonPair> & mortonPairs, const std::vector<RTTriangle> * triangles);
	};
};