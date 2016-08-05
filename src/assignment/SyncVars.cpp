#include "SyncVars.h"
#include <stdio.h>
#include <math.h>

namespace FWSync {

	const sync_track *simulationStepPtr;
	float simulationStep;

	const sync_track *curlStepPtr;
	float curlStep;

	const sync_track *logoSDFdtPtr;
	float logoSDFdt;

	const sync_track *gradRPtr;
	float gradR;
	const sync_track *gradGPtr;
	float gradG;
	const sync_track *gradBPtr;
	float gradB;
	const sync_track *gradMixPtr;
	float gradMix;

	const sync_track *sceneIndexPtr;
	float sceneIndex;

	const sync_track *colorGradingIndexPtr;
	float colorGradingIndex;

	const sync_track *blurOutPtr;
	float blurOut;

	SYNC_DEF(logoGodrayColorR);
	SYNC_DEF(logoGodrayColorG);
	SYNC_DEF(logoGodrayColorB);

	SYNC_DEF(logoGodrayDecay);

	SYNC_DEF(waterLightParticleSize);

	SYNC_DEF(cameraTime);
	SYNC_DEF(cameraIndex);
	SYNC_DEF(rocketTriangles);
	SYNC_DEF(submarineOffset);
	SYNC_DEF(submarineExplode);
	SYNC_DEF(rocketRibbonAlpha);
	SYNC_DEF(ribbonStart);
	SYNC_DEF(ribbonEnd);
	SYNC_DEF(bloom_lumi);
	SYNC_DEF(bloom_thre);
	SYNC_DEF(bloom_offset);
	SYNC_DEF(exposure);

	SYNC_DEF(nefertitiParticleStep);
	SYNC_DEF(nefertitiParticleCurlStep);
	SYNC_DEF(cloudParticleStep);
	SYNC_DEF(cloudParticleCurlStep);
	SYNC_DEF(nefertitiAlpha);
	SYNC_DEF(godrayWeight);

	SYNC_DEF(sdf1);
	SYNC_DEF(sdf2);
	SYNC_DEF(sdf3);
	SYNC_DEF(sdf4);
	SYNC_DEF(sdf5);
	SYNC_DEF(sdf6);
	SYNC_DEF(sdf7);
	SYNC_DEF(sdf8);
	SYNC_DEF(sdf9);
	SYNC_DEF(sdf10);
	SYNC_DEF(sdf11);
	SYNC_DEF(sdf12);
	SYNC_DEF(sdf13);
	SYNC_DEF(sdf14);
	SYNC_DEF(sdf15);
	SYNC_DEF(sdf16);

	SYNC_DEF(attractorPower);

	SYNC_DEF(trefoilTime);
	SYNC_DEF(overlayIndex);
	SYNC_DEF(overlayAlpha);

	SYNC_DEF(citySineWaveTime);

	SYNC_DEF(fadeMix);
	SYNC_DEF(fadeColor);

	SYNC_DEF(fov);

	SYNC_DEF(knotRotate);

	SYNC_DEF(budhaScale);

	SYNC_DEF(particleSize);
	SYNC_DEF(particleDim);
	SYNC_DEF(pUpdateTo);
	SYNC_DEF(pUpdateFrom);

	static const float bpm = 150.0f; /* beats per minute */
	static const float rpb = 8.0f; /* rows per beat */
	static const double row_rate = (double(bpm) / 60) * rpb;
#ifndef SYNC_PLAYER
	static void bass_pause(void *d, int flag)
	{
		if (flag)
			BASS_ChannelPause((HSTREAM)d);
		else
			BASS_ChannelPlay((HSTREAM)d, false);
	}

	static void bass_set_row(void *d, int row)
	{
		QWORD pos = BASS_ChannelSeconds2Bytes((HSTREAM)d, row / row_rate);
		BASS_ChannelSetPosition((HSTREAM)d, pos, BASS_POS_BYTE);
	}

	static int bass_is_playing(void *d)
	{
		return BASS_ChannelIsActive((HSTREAM)d) == BASS_ACTIVE_PLAYING;
	}

	struct sync_cb bass_cb = {
		bass_pause,
		bass_set_row,
		bass_is_playing
	};
#endif

	static double bass_get_row(HSTREAM h)
	{
		QWORD pos = BASS_ChannelGetPosition(h, BASS_POS_BYTE);
		double time = BASS_ChannelBytes2Seconds(h, pos);
		return time * row_rate;
	}

	void bindTracks(sync_device * rocket, HSTREAM hstream) {

#ifndef SYNC_PLAYER
		sync_set_callbacks(rocket, &bass_cb, (void *)hstream);
		if (sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT)) {
			::printf("failed to connect to host");
			exit(1);
		}
#endif

		simulationStepPtr = sync_get_track(rocket, "simStep");
		curlStepPtr = sync_get_track(rocket, "curlStep");
		logoSDFdtPtr = sync_get_track(rocket, "logoDT");

		gradRPtr = sync_get_track(rocket, "grad_r");
		gradGPtr = sync_get_track(rocket, "grad_g");
		gradBPtr = sync_get_track(rocket, "grad_b");
		gradMixPtr = sync_get_track(rocket, "grad_mix");

		blurOutPtr = sync_get_track(rocket, "fade_out");
		sceneIndexPtr = sync_get_track(rocket, "scene_index");
		colorGradingIndexPtr = sync_get_track(rocket, "color_LUT");

		logoGodrayColorRPtr = sync_get_track(rocket, "lgc_r");
		logoGodrayColorGPtr = sync_get_track(rocket, "lgc_g");
		logoGodrayColorBPtr = sync_get_track(rocket, "lgc_b");

		logoGodrayDecayPtr = sync_get_track(rocket, "lg_decay");

		waterLightParticleSizePtr = sync_get_track(rocket, "wa_p_size");
		cameraTimePtr = sync_get_track(rocket, "cam_time");

		submarineOffsetPtr = sync_get_track(rocket, "sub_off");
		rocketTrianglesPtr = sync_get_track(rocket, "rocket_triangles");
		submarineExplodePtr = sync_get_track(rocket, "sub_expl");
		rocketRibbonAlphaPtr = sync_get_track(rocket, "rr_alpha");

		cameraIndexPtr = sync_get_track(rocket, "cam_idx");

		ribbonStartPtr = sync_get_track(rocket, "ribbon_start");
		ribbonEndPtr = sync_get_track(rocket, "ribbon_end");

		bloom_lumiPtr = sync_get_track(rocket, "bloom_lumi");
		bloom_threPtr = sync_get_track(rocket, "bloom_thre");
		bloom_offsetPtr = sync_get_track(rocket, "bloom_offset");
		exposurePtr = sync_get_track(rocket, "exposure");

		nefertitiParticleStepPtr = sync_get_track(rocket, "nef_p_step");
		nefertitiParticleCurlStepPtr = sync_get_track(rocket, "nef_curl_step");
		cloudParticleStepPtr = sync_get_track(rocket, "cloud_p_step");
		cloudParticleCurlStepPtr = sync_get_track(rocket, "cloud_curl_step");
		nefertitiAlphaPtr = sync_get_track(rocket, "nefertiti_alpha");
		godrayWeightPtr = sync_get_track(rocket, "godray_weight");

		sdf1Ptr = sync_get_track(rocket, "sdf_1");
		sdf2Ptr = sync_get_track(rocket, "sdf_2");
		sdf3Ptr = sync_get_track(rocket, "sdf_3");
		sdf4Ptr = sync_get_track(rocket, "sdf_4");
		sdf5Ptr = sync_get_track(rocket, "sdf_5");
		sdf6Ptr = sync_get_track(rocket, "sdf_6");
		sdf7Ptr = sync_get_track(rocket, "sdf_7");
		sdf8Ptr = sync_get_track(rocket, "sdf_8");
		sdf9Ptr = sync_get_track(rocket, "sdf_9");
		sdf10Ptr = sync_get_track(rocket, "sdf_10");
		sdf11Ptr = sync_get_track(rocket, "sdf_11");
		sdf12Ptr = sync_get_track(rocket, "sdf_12");

		sdf13Ptr = sync_get_track(rocket, "sdf_13");
		sdf14Ptr = sync_get_track(rocket, "sdf_14");
		sdf15Ptr = sync_get_track(rocket, "sdf_15");
		sdf16Ptr = sync_get_track(rocket, "sdf_16");

		attractorPowerPtr = sync_get_track(rocket, "attractorPower");

		trefoilTimePtr = sync_get_track(rocket, "trefoil_t");

		overlayIndexPtr = sync_get_track(rocket, "overlay_idx");
		overlayAlphaPtr = sync_get_track(rocket, "overlay_a");

		citySineWaveTimePtr = sync_get_track(rocket, "city_sine_t");

		fadeMixPtr = sync_get_track(rocket, "fade_mix");
		fadeColorPtr = sync_get_track(rocket, "fade_color");

		fovPtr = sync_get_track(rocket, "fov");

		knotRotatePtr = sync_get_track(rocket, "knot_rotate");

		budhaScalePtr = sync_get_track(rocket, "budha_scale");

		particleSizePtr = sync_get_track(rocket, "particle_size");
		particleDimPtr = sync_get_track(rocket, "particle_dim");
		pUpdateFromPtr = sync_get_track(rocket, "p_update_from");
		pUpdateToPtr = sync_get_track(rocket, "p_update_to");
	}



	void updateValues(sync_device * rocket, HSTREAM hstream) {
		double row = bass_get_row(hstream);

#ifndef SYNC_PLAYER
		if (sync_update(rocket, (int)::floor(row)))
			sync_connect(rocket, "localhost", SYNC_DEFAULT_PORT);
#endif

		simulationStep = sync_get_val(FWSync::simulationStepPtr, row);
		curlStep = sync_get_val(FWSync::curlStepPtr, row);
		
		logoSDFdt = sync_get_val(FWSync::logoSDFdtPtr, row);

		gradR = sync_get_val(FWSync::gradRPtr, row);
		gradG = sync_get_val(FWSync::gradGPtr, row);
		gradB = sync_get_val(FWSync::gradBPtr, row);
		gradMix = sync_get_val(FWSync::gradMixPtr, row);

		blurOut = sync_get_val(FWSync::blurOutPtr, row);
		sceneIndex = sync_get_val(FWSync::sceneIndexPtr, row);
		colorGradingIndex = sync_get_val(FWSync::colorGradingIndexPtr, row);

		logoGodrayColorR = sync_get_val(FWSync::logoGodrayColorRPtr, row);
		logoGodrayColorG = sync_get_val(FWSync::logoGodrayColorGPtr, row);
		logoGodrayColorB = sync_get_val(FWSync::logoGodrayColorBPtr, row);

		logoGodrayDecay = sync_get_val(FWSync::logoGodrayDecayPtr, row);

		waterLightParticleSize = sync_get_val(FWSync::waterLightParticleSizePtr, row);
		
		rocketTriangles = sync_get_val(FWSync::rocketTrianglesPtr, row);
		submarineOffset = sync_get_val(FWSync::submarineOffsetPtr, row);
		submarineExplode = sync_get_val(FWSync::submarineExplodePtr, row);
		rocketRibbonAlpha = sync_get_val(FWSync::rocketRibbonAlphaPtr, row);

		cameraTime = sync_get_val(FWSync::cameraTimePtr, row);
		cameraIndex = sync_get_val(FWSync::cameraIndexPtr, row);

		ribbonStart = sync_get_val(FWSync::ribbonStartPtr, row);
		ribbonEnd = sync_get_val(FWSync::ribbonEndPtr, row);

		bloom_lumi = sync_get_val(FWSync::bloom_lumiPtr, row);
		bloom_offset = sync_get_val(FWSync::bloom_offsetPtr, row);
		bloom_thre = sync_get_val(FWSync::bloom_threPtr, row);
		exposure = sync_get_val(FWSync::exposurePtr, row);

		nefertitiParticleStep = sync_get_val(FWSync::nefertitiParticleStepPtr, row);
		nefertitiParticleCurlStep = sync_get_val(FWSync::nefertitiParticleCurlStepPtr, row);
		cloudParticleStep = sync_get_val(FWSync::cloudParticleStepPtr, row);
		cloudParticleCurlStep = sync_get_val(FWSync::cloudParticleCurlStepPtr, row);
		nefertitiAlpha = sync_get_val(FWSync::nefertitiAlphaPtr, row);
		godrayWeight = sync_get_val(FWSync::godrayWeightPtr, row);

		sdf1 = sync_get_val(FWSync::sdf1Ptr, row);
		sdf2 = sync_get_val(FWSync::sdf2Ptr, row);
		sdf3 = sync_get_val(FWSync::sdf3Ptr, row);
		sdf4 = sync_get_val(FWSync::sdf4Ptr, row);
		sdf5 = sync_get_val(FWSync::sdf5Ptr, row);
		sdf6 = sync_get_val(FWSync::sdf6Ptr, row);
		sdf7 = sync_get_val(FWSync::sdf7Ptr, row);
		sdf8 = sync_get_val(FWSync::sdf8Ptr, row);
		sdf9 = sync_get_val(FWSync::sdf9Ptr, row);
		sdf10 = sync_get_val(FWSync::sdf10Ptr, row);
		sdf11 = sync_get_val(FWSync::sdf11Ptr, row);
		sdf12 = sync_get_val(FWSync::sdf12Ptr, row);
		sdf13 = sync_get_val(FWSync::sdf13Ptr, row);
		sdf14 = sync_get_val(FWSync::sdf14Ptr, row);
		sdf15 = sync_get_val(FWSync::sdf15Ptr, row);
		sdf16 = sync_get_val(FWSync::sdf16Ptr, row);

		attractorPower = sync_get_val(FWSync::attractorPowerPtr, row);
		trefoilTime = sync_get_val(FWSync::trefoilTimePtr, row);

		overlayIndex = sync_get_val(FWSync::overlayIndexPtr, row);
		overlayAlpha = sync_get_val(FWSync::overlayAlphaPtr, row);

		citySineWaveTime = sync_get_val(FWSync::citySineWaveTimePtr, row);

		fadeMix = sync_get_val(FWSync::fadeMixPtr, row);
		fadeColor = sync_get_val(FWSync::fadeColorPtr, row);

		fov = sync_get_val(FWSync::fovPtr, row);
		knotRotate = sync_get_val(FWSync::knotRotatePtr, row);

		budhaScale = sync_get_val(FWSync::budhaScalePtr, row);

		particleSize = sync_get_val(FWSync::particleSizePtr, row);
		particleDim = sync_get_val(FWSync::particleDimPtr, row);

		pUpdateFrom = sync_get_val(FWSync::pUpdateFromPtr, row);
		pUpdateTo = sync_get_val(FWSync::pUpdateToPtr, row);
	}

};