#pragma once

class DualNumber {
public:

	DualNumber() = delete;
	DualNumber(float value, float derivative);
	
	DualNumber operator *(const DualNumber & rht) const;
	DualNumber operator +(const DualNumber & rht) const;
	DualNumber operator -(const DualNumber & rht) const;
	DualNumber operator /(const DualNumber & rht) const;

	float getValue() const {
		return value_;
	}

	float getDerivative() const {
		return derivative_;
	}

	static DualNumber fromConstant(float constant);
	static DualNumber sin(const DualNumber & value);
	static DualNumber cos(const DualNumber & value);
	static DualNumber sqrt(const DualNumber & value);
	static DualNumber exp(const DualNumber & value);
private:
	float value_;
	float derivative_;
};