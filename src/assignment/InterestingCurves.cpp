#include "InterestingCurves.h"

std::function<DualParamCoord(float t)> getHeartFunction() {
	auto heartF = [](float t) {

		DualNumber xt = DualNumber::sin(DualNumber(t, 1.0f));
		xt = xt * xt * xt * DualNumber::fromConstant(16.0f);
		DualNumber myCosT = DualNumber::cos(DualNumber(t, 1.0));
		DualNumber myCos2T = DualNumber::cos(DualNumber(2.0 * t, 2.0));
		DualNumber myCos3T = DualNumber::cos(DualNumber(3.0 * t, 3.0));
		DualNumber myCos4T = DualNumber::cos(DualNumber(4.0 * t, 4.0));
		DualNumber yt = myCosT * DualNumber::fromConstant(13.0f)
			- myCos2T * DualNumber::fromConstant(5.0f)
			- myCos3T * DualNumber::fromConstant(2.0f) - myCos4T;

		return DualParamCoord(xt, yt, DualNumber(0.0f, 0.0f));

	};
	return heartF;
}

std::function<DualParamCoord(float t)> getVivianiFunction(float sphereRadius) {
	auto vivianiF = [sphereRadius](float t) {

		DualNumber cosT = DualNumber::cos(DualNumber(t, 1.0f));
		DualNumber sinT = DualNumber::sin(DualNumber(t, 1.0f));
		DualNumber sinHalfT = DualNumber::sin(DualNumber(t * 0.5f, 0.5f));

		DualNumber A = DualNumber::fromConstant(sphereRadius);

		DualNumber xt = A * (DualNumber::fromConstant(1.0f) + cosT);
		DualNumber yt = A * sinT;
		DualNumber zt = DualNumber::fromConstant(2.0f) * A * sinHalfT;

		return DualParamCoord(xt, yt, zt);

	};
	return vivianiF;
}