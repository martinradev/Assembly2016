#include "MarchingTetrahedra.h"

#include "Util.h"
#include "DistanceField.h"

using namespace FW;

void MarchingTetrahedra::generateTetrahedra(const Vec3f & from, const float cubeLen, std::array<Tetrahedra, 6> & tetrahedras) {
	
	const unsigned tetraIndices[][4] {
		{ 0, 1, 5, 6 },
		{ 0, 5, 6, 4 },
		{ 0, 1, 6, 2 },
		{ 0, 4, 6 ,7 },
		{ 0, 3, 7, 6 },
		{ 0, 3, 2, 6}
	};

	std::array<Vec3f, 8> cubeVertices = {
		from, 
		from + Vec3f(cubeLen, 0.0f, 0.0f),
		from + Vec3f(cubeLen, 0.0f, cubeLen),
		from + Vec3f(0.0f, 0.0f, cubeLen),
		from + Vec3f(0.0f, cubeLen, 0.0f),
		from + Vec3f(cubeLen, cubeLen, 0.0f),
		from + Vec3f(cubeLen, cubeLen, cubeLen),
		from + Vec3f(0.0f, cubeLen, cubeLen),
	};

	for (int i = 0; i < 6; ++i) {
		tetrahedras[i] = Tetrahedra(
			cubeVertices[tetraIndices[i][0]],
			cubeVertices[tetraIndices[i][1]],
			cubeVertices[tetraIndices[i][2]],
			cubeVertices[tetraIndices[i][3]]
			);
	}

}

bool MarchingTetrahedra::intersects(const Vec3f & edgeA, const Vec3f & edgeB, Vec3f & intersectionPoint, const std::function<float(const Vec3f&)> & fieldFunc) {
	
	float f1 = fieldFunc(edgeA);
	float f2 = fieldFunc(edgeB);
	int s1 = sign(f1);
	int s2 = sign(f2);

	Vec3f p0 = edgeA;
	Vec3f p1 = edgeB;
	if (s1 > s2) std::swap(s1, s2), std::swap(p0,p1), std::swap(f1,f2);

	if (s1 < 0 && s2 >= 0) {

		// must intersect

		// binary search
		

		Vec3f step = p1 - p0;
		intersectionPoint = p0;
		for (int i = 0; i < 32; ++i) {
			
			Vec3f mid = intersectionPoint + step;
			const int s3 = sign(fieldFunc(mid));
			if (s3 <= 0) {
				intersectionPoint = mid;
			}
			step /= 2.0f;
		}
		return true;
	}
	return false;
}

static Vec3f getDistanceFieldNormal(const FW::Vec3f & p, std::function<float(const Vec3f&)> & fieldFunc) {
	static const float eps = 0.001f;
	Vec3f n = Vec3f(
		fieldFunc(p + Vec3f(eps, 0.0f, 0.0f)) - fieldFunc(p - Vec3f(eps, 0.0f, 0.0f)),
		fieldFunc(p + Vec3f(0.0f, eps, 0.0f)) - fieldFunc(p - Vec3f(0.0f, eps, 0.0f)),
		fieldFunc(p + Vec3f(0.0f, 0.0f, eps)) - fieldFunc(p - Vec3f(0.0f, 0.0f, eps))
		);
	return n.normalized();
}

Surface MarchingTetrahedra::march(const Vec3f & f, const Vec3f & t, float cubeLen, std::function<float(const Vec3f&)> & fieldFunc) {

	Surface surface;

	Vec3f from = FW::min(f,t);
	Vec3f to = FW::max(f,t);
	Vec3f delta = (to - from) / cubeLen; 
	Vec3i steps = Vec3i((floor(delta.x) + 1.0f), (floor(delta.y) + 1.0f), (floor(delta.z) + 1.0f));
	to.x = from.x + steps.x*cubeLen;
	to.y = from.y + steps.y*cubeLen;
	to.z = from.z + steps.z*cubeLen;


	for (int x = 0; x < steps.x; ++x) {

		for (int y = 0; y < steps.y; ++y) {

			for (int z = 0; z < steps.z; ++z) {

				Vec3f cubeS = from + cubeLen * Vec3f(x,y,z);
				
				std::array<Tetrahedra, 6> tetrahedra;
				generateTetrahedra(cubeS, cubeLen, tetrahedra);

				Vec3f points[6];
				for (int q = 0; q < 6; ++q) {

					int e = 0;

					for (int i = 0; i < 4; ++i) {
						for (int j = i + 1; j < 4; ++j) {
							Vec3f intersectionPoint;
							if (intersects(tetrahedra[q].vertices_[i], tetrahedra[q].vertices_[j], intersectionPoint, fieldFunc)) {
								//std::cout << "intersect" << std::endl;
								points[e++] = intersectionPoint;
								
							}

						}
					}
					if (e == 3) {

						int sz = surface.VV.size();

						surface.VV.push_back(points[0]);
						surface.VN.push_back(getDistanceFieldNormal(points[0], fieldFunc));

						surface.VV.push_back(points[1]);
						surface.VN.push_back(getDistanceFieldNormal(points[1], fieldFunc));

						surface.VV.push_back(points[2]);
						surface.VN.push_back(getDistanceFieldNormal(points[2], fieldFunc));

						
						
						surface.VF.push_back(Vec3i(sz, sz + 1, sz + 2));

					}
					else if (e == 4) {

						int sz = surface.VV.size();

						surface.VV.push_back(points[0]);
						surface.VN.push_back(getDistanceFieldNormal(points[0], fieldFunc));

						surface.VV.push_back(points[2]);
						surface.VN.push_back(getDistanceFieldNormal(points[2], fieldFunc));

						surface.VV.push_back(points[3]);
						surface.VN.push_back(getDistanceFieldNormal(points[3], fieldFunc));

						surface.VF.push_back(Vec3i(sz, sz + 1, sz + 2));
						// check other possibilities

						surface.VV.push_back(points[0]);
						surface.VN.push_back(getDistanceFieldNormal(points[0], fieldFunc));

						surface.VV.push_back(points[3]);
						surface.VN.push_back(getDistanceFieldNormal(points[3], fieldFunc));

						surface.VV.push_back(points[1]);
						surface.VN.push_back(getDistanceFieldNormal(points[1], fieldFunc));

						surface.VF.push_back(Vec3i(sz+3, sz + 4, sz + 5));
					}
					
				}
				


				// generate 6 polyhedra
				// for each polyhedra
				// for each edge
				// find intersection point
				// compute triangles for each polyhedra and push to vertex vector


			}

		}

	}
	
	// empty tex coords
	surface.VT.resize(surface.VV.size());

	return surface;

}
