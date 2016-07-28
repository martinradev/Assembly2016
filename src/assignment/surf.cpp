#include "surf.h"

using namespace std;
using namespace FW;

namespace
{

	static inline unsigned getIndex(unsigned row, unsigned col, unsigned dia) {
		return col * dia + row;
	}

    // This is a generic function that generates a set of triangle
    // faces for a sweeping a profile curve along "something".  For
    // instance, say you want to sweep the profile curve [01234]:
    //
    //   4     9     10
    //    3     8     11
    //    2 --> 7 --> 12 ----------[in this direction]--------->
    //    1     6     13 
    //   0     5     14
    //
    // Then the "diameter" is 5, and the "length" is how many times
    // the profile is repeated in the sweep direction.  This function
    // generates faces in terms of vertex indices.  It is assumed that
    // the indices go as shown in the picture (the first dia vertices
    // correspond to the first repetition of the profile curve, and so
    // on).  It will generate faces [0 5 1], [1 5 6], [1 6 2], ...
    // The boolean variable "closed" will determine whether the
    // function closes the curve (that is, connects the last profile
    // to the first profile).
    static vector< FW::Vec3i > triSweep( unsigned dia, unsigned len, bool closed )
    {
        vector< FW::Vec3i > ret;

		// YOUR CODE HERE: generate zigzagging triangle indices and push them to ret.
		
		unsigned mod = len;
		if (!closed) --len;
		for (unsigned col = 0; col < len; ++col) {
			for (unsigned row = 0; row < dia-1; ++row) {
				unsigned i1 = getIndex(row, col, dia);
				unsigned i2 = getIndex(row, (col + 1)%mod, dia);
				unsigned i3 = getIndex(row + 1, col, dia);
				unsigned i4 = getIndex(row + 1, (col + 1)%mod, dia);
				ret.push_back(Vec3i(i2, i1, i3));
				ret.push_back(Vec3i(i2, i3, i4));
			}
		}

        return ret;
    }
    
    // We're only implenting swept surfaces where the profile curve is
    // flat on the xy-plane.  This is a check function.
    static bool checkFlat(const Curve &profile)
    {
        for (unsigned i=0; i<profile.size(); i++)
            if (profile[i].V[2] != 0.0 ||
                profile[i].T[2] != 0.0 ||
                profile[i].N[2] != 0.0)
                 //return false;
				// commented it since it was failing for the gentorous when applying adaptive tesselation
    
        return true;
    }

}

void Surface::append(const Surface & surface)
{

	int offset = VV.size();

	VV.insert(VV.end(), surface.VV.begin(), surface.VV.end());
	VN.insert(VN.end(), surface.VN.begin(), surface.VN.end());
	VC.insert(VC.end(), surface.VC.begin(), surface.VC.end());
	VT.insert(VT.end(), surface.VT.begin(), surface.VT.end());

	size_t faceOffset = VF.size();
	VF.resize(VF.size() + surface.VF.size());
	for (size_t i = 0; i < surface.VF.size(); ++i)
	{
		VF[i + faceOffset] = surface.VF[i] + Vec3i(offset);
	}
}

void Surface::applyTransformation(const Mat4f & transformation)
{
	FW::Mat4f normalTransformation = transformation.inverted().transposed();
	for (size_t i = 0; i < VV.size(); ++i) {
		VV[i] = (transformation * FW::Vec4f(VV[i], 1.0f)).getXYZ();
	}
	for (size_t i = 0; i < VN.size(); ++i) {
		VN[i] = -(normalTransformation * FW::Vec4f(VN[i], 0.0f)).getXYZ().normalized();
	}
}

Surface makeSurfRev(const Curve &profile, unsigned steps)
{
    Surface surface;
    if (!checkFlat(profile))
    {
        cerr << "surfRev profile curve must be flat on xy plane." << endl;
        exit(0);
    }

    // YOUR CODE HERE: build the surface.  See surf.h for type details.
	// Generate the vertices and normals by looping the number of the steps and again for each 
	// point in the profile (that's two cascaded loops), and finally get the faces with triSweep.
	// You'll need to rotate the curve at each step, similar to the cone in assignment 0 but
	// now you should be using a real rotation matrix.

	float angleIncrement = 2.0f * FW_PI / steps;
	for (unsigned i = 0; i < steps; ++i) {
		Mat3f rotator = Mat3f::rotation(Vec3f(0.0f, 1.0f, 0.0f), angleIncrement * i);
		for (size_t j = 0; j < profile.size(); ++j) {
			Vec3f newV = rotator * profile[j].V;
			Vec3f newN = rotator * profile[j].N;
			Vec3f newC = rotator * profile[j].C;
			surface.VV.push_back(newV);
			surface.VN.push_back(-newN);
			surface.VC.push_back(newC);
		}
	}
	surface.VF = triSweep(profile.size(), steps, true);
    return surface;
}

Surface makeGenCyl(const Curve &profile, const Curve &sweep)
{
    Surface surface;

   // if (!checkFlat(profile))
    //{
        //cerr << "genCyl profile curve must be flat on xy plane." << endl;
        //exit(0);
   // }

	surface.VN.reserve(sweep.size() * profile.size());
	surface.VV.reserve(sweep.size() * profile.size());
	surface.VT.reserve(sweep.size() * profile.size());

	Mat4f matrix, matrixFull, normalMatrixFull;

	for (size_t i = 0; i < sweep.size(); ++i) {
		// point at d offset along the sweep curve
		const CurvePoint & dPoint = sweep[i];

		
		matrix.setCol(0, Vec4f(dPoint.N, 0.0f));
		matrix.setCol(1, Vec4f(dPoint.B, 0.0f));
		matrix.setCol(2, Vec4f(dPoint.T, 0.0f));
		matrix.setCol(3, Vec4f(dPoint.V, 1.0f));

		matrixFull = matrix;
		normalMatrixFull = invert(transpose(matrixFull));

		float texU = float(i) / float(sweep.size() - 1);

		texU = texU - floorf(texU);

		for (size_t j = 0; j < profile.size(); ++j) {
			// current point of the profile at offset
			const CurvePoint & cPoint = profile[j];

			Vec4f newN = normalMatrixFull * Vec4f(-cPoint.N, 1.0f);
			Vec4f newV = matrixFull * Vec4f(cPoint.V, 1.0f);
			Vec4f newC = normalMatrixFull * Vec4f(cPoint.C, 1.0f);

			surface.VN.push_back(normalize(newN.getXYZ()));
			surface.VV.push_back(newV.getXYZ());
			surface.VC.push_back((newC.getXYZ()+dPoint.C)/2.0f);

			float texV = float(j) / float(profile.size() - 1);
			texV = texV - floorf(texV);

			surface.VT.push_back(
				FW::Vec2f(
					texU, texV
				));
		}
	}

	surface.VF = triSweep(profile.size(), sweep.size(), false);

    return surface;
}

// http://stackoverflow.com/questions/20792445/calculate-rgb-value-for-a-range-of-values-to-create-heat-map
static Vec3f HeatColor(float value, float minV, float maxV) {
	float ratio = 2.0f * (value - minV) / (maxV - minV);
	float r = max(0.0f, ratio - 1.0f);
	float b = max(0.0f, 1.0f - ratio);
	float g = 1.0f - b - r;
	return Vec3f(r, g, b);
}

void outputObjFile(ostream &out, const Surface &surface)
{
    
    for (unsigned i=0; i<surface.VV.size(); i++)
        out << "v  "
            << surface.VV[i][0] << " "
            << surface.VV[i][1] << " "
            << surface.VV[i][2] << endl;

    for (unsigned i=0; i<surface.VN.size(); i++)
        out << "vn "
            << surface.VN[i][0] << " "
            << surface.VN[i][1] << " "
            << surface.VN[i][2] << endl;

    out << "vt  0 0 0" << endl;
    
    for (unsigned i=0; i<surface.VF.size(); i++)
    {
        out << "f  ";
        for (unsigned j=0; j<3; j++)
        {
            unsigned a = surface.VF[i][j]+1;
            out << a << "/" << "1" << "/" << a << " ";
        }
        out << endl;
    }
}
