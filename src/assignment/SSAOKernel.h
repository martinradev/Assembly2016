#pragma once

#include "base/Math.hpp"
#include "base/Random.hpp"

#include <vector>

namespace FW {

	class SSAOKernel {

	public:
		SSAOKernel() : m_sampleSSBO(0) {};
		SSAOKernel(unsigned numSamples);

		int getNumSamples() const {
			return m_hemisphereVectors.size();
		}

		GLuint getSSBOBuffer() const {
			return m_sampleSSBO;
		}

	private:
		std::vector<Vec4f> m_hemisphereVectors;
		GLuint m_sampleSSBO;
	};

};