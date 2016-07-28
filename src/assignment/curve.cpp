#include "curve.h"
#include "AutomaticDifferentiation.h"
#include <iostream>

#ifndef M_PI
#define M_PI  3.14159265358979
#endif

using namespace std;
using namespace FW;

namespace {

// Approximately equal to.  We don't want to use == because of
// precision issues with floating point.
inline bool approx(const Vec3f& lhs, const Vec3f& rhs) {
	const float eps = 1e-8f;
	return (lhs - rhs).lenSqr() < eps;
}

static Vec3f getBInit(const Vec3f & p1, const Vec3f & p2) {
	static const Vec3f axes[2] = {
		Vec3f(1.0f, 0.0f, 0.0f),
		Vec3f(0.0f, 1.0f, 0.0f)
	};
	Vec3f tangent = p2 - p1;
	Vec3f res = cross(tangent, axes[0]);
	if (res.isZero()) return cross(tangent, axes[1]).normalized();
	return res.normalized();
}

static Mat4f bezierBaseMatrix() {
	static Mat4f B;
	B.setRow(0, Vec4f(1.0f, -3.0f, 3.0f, -1.0f));
	B.setRow(1, Vec4f(0.0f, 3.0f, -6.0f, 3.0f));
	B.setRow(2, Vec4f(0.0f, 0.0f, 3.0f, -3.0f));
	B.setRow(3, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
	return B;
}

static Mat4f bezierTangentBaseMatrix() {
	static Mat4f B;
	B.setRow(0, Vec4f(-3.0f, 6.0f, -3.0f, 0.0f));
	B.setRow(1, Vec4f(3.0f, -12.0f, 9.0f, 0.0f));
	B.setRow(2, Vec4f(0.0f, 6.0f, -9.0f, 0.0f));
	B.setRow(3, Vec4f(0.0f, 0.0f, 3.0f, 0.0f));
	return B;
}

static Mat4f bSplineBaseMatrix() {
	static Mat4f B;
	B.setRow(0, Vec4f(1.0f, -3.0f, 3.0f, -1.0f));
	B.setRow(1, Vec4f(4.0f, 0.0f, -6.0f, 3.0f));
	B.setRow(2, Vec4f(1.0f, 3.0f, 3.0f, -3.0f));
	B.setRow(3, Vec4f(0.0f, 0.0f, 0.0f, 1.0f));
	return B/6.0f;
}

static Mat4f catmullRomBaseMatrix() {
	static Mat4f B;
	B.setCol(0, Vec4f(0.0f, 2.0f, 0.0f, 0.0f));
	B.setCol(1, Vec4f(-1.0f, 0.0f, 1.0f, 0.0f));
	B.setCol(2, Vec4f(2.0f, -5.0f, 4.0f, -1.0f));
	B.setCol(3, Vec4f(-1.0f, 3.0f, -3.0f, 1.0f));
	return 0.5f * B;
}

static Vec3f getBezierCurvature(const Vec3f & p1, const Vec3f & p2, const Vec3f & p3, const Vec3f & p4, float t, float TValueSQ) {
	return (6.0f * (1.0f - t) * p1 + (-12.0f + 18.0f * t) * p2 + (6.0f - 18.0f * t) * p3 + 6.0f * t * p4) / sqrtf(TValueSQ);
}

static Mat4f geometryMatrix(const Vec3f & p0, const Vec3f & p1, const Vec3f & p2, const Vec3f & p3) {
	static Mat4f G;
	G.setCol(0, Vec4f(p0, 1.0f));
	G.setCol(1, Vec4f(p1, 1.0f));
	G.setCol(2, Vec4f(p2, 1.0f));
	G.setCol(3, Vec4f(p3, 1.0f));
	return G;
}


/*
	Fixes the coordinate space of the beginning and the end of the curve
*/

static void alignNormalsOfCurve(Curve & curve) {

	const float eps = 1e-9f;

	if (!approx(curve[0].V, curve.back().V)) return;

	Vec3f N0 = curve[0].T;
	Vec3f N1 = curve[0].N;
	Vec3f N2 = curve.back().N;

	float cosAngle = clamp(dot(N1, N2), -1.0f, 1.0f);

	float angle = acosf(cosAngle);

	if (dot(N0, cross(N1, N2)) <= 0.0f) {
		angle = -angle;
	}


	if (angle >= -eps && angle <= eps) return;
	

	for (size_t i = 1; i < curve.size(); ++i) {
		float cAngle = (angle * i) / float(curve.size()-1);
		Mat3f rot = Mat3f::rotation(curve[i].T.normalized(), -cAngle);
		curve[i].B = rot * curve[i].B;
		curve[i].N = rot * curve[i].N;
		curve[i].C = rot * curve[i].C;
	}

}

// This is the core routine of the curve evaluation code. Unlike
// evalBezier, this is only designed to work on 4 control points.
// Furthermore, it requires you to specify an initial binormal
// Binit, which is iteratively propagated throughout the curve as
// the curvepoints are generated. Any other function that creates
// cubic splines can use this function by a corresponding change
// of basis.
Curve coreBezier(const Vec3f& p0,
				 const Vec3f& p1,
				 const Vec3f& p2,
				 const Vec3f& p3,
				 const Vec3f& Binit,
				 unsigned steps, float tOffset = 0.0f) {

	Curve R(steps + 1);
	// YOUR CODE HERE (R1): build the basis matrix and loop the given number of steps,
	// computing points on the spline

	// base matrix
	Mat4f B = bezierBaseMatrix();
	Mat4f BTangent = bezierTangentBaseMatrix();

	// geometry matrix
	Mat4f G = geometryMatrix(p0, p1, p2, p3);

	Mat4f GB = G*B;
	Mat4f GBT = G * BTangent;
	
	float t = 0.0f;
	Vec4f tVec;

	// ...
	float dt = 1.0f / float(steps);
	for (unsigned i = 0; i <= steps; ++i) {
		tVec = Vec4f(1.0f, t, t*t, t*t*t);
		R[i].V = (GB * tVec).getXYZ();
		R[i].T = (GBT * tVec).getXYZ();
		
		R[i].C = getBezierCurvature(p0, p1, p2, p3, t, R[i].T.lenSqr());
		R[i].T.normalize();
		if (i == 0) {
			R[i].N = cross(Binit, R[i].T).normalized();
		}
		else {
			R[i].N = cross(R[i - 1].B, R[i].T).normalized();
		}
		R[i].B = cross(R[i].T, R[i].N).normalized();
		R[i].t = t+ tOffset;
		
		t += dt;
	}

	return R;
}    

} // namespace

static float distanceToSegment(const Vec3f & p1, const Vec3f & p2, const Vec3f & q) {
	Vec3f v = (p2 - p1).normalized();
	Vec3f u = q - p1;
	float d = dot(u, v);
	return sqrtf(u.lenSqr() - d*d);
}

Curve coreBezierRecursive(const Vec3f & p1,
	const Vec3f & p2,
	const Vec3f & p3,
	const Vec3f & p4,
	const Vec3f & Binit,
	const float begin,
	const float end,
	const float errorbound,
	const float minstep) {

	Vec3f p12 = (p1 + p2) / 2.0f;
	Vec3f p23 = (p2 + p3) / 2.0f;
	Vec3f p34 = (p3 + p4) / 2.0f;
	Vec3f p123 = (p12 + p23) / 2.0f;
	Vec3f p234 = (p23 + p34) / 2.0f;
	Vec3f p1234 = (p123 + p234) / 2.0f;

	
	float d1 = distanceToSegment(p1, p4, p2);
	float d2 = distanceToSegment(p1, p4, p3);
	float error = d1 < d2 ? d2 : d1;
	
	float tmid = (end - begin) / 2.0f;

	if (errorbound > error) {

		Mat4f G = geometryMatrix(p1, p2, p3, p4);
		Mat4f BTangent = bezierTangentBaseMatrix();
		float t = 0.5f;
		Vec4f tVec(1.0f, t, t*t, t*t*t);

		Curve curve;
		CurvePoint point;
		point.V = p1234;
		point.T = (G * BTangent * tVec).getXYZ();
		point.C = getBezierCurvature(p1,p2,p3,p4, t, point.T.lenSqr());
		point.T.normalize();
		point.N = cross(Binit, point.T).normalized();
		point.B = cross(point.T, point.N).normalized();
		point.t = tmid;
		curve.push_back(point);
		return curve;
	}

	
	Curve curve;
	
	Curve left = coreBezierRecursive(p1, p12, p123, p1234, Binit, begin, tmid, errorbound, minstep);
	Curve right = coreBezierRecursive(p1234, p234, p34, p4, left.back().B, tmid, end, errorbound, minstep);

	curve.insert(curve.begin(), left.begin(), left.end());
	curve.insert(curve.end(), right.begin(), right.end());


	return curve;

}

Curve coreBezier(const Vec3f& p1,
	const Vec3f& p2,
	const Vec3f& p3,
	const Vec3f& p4,
	const Vec3f& Binit,
	const float begin, const float end, const float errorbound, const float minstep) {

	Curve curve;

	CurvePoint point;
	point.T = (p2 - p1).normalized();
	point.N = cross(Binit, point.T).normalized();
	point.B = cross(point.T, point.N);
	point.C = getBezierCurvature(p1, p2, p3, p4, begin, (p2 - p1).lenSqr());
	point.V = p1;
	point.t = begin;
	curve.push_back(point);

	Curve midCurve = coreBezierRecursive(p1, p2, p3, p4, point.B, begin, end, errorbound, minstep);
	curve.insert(curve.end(), midCurve.begin(), midCurve.end());

	point.V = p4;
	point.T = (p4 - p3).normalized();
	point.N = cross(midCurve.back().B, point.T);
	point.B = cross(point.T, point.N);
	point.C = getBezierCurvature(p1, p2, p3, p4, end, (p4-p3).lenSqr());
	point.t = end;
	curve.push_back(point);

	alignNormalsOfCurve(curve);

	return curve;
}
    
// the P argument holds the control points and steps gives the amount of uniform tessellation.
// the rest of the arguments are for the adaptive tessellation extra.
Curve evalBezier(const vector<Vec3f>& P, unsigned steps, bool adaptive, float errorbound, float minstep) {
    // Check

	Curve curve;
    if (P.size() < 4 || P.size() % 3 != 1) {
        //cerr << "evalBezier must be called with 3n+1 control points." << endl;
		return curve;
	}

    // YOUR CODE HERE (R1):
    // You should implement this function so that it returns a Curve
    // (e.g., a vector<CurvePoint>).  The variable "steps" tells you
    // the number of points to generate on each piece of the spline.
    // At least, that's how the sample solution is implemented and how
    // the SWP files are written.  But you are free to interpret this
    // variable however you want, so long as you can control the
    // "resolution" of the discretized spline curve with it.

	// EXTRA CREDIT NOTE:
    // Also compute the other Vec3fs for each CurvePoint: T, N, B.
    // A matrix [N, B, T] should be unit and orthogonal.
    // Also note that you may assume that all Bezier curves that you
    // receive have G1 continuity. The T, N and B vectors will not
	// have to be defined at points where this does not hold.

	
	
	Vec3f bInit = getBInit(P[0], P[1]);

	for (size_t i = 0; i < P.size()-3; i+=3) {
		const Vec3f & p1 = P[i];
		const Vec3f & p2 = P[i+1];
		const Vec3f & p3 = P[i+2];
		const Vec3f & p4 = P[i+3];
		
		Curve segmentCurve;

		if (adaptive) {
			segmentCurve = coreBezier(p1, p2, p3, p4, bInit, 0.0f, 1.0f, errorbound, minstep);
		}
		else {
			segmentCurve = coreBezier(p1, p2, p3, p4, bInit, steps);
		}
		curve.insert(curve.end(), segmentCurve.begin(), segmentCurve.end());

		bInit = segmentCurve.back().B;
	}

	// fix curve
	alignNormalsOfCurve(curve);

    // Right now this will just return this empty curve.
	return curve;
}

// the P argument holds the control points and steps gives the amount of uniform tessellation.
// the rest of the arguments are for the adaptive tessellation extra.
Curve evalBspline(const vector<Vec3f>& P, unsigned steps, bool adaptive, float errorbound, float minstep) {
    // Check
    if (P.size() < 4) {
       // cerr << "evalBspline must be called with 4 or more control points." << endl;
		//CrtDbgBreak();
       // exit(0);
    }

    // YOUR CODE HERE (R2):
    // We suggest you implement this function via a change of basis from
	// B-spline to Bezier.  That way, you can just call your evalBezier function.


	Curve curve;

	// G * B spline * T(t) = G' * B bezier * T(t)
	// we are interested in what G' should be. We know G, B spline and B bezier
	// G' = G * B spline * B bezier ^ (-1)

	Mat4f BB = bSplineBaseMatrix() * invert(bezierBaseMatrix());
	
	Vec3f bInit = getBInit(P[0], P[1]);
	if (bInit.isZero()) bInit = Vec3f(0, 1, 0);
	// transform and copy control points
	for (size_t i = 0; i < P.size()-3; ++i) {

		Mat4f G = geometryMatrix(P[i], P[i + 1], P[i + 2], P[i + 3]);
		Mat4f Gp = G * BB;

		Vec3f p0 = Vec3f(Gp(0, 0), Gp(1, 0), Gp(2, 0));
		Vec3f p1 = Vec3f(Gp(0, 1), Gp(1, 1), Gp(2, 1));
		Vec3f p2 = Vec3f(Gp(0, 2), Gp(1, 2), Gp(2, 2));
		Vec3f p3 = Vec3f(Gp(0, 3), Gp(1, 3), Gp(2, 3));
		
		Curve segmentCurve;
		if (adaptive) {
			segmentCurve = coreBezier(p0, p1, p2, p3, bInit, 0.0f, 1.0f, errorbound, minstep);
		}
		else {
			segmentCurve = coreBezier(p0, p1, p2, p3, bInit, steps);
		}

		curve.insert(curve.end(), segmentCurve.begin(), segmentCurve.end());

		bInit = curve.back().B;
	}

	// fix curve
	alignNormalsOfCurve(curve);

	return curve;
}

Curve evalCatmullRomspline(const vector<Vec3f>& P, unsigned steps, bool adaptive, float errorbound, float minstep) {
	// Check
	if (P.size() < 4) {
		cerr << "evalCatmullRomspline must be called with 4 or more control points." << endl;
		_CrtDbgBreak();
		exit(0);
	}

	

	// fake P points to interpolate nicely first and last point

	vector<Vec3f> Pfake(P.size() + 2);
	for (size_t i = 0; i < P.size(); ++i) Pfake[i + 1] = P[i];
	Pfake[0] = 2.0f * P[0] - P[1];
	Pfake[P.size() + 1] = 2.0f * P.back() - P[P.size()-1];


	Curve curve;

	Mat4f BB = catmullRomBaseMatrix() * invert(bezierBaseMatrix());

	// note that B init corresponds to p0 and p2 since we know that the tanget at p1 is p2-p0
	Vec3f bInit = getBInit(P[0], P[2]);

	// transform and copy control points
	for (size_t i = 1; i < Pfake.size() - 2; ++i) {

		Mat4f G = geometryMatrix(Pfake[i - 1], Pfake[i], Pfake[i + 1], Pfake[i + 2]);
		Mat4f Gp = G * BB;

		Vec3f p0 = Vec3f(Gp(0, 0), Gp(1, 0), Gp(2, 0));
		Vec3f p1 = Vec3f(Gp(0, 1), Gp(1, 1), Gp(2, 1));
		Vec3f p2 = Vec3f(Gp(0, 2), Gp(1, 2), Gp(2, 2));
		Vec3f p3 = Vec3f(Gp(0, 3), Gp(1, 3), Gp(2, 3));

		Curve segmentCurve;
		if (adaptive) {
			segmentCurve = coreBezier(p0, p1, p2, p3, bInit, 0.0f, 1.0f, errorbound, minstep);
		}
		else {
			segmentCurve = coreBezier(p0, p1, p2, p3, bInit, steps, i-1);
		}

		curve.insert(curve.end(), segmentCurve.begin(), segmentCurve.end());

		bInit = curve.back().B;
	}

	// fix curve
	//alignNormalsOfCurve(curve);

	return curve;
}

Curve evalCircle(float radius, unsigned steps) {
    // This is a sample function on how to properly initialize a Curve
    // (which is a vector<CurvePoint>).
    
    // Preallocate a curve with steps+1 CurvePoints
    Curve R(steps+1);

    // Fill it in counterclockwise
    for (unsigned i = 0; i <= steps; ++i) {
        // step from 0 to 2pi
        float t = 2.0f * (float)M_PI * float(i) / steps;

        // Initialize position
        // We're pivoting counterclockwise around the y-axis
        R[i].V = radius * Vec3f(FW::cos(t), FW::sin(t), 0);
        
        // Tangent vector is first derivative
        R[i].T = Vec3f(-FW::sin(t), FW::cos(t), 0);
        
        // Normal vector is second derivative
        R[i].N = Vec3f(-FW::cos(t), -FW::sin(t), 0);

		// curvature vector
		R[i].C = R[i].N / radius;

        // Finally, binormal is facing up.
        R[i].B = Vec3f(0, 0, 1);

		R[i].t = t;

		
    }

    return R;
}

Curve evalTrefoilKnot(unsigned steps) {
	Curve curve(steps+1);

	float step = 2.0f * FW_PI / steps;

	Vec3f Binit = Vec3f(0.0f, 1.0f, 0.0f);

	for (unsigned i = 0; i <= steps; ++i) {

		float t = step * i;
		float tmp = (2.0f + cosf(3.0f*t));

		float xCoord = tmp * cosf(2.0f * t);
		float yCoord = tmp * sinf(2.0f * t);
		float zCoord = sinf(3.0f * t);
		curve[i].V = Vec3f(xCoord, yCoord, zCoord);

		float dx = -0.5f * (sinf(t) + 8.0f * sinf(2.0f * t) + 5.0f * sinf(5.0f * t));
		float dy = 0.5f * (-cosf(t) + 8.0f * cosf(2.0f * t) + 5.0f * cosf(5.0f * t));
		float dz = 3.0f * cosf(3.0f * t);
		curve[i].T = Vec3f(dx, dy, dz);

		if (i == 0) {
			curve[i].N = cross(Binit, curve[i].T).normalized();
		}
		else {
			curve[i].N = cross(curve[i - 1].B, curve[i].T).normalized();
		}
		curve[i].B = cross(curve[i].T, curve[i].N).normalized();
		curve[i].C = curve[i].N * Vec3f(
			0.5f * (-cosf(t) - 16.0f * cosf(2.0f * t) -25.0f * cosf(5.0f * t)),
			0.5f * (sinf(t) - 16.0f * sinf(2.0f * t) - 25.0f * sinf(5.0f * t)),
			-9.0f * sinf(3.0f * t)).length();
		curve[i].t = t;

	}

	alignNormalsOfCurve(curve);

	return curve;
}

Curve evalCustomCurve(const std::function<DualParamCoord(float)> & function, unsigned steps, float from, float to) {
	Curve curve(steps + 1);

	float span = (to - from);
	float step = span / steps;

	Vec3f Binit = Vec3f(0.0f, 1.0f, 0.0f);

	for (unsigned i = 0; i <= steps; ++i) {

		float t = from + step * i;
		
		DualParamCoord result = function(t);


		curve[i].V = Vec3f(result.x_.getValue(), result.y_.getValue(), result.z_.getValue());
		curve[i].T = Vec3f(result.x_.getDerivative(), result.y_.getDerivative(), result.z_.getDerivative()).normalized();

		if (i == 0) {
			curve[i].N = cross(Binit, curve[i].T).normalized();
		}
		else {
			curve[i].N = cross(curve[i - 1].B, curve[i].T).normalized();
		}
		curve[i].B = cross(curve[i].T, curve[i].N).normalized();
		// don't care at the moment
		curve[i].C = Vec3f(1);
		curve[i].t = t;

	}

	alignNormalsOfCurve(curve);

	return curve;
}

Curve evalArchytasKnot(unsigned steps) {
	Curve curve(steps + 1);

	float step = 2.0f * FW_PI / steps;

	Vec3f Binit = Vec3f(0.0f, 1.0f, 0.0f);

	for (unsigned i = 0; i <= steps; ++i) {

		float t = step * i;
		
		DualNumber cosT = DualNumber::cos(DualNumber(t, 1.0f));
		DualNumber sinT = DualNumber::sin(DualNumber(t, 1.0f));
		DualNumber sin2T = DualNumber::sin(DualNumber(2.0 * t, 2.0f));

		DualNumber cos4T = cosT*cosT;
		cos4T = cos4T * cos4T;
		DualNumber det = DualNumber::fromConstant(1.0f) + sinT * sinT;
		DualNumber det2 = det * det;

		DualNumber XDual = cos4T / det2;
		DualNumber YDual = (cosT * sin2T) / det2;
		DualNumber ZDual = sin2T / (det * DualNumber::fromConstant(sqrtf(2.0f)));

		curve[i].V = Vec3f(XDual.getValue(), YDual.getValue(), ZDual.getValue());
		curve[i].T = Vec3f(XDual.getDerivative(), YDual.getDerivative(), ZDual.getDerivative()).normalized();

		if (i == 0) {
			curve[i].N = cross(Binit, curve[i].T).normalized();
		}
		else {
			curve[i].N = cross(curve[i - 1].B, curve[i].T).normalized();
		}
		curve[i].B = cross(curve[i].T, curve[i].N).normalized();

		// don't care at the moment
		curve[i].C = Vec3f(1);
		curve[i].t = t;

	}

	alignNormalsOfCurve(curve);

	return curve;
}

float getCurveLength(const Curve & curve) {
	float d = 0.0f;
	for (size_t i = 1; i < curve.size(); ++i) {
		d += (curve[i].V - curve[i - 1].V).length();
	}
	return d;
}

Curve evalUnitConstantCurve(float scale) {
	Curve curve(2);
	curve[0].V = curve[1].V = Vec3f(scale);
	curve[0].t = 0.0f;
	curve[1].t = 1.0f;
	return curve;
}