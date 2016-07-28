#version 430

#define EPS 0.0001

// -------------------------------------------------------------------

/* BEGIN SCENE DECLARATIONS */

float fScene(in vec3 p);

vec3 getNormal(in vec3 p) {
	
	vec3 NEPS = vec3(0.0001, 0.0, 0.0);
	
	vec3 delta = vec3(
					fScene(p + NEPS) - fScene(p - NEPS),
					fScene(p + NEPS.yxz) - fScene(p - NEPS.yxz),
					fScene(p + NEPS.yzx) - fScene(p - NEPS.yzx)
				);
	
	return normalize(delta);
	
}

/* END SCENE DECLARATIONS */

// -------------------------------------------------------------------

/* BEGIN NOISE FUNCTIONS */

float noise3D(in vec3 p);
float noise2D(in vec2 p);
float noise1D(in float p);

/* END NOISE FUNCTIONS */

// -------------------------------------------------------------------

/* BEGIN SDF FUNCTIONS FROM HG SDF */

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

/* END SDF FUNCTIONS FROM HG SDF */


// -------------------------------------------------------------------