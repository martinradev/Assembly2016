#include "SSAOKernel.h"

namespace FW {

	SSAOKernel::SSAOKernel(unsigned numSamples) {

		m_hemisphereVectors.resize(numSamples);

		Random R;

		for (unsigned i = 0; i < numSamples; ++i) {

			Vec3f rndVec(R.getF32(-1,1), R.getF32(-1, 1), R.getF32(0,1));
			rndVec.normalize();
			m_hemisphereVectors[i] = Vec4f(rndVec, 0);

		}

		glGenBuffers(1, &m_sampleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_sampleSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numSamples * sizeof(Vec4f), m_hemisphereVectors.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


	}

};