#include "AutomaticDifferentiation.h"

#include "base/Math.hpp"

DualNumber::DualNumber(float value, float derivative) :
value_(value),
derivative_(derivative) {}

DualNumber DualNumber::operator *(const DualNumber & rht) const {
	return DualNumber(value_ * rht.value_, value_ * rht.derivative_ + derivative_ * rht.value_);
}

DualNumber DualNumber::operator +(const DualNumber & rht) const {
	return DualNumber(value_ + rht.value_, rht.derivative_ + derivative_);
}

DualNumber DualNumber::operator -(const DualNumber & rht) const {
	return DualNumber(value_ - rht.value_, derivative_ - rht.derivative_);
}

DualNumber DualNumber::operator /(const DualNumber & rht) const {
	return DualNumber(
		value_ * rht.value_ / (rht.value_ *  rht.value_),
		(rht.value_ * derivative_ - value_ * rht.derivative_) / (rht.value_ *  rht.value_)
	);
}

DualNumber DualNumber::fromConstant(float value) {
	return DualNumber(value, 0.0f);
}

DualNumber DualNumber::sin(const DualNumber & value) {
	return DualNumber(sinf(value.value_), cosf(value.value_) * value.derivative_);
}

DualNumber DualNumber::cos(const DualNumber & value) {
	return DualNumber(cosf(value.value_), -sinf(value.value_) * value.derivative_);
}

DualNumber DualNumber::sqrt(const DualNumber & value) {
	float mySqrt = sqrtf(value.value_);
	return DualNumber(mySqrt, value.derivative_ * 0.5f / mySqrt);
}

DualNumber DualNumber::exp(const DualNumber & value) {
	float myExp = expf(value.value_);
	return DualNumber(myExp, value.derivative_ * myExp);
}