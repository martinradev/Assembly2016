#version 430

float noise3D(vec3 p);
float tetraVoronoi(in vec3 p);

#define PI 3.14159265
#define TAU (2*PI)
#define PHI (sqrt(5)*0.5 + 0.5)
#define saturate(x) clamp(x, 0, 1)
float sgn(float x);
vec2 sgn(vec2 v);
float square (float x);
vec2 square (vec2 x);
vec3 square (vec3 x);
float lengthSqr(vec3 x);
float vmax(vec2 v);
float vmax(vec3 v);
float vmax(vec4 v);
float vmin(vec2 v) ;
float vmin(vec3 v) ;
float vmin(vec4 v);



float fSphere(vec3 p, float r);

// Plane with normal n (n is normalized) at some distance from the origin
float fPlane(vec3 p, vec3 n, float distanceFromOrigin);

// Cheap Box: distance to corners is overestimated
float fBoxCheap(vec3 p, vec3 b);

// Box: correct distance to corners
float fBox(vec3 p, vec3 b);

// Same as above, but in two dimensions (an endless box)
float fBox2Cheap(vec2 p, vec2 b);

float fBox2(vec2 p, vec2 b);


// Endless "corner"
float fCorner (vec2 p);

// Blobby ball object. You've probably seen it somewhere. This is not a correct distance bound, beware.
float fBlob(vec3 p);

// Cylinder standing upright on the xz plane
float fCylinder(vec3 p, float r, float height);

// Capsule: A Cylinder with round caps on both sides
float fCapsule(vec3 p, float r, float c);

// Distance to line segment between <a> and <b>, used for fCapsule() version 2below
float fLineSegment(vec3 p, vec3 a, vec3 b);

// Capsule version 2: between two end points <a> and <b> with radius r 
float fCapsule(vec3 p, vec3 a, vec3 b, float r);

// Torus in the XZ-plane
float fTorus(vec3 p, float smallRadius, float largeRadius);
// A circle line. Can also be used to make a torus by subtracting the smaller radius of the torus.
float fCircle(vec3 p, float r);

// A circular disc with no thickness (i.e. a cylinder with no height).
// Subtract some value to make a flat disc with rounded edge.
float fDisc(vec3 p, float r);

// Hexagonal prism, circumcircle variant
float fHexagonCircumcircle(vec3 p, vec2 h);

// Hexagonal prism, incircle variant
float fHexagonIncircle(vec3 p, vec2 h);

// Cone with correct distances to tip and base circle. Y is up, 0 is in the middle of the base.
float fCone(vec3 p, float radius, float height);


// Version with variable exponent.
// This is slow and does not produce correct distances, but allows for bulging of objects.
float fGDF(vec3 p, float r, float e, int begin, int end);

// Version with without exponent, creates objects with sharp edges and flat faces
float fGDF(vec3 p, float r, int begin, int end);

// Primitives follow:

float fOctahedron(vec3 p, float r, float e);

float fDodecahedron(vec3 p, float r, float e);

float fIcosahedron(vec3 p, float r, float e);

float fTruncatedOctahedron(vec3 p, float r, float e);

float fTruncatedIcosahedron(vec3 p, float r, float e);


float fOctahedron(vec3 p, float r);

float fDodecahedron(vec3 p, float r);

float fIcosahedron(vec3 p, float r);

float fTruncatedOctahedron(vec3 p, float r);

float fTruncatedIcosahedron(vec3 p, float r);


// Rotate around a coordinate axis (i.e. in a plane perpendicular to that axis) by angle <a>.
// Read like this: R(p.xz, a) rotates "x towards z".
// This is fast if <a> is a compile-time constant and slower (but still practical) if not.
void pR(inout vec2 p, float a);

// Shortcut for 45-degrees rotation
void pR45(inout vec2 p);

// Repeat space along one axis. Use like this to repeat along the x axis:
// <float cell = pMod1(p.x,5);> - using the return value is optional.
float pMod1(inout float p, float size);

// Same, but mirror every second cell so they match at the boundaries
float pModMirror1(inout float p, float size);

// Repeat the domain only in positive direction. Everything in the negative half-space is unchanged.
float pModSingle1(inout float p, float size);

// Repeat only a few times: from indices <start> to <stop> (similar to above, but more flexible)
float pModInterval1(inout float p, float size, float start, float stop);


// Repeat around the origin by a fixed angle.
// For easier use, num of repetitions is use to specify the angle.
float pModPolar(inout vec2 p, float repetitions);

// Repeat in two dimensions
vec2 pMod2(inout vec2 p, vec2 size);

// Same, but mirror every second cell so all boundaries match
vec2 pModMirror2(inout vec2 p, vec2 size);


// Same, but mirror every second cell at the diagonal as well
vec2 pModGrid2(inout vec2 p, vec2 size);

// Repeat in three dimensions
vec3 pMod3(inout vec3 p, vec3 size);

// Mirror at an axis-aligned plane which is at a specified distance <dist> from the origin.
float pMirror (inout float p, float dist);

// Mirror in both dimensions and at the diagonal, yielding one eighth of the space.
// translate by dist before mirroring.
vec2 pMirrorOctant (inout vec2 p, vec2 dist);

// Reflect space at a plane
float pReflect(inout vec3 p, vec3 planeNormal, float offset);


// The "Chamfer" flavour makes a 45-degree chamfered edge (the diagonal of a square of size <r>):
float fOpUnionChamfer(float a, float b, float r);

// Intersection has to deal with what is normally the inside of the resulting object
// when using union, which we normally don't care about too much. Thus, intersection
// implementations sometimes differ from union implementations.
float fOpIntersectionChamfer(float a, float b, float r);

// Difference can be built from Intersection or Union:
float fOpDifferenceChamfer (float a, float b, float r);

// The "Round" variant uses a quarter-circle to join the two objects smoothly:
float fOpUnionRound(float a, float b, float r);

float fOpIntersectionRound(float a, float b, float r);

float fOpDifferenceRound (float a, float b, float r);


// The "Columns" flavour makes n-1 circular columns at a 45 degree angle:
float fOpUnionColumns(float a, float b, float r, float n);

float fOpDifferenceColumns(float a, float b, float r, float n);

float fOpIntersectionColumns(float a, float b, float r, float n);

// The "Stairs" flavour produces n-1 steps of a staircase:
// much less stupid version by paniq
float fOpUnionStairs(float a, float b, float r, float n);

// We can just call Union since stairs are symmetric.
float fOpIntersectionStairs(float a, float b, float r, float n);

float fOpDifferenceStairs(float a, float b, float r, float n);


// Similar to fOpUnionRound, but more lipschitz-y at acute angles
// (and less so at 90 degrees). Useful when fudging around too much
// by MediaMolecule, from Alex Evans' siggraph slides
float fOpUnionSoft(float a, float b, float r);


// produces a cylindical pipe that runs along the intersection.
// No objects remain, only the pipe. This is not a boolean operator.
float fOpPipe(float a, float b, float r);

// first object gets a v-shaped engraving where it intersect the second
float fOpEngrave(float a, float b, float r);

// first object gets a capenter-style groove cut out
float fOpGroove(float a, float b, float ra, float rb);

// first object gets a capenter-style tongue attached
float fOpTongue(float a, float b, float ra, float rb);

float fSmallBuilding(in vec3 p) {
	pModPolar(p.xz, 6);
	p.x = -abs(p.x) + 10;
	pMod1(p.z, 20);
	float wall = fBox2(p.xy, vec2(1.0, 8.0));
	p.y += 5;
	float box = fBox(p.xyz, vec3(3, 7, 3.5));
	p.y -= 6.86;
	float cyllinder = fCylinder(p.zxy, 3.5, 3);
	float window = min(box, cyllinder);
	float facade = fOpDifferenceColumns(wall, window, 0.7,3);
	p.y -= 7;
	pR(p.xy, 0.4);
	p.x -= 5;
	float roof = fBox2(p.xy, vec2(10,1));
	float house = min(roof, facade);
	return house;
}

float fTrig(in vec3 p) {
	pModPolar(p.xz, 3);
	return fBox2(p.xy, vec2(0.71));
}

float fStar(in vec3 p) {
	
	float trig1 = fTrig(p);
	
	pR(p.xz, 1.0);
	float trig2 = fTrig(p);
	
	return min(trig1, trig2);
}

float fUFOBase(in vec3 p) {
	vec3 pCol = p;
	float l = 10;
	float h = 5;
	float base = fBox(pCol, vec3(l, h, l));
	pR(pCol.zx, 2.09);
	pCol.x += l * 0.8387;
	base = min(base, fBox(pCol, vec3(l, h, l)));
	
	pCol = p;
	pR(pCol.zx, -2.09);
	pCol.x += l * 0.8387;
	base = min(base, fBox(pCol, vec3(l, h, l)));
	
	float baseSphere = fSphere(p + vec3(13.387, 0, 22.025), l*2);
	base = fOpDifferenceChamfer(base, baseSphere, 0.0);
	
	
	baseSphere = fSphere(p + vec3(-26.87, 0, -2.0769), l*2);
	base = fOpDifferenceChamfer(base, baseSphere, 0.0);
	
	baseSphere = fSphere(p + vec3(15.451, 0, -24.641), l*2);
	base = fOpDifferenceChamfer(base, baseSphere, 0.0);
	
	float baseExt = fBox(p + vec3(16.483871,0.0,1.0-0.2249 - 2.28), vec3(15.8117, h, 4.05));
	
	base = min(base, baseExt);
	
	return base;
}

float fUFOColumnAndStar(in vec3 p) {
	// generate column with star
	float column = fBox(p + vec3(8.27, -5.7, 0.0), vec3(1,8,2.2));
	vec3 pCol = p;
	pR(pCol.yz, 0.045);
	float columnSub1 = fBox(pCol + vec3(7.7532, -3.6451, -2.63637), vec3(2,20,1.6));
	pCol = p;
	pR(pCol.yz, -0.045);
	float columnSub2 = fBox(pCol + vec3(7.7532, -3.6451, 3.0779), vec3(2,20,1.6));
	column = fOpDifferenceChamfer(column, columnSub1,0.0);
	column = fOpDifferenceChamfer(column, columnSub2,0.0);
	return column;
}

float fUFOStar(in vec3 p) {
	return fStar(p.yxz + vec3(-10.3548, 7.837, 0.3305));
}

float fStairs(vec3 p) {
	
	return 1;
	
}

float fDoorSub(in vec3 p) {

	return fBox(p - vec3(2.5483, -3.9645, 0.4), vec3(1,0.75,1));

}

float fDoor(in vec3 p) {

	return fBox(p - vec3(2.5483 - 0.9, -3.9345, 0.4), vec3(0.2,0.75,1));

}

float columnNoise(in vec3 p) {
	
	float fbm = 0.0;
	
	fbm += 0.04*noise3D(p);
	fbm += 0.02*noise3D(p*0.2);
	fbm += 0.01*noise3D(p*0.4);
	
	return fbm;
}

float fScene(in vec3 p) {

	// column
	float column = fUFOColumnAndStar(p + vec3(0.0, 0, -0.64));
	float columnStar = fUFOStar(p + vec3(0, 0, -0.64));
	
	
	// base
	float baseScale = 3.48387;
	float base = fUFOBase(p * baseScale + vec3(0, 11.3225, 0)) / baseScale;
	
	// door
	float door = fDoor(p);
	float doorSub = fDoorSub(p);
	
	// construct hull
	float sphere = fSphere(p, 6.0);
	float sphereFloor = fSphere(p, 5.9);
	float sphereIn = fSphere(p, 5.2);
	
	sphere = fOpDifferenceChamfer(sphere, sphereIn, 0.0);
	
		
	// create top
	float topCone = fSphere(p + vec3(0, 23.6055 , 0.0), 25.0);
	topCone = fOpIntersectionChamfer(topCone, fSphere(p + vec3(0, 22.35, 0.0), 23.8), 0.0);
	float topConeBox = fBox(p + vec3(0, 23.6055 + 0.84, 0.0), vec3(25));
	topCone = fOpDifferenceChamfer(topCone, topConeBox, 0.0);
	
	// remove bottom part of the sphere hull
	// 6.67
	p.y -= 6.67;
	float topBox = fBox(p, vec3(6.0));
	// -14.84
	p.y += 14.84;
	float botBox = fBox(p, vec3(6.0));
	
	float ufo = fOpDifferenceChamfer(sphere, topBox, 0.0);
	ufo = fOpDifferenceRound(ufo, botBox, 0);
	
	// add some detail to the ufo
	

	// generate ufo floor
	topBox = -fBox(p - vec3(0,0.7,0), vec3(6.0));
	botBox = fBox(p, vec3(6.0));
	sphereFloor = fOpDifferenceChamfer(sphereFloor, botBox, 0);
	sphereFloor = fOpDifferenceChamfer(sphereFloor, topBox, 0);
	
	
	// generate windows
	p.y -= 8.25;
	pModPolar(p.xz, 10);
	p.x = -abs(p.x) + 4.37;
	pMod1(p.z, 20);
	float wndBox = fBox(p, vec3(2,0.6,1.5));
	float wndCyl = fSphere(p - vec3(0,8.35,0), 9.0);
	float windows = fOpIntersectionChamfer(wndBox, wndCyl, 0.0);
	
	
	ufo = fOpDifferenceRound(ufo, windows, 0.05);
	base = fOpDifferenceChamfer(base, doorSub, 0.0);
	
	//foundPart = HULL;

	if (ufo > topCone) {
		ufo = topCone;
		//foundPart = ROOF;
	}
	
	if (ufo > sphereFloor) {
		ufo = sphereFloor;
		//foundPart = FLOOR;
	}
	
	if (ufo > column) {
		ufo = column;
		//foundPart = COLUMN;
	}
	
	if (ufo > base) {
		ufo = base;
		//foundPart = BASE;
	}
	
	if (ufo > columnStar) {
		ufo = columnStar;
		//foundPart = STAR;
	}
	
	if (ufo > door) {
		ufo = door;
		//foundPart = DOOR;
	}
	
	return ufo;
	
}