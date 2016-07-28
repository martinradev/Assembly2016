#include "Samplers.h"
#include "Util.h"
#include "base/Random.hpp"

namespace FW {

	LowDiscrepancySampler::LowDiscrepancySampler(unsigned numSamples) 
		:
		m_numSamples(numSamples*numSamples)
	{
		m_samples.resize(m_numSamples);

		float strataStep = 1.0f / float(numSamples);
		Random rnd;

		const unsigned scramble[2] = { rnd.getU32(), rnd.getU32() };
		float strataX = 0.0f, strataY = 0.0f;
		unsigned currentSampleIndex = 0;
		for (unsigned i = 0; i < numSamples; ++i) {
			strataY = 0.0f;
			for (unsigned j = 0; j < numSamples; ++j) {
				FW::Vec2f off = Vec2f(strataX + VanDerCorput(currentSampleIndex, scramble[0]) * strataStep, strataY + Sobol2(currentSampleIndex, scramble[1]) * strataStep);
				m_samples[currentSampleIndex++] = off;
				strataY += strataStep;
			}
			strataX += strataStep;
		}

	}

};