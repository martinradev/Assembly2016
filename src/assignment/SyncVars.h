#pragma once

#include "track.h"
#include "sync.h"
#include "bass.h"

#define SYNC_EXT(name) \
		extern const sync_track * name##Ptr; \
		extern float name;

#define SYNC_DEF(name) \
		const sync_track * name##Ptr; \
		float name;

namespace FWSync {

	extern const sync_track *simulationStepPtr;
	extern float simulationStep;

	extern const sync_track *curlStepPtr;
	extern float curlStep;

	extern const sync_track *logoSDFdtPtr;
	extern float logoSDFdt;

	extern const sync_track * gradRPtr;
	extern float gradR;
	extern const sync_track *gradGPtr;
	extern float gradG;
	extern const sync_track *gradBPtr;
	extern float gradB;
	extern const sync_track *gradMixPtr;
	extern float gradMix;

	extern const sync_track *blurOutPtr;
	extern float blurOut;

	extern const sync_track *sceneIndexPtr;
	extern float sceneIndex;

	extern const sync_track *colorGradingIndexPtr;
	extern float colorGradingIndex;

	SYNC_EXT(logoGodrayColorR);
	SYNC_EXT(logoGodrayColorG);
	SYNC_EXT(logoGodrayColorB);

	SYNC_EXT(logoGodrayDecay);

	SYNC_EXT(waterLightParticleSize);

	SYNC_EXT(cameraIndex);
	SYNC_EXT(cameraTime);

	SYNC_EXT(rocketTriangles);

	SYNC_EXT(submarineOffset);
	SYNC_EXT(submarineExplode);

	SYNC_EXT(rocketRibbonAlpha);

	SYNC_EXT(ribbonStart);
	SYNC_EXT(ribbonEnd);

	SYNC_EXT(bloom_lumi);
	SYNC_EXT(bloom_thre);
	SYNC_EXT(bloom_offset);
	SYNC_EXT(exposure);

	SYNC_EXT(nefertitiParticleStep);
	SYNC_EXT(nefertitiParticleCurlStep);
	SYNC_EXT(cloudParticleStep);
	SYNC_EXT(cloudParticleCurlStep);
	SYNC_EXT(nefertitiAlpha);
	SYNC_EXT(godrayWeight);

	SYNC_EXT(sdf1);
	SYNC_EXT(sdf2);
	SYNC_EXT(sdf3);
	SYNC_EXT(sdf4);
	SYNC_EXT(sdf5);
	SYNC_EXT(sdf6);
	SYNC_EXT(sdf7);
	SYNC_EXT(sdf8);
	SYNC_EXT(sdf9);
	SYNC_EXT(sdf10);
	SYNC_EXT(sdf11);
	SYNC_EXT(sdf12);

	SYNC_EXT(sdf13);
	SYNC_EXT(sdf14);
	SYNC_EXT(sdf15);
	SYNC_EXT(sdf16);

	SYNC_EXT(attractorPower);

	SYNC_EXT(trefoilTime);

	SYNC_EXT(overlayIndex);
	SYNC_EXT(overlayAlpha);

	SYNC_EXT(citySineWaveTime);

	SYNC_EXT(fadeMix);
	SYNC_EXT(fadeColor);

	SYNC_EXT(fov);

	SYNC_EXT(knotRotate);


	SYNC_EXT(budhaScale);

	SYNC_EXT(particleSize);

	void bindTracks(sync_device * rocket, HSTREAM hstream);
	void updateValues(sync_device * rocket, HSTREAM hstream);

};