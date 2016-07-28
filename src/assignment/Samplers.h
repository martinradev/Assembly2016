#pragma once

#include "base/Math.hpp"

#include <vector>

namespace FW {
	
	class LowDiscrepancySampler {

	public:

		LowDiscrepancySampler(unsigned numSamples);

		unsigned getNumSamples() const {
			return m_numSamples;
		}
		Vec2f getSample(unsigned sampleIndex) const {
			return m_samples[sampleIndex];
		}
		const void * getData() const {
			return (void*)m_samples.data();
		}

	private:

		std::vector<Vec2f> m_samples;
		unsigned m_numSamples;
		
	};

}