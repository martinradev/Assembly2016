
uniform vec4 knob0;
uniform vec4 knob1;
uniform vec4 knob2;
uniform vec4 knob3;

float fHeart(in vec3 p) {
	float a = pow(p.x*p.x + 2.25 * p.y*p.y +p.z*p.z - 1.0, 3.0);
	float b = p.x*p.x *p.z*p.z*p.z;
	float c = p.z*p.z*p.z * p.y*p.y * 9.0/80.0;
	return a -b-c;
}

float scene1(in vec3 p)
{
	
	float cost = 0.0;
	
	vec3 pCopy = p;

	pModPolar(p.xy, knob0.x);
	
	p.x = -abs(p.x) + knob0.y;
	p.y = -abs(p.y) + knob0.y;
	
	float f1 = fTorus(p, 2.0, 12.0);
	
	p = pCopy;
	pModPolar(p.xz, knob1.x);
	
	pModPolar(p.yx, knob1.x);
	
	float f2 = fOctahedron(p, knob1.y, 1.0);
	
	float f3 = fOpUnionChamfer(f1,f2,0.0);
	
	
	float f4 = fCapsule(p, knob2.x, knob1.z);
	float f5 = fCapsule(p, 0.3*knob2.x, 1.3*knob1.z);
	
	f3 = fOpDifferenceChamfer(f3, f4, 1.0);
	f3 = fOpUnionChamfer(f3,f5, 0.0);
	
	if (knob2.y > 0.0)
	{
		p = pCopy;
		pR(p.xz, knob2.w);
		pR(p.yz, knob2.w);
		f4 = fTorus(p, 2.0, knob2.y);
		f3 = fOpUnionChamfer(f3,f4,1.0);
		f4 = fTorus(p.yxz, 2.0, knob2.y);
		f3 = fOpUnionChamfer(f3,f4,1.0);
	}
	
	if (knob2.z > 0.0)
	{
		p = pCopy;
		f4 = fTorus(p, knob2.z, 16.0);
		f3 = fOpDifferenceStairs(f3,f4,0.0, 1.0);
		f4 = fTorus(p.yxz, knob2.z, 16.0);
		f3 = fOpDifferenceStairs(f3,f4,0.0, 1.0);
	}
	
	cost = f3;
	
	return cost;
	
}

float scene2(in vec3 p)
{
	
	float cost = 0.0;
	vec3 pCopy = p;
	p.x += knob0.x*sin(pCopy.y+knob1.x);
	p.y += knob0.x*sin(pCopy.z+knob1.y);
	p.z += knob0.x*sin(pCopy.x+knob1.z);
	float f1 = fSphere(p, knob0.y);
	float f2 = fSphere(p+vec3(knob0.z), knob0.y);
	f1 = fOpPipe(f1,f2,knob0.w);
	
	p = pCopy;
	pModPolar(p.xz, knob2.w);
	pModPolar(p.yx, knob3.x);
	
	p.x += knob0.x*sin(0.5*(pCopy.y+knob1.x));
	p.z += knob0.x*sin(0.5*(pCopy.z+knob1.z));
	float f3 = fCone(p, knob2.x, knob2.y);
	f1 = fOpUnionChamfer(f1,f3,0.0);
	
	f3 = fHeart(knob2.z*pCopy);
	f1 = fOpUnionChamfer(f1,f3,1.5);
	
	cost = f1; 
	
	return cost;
}

float scene3(in vec3 p)
{
	return 0.0;
}

float fScene(in vec3 p) {

	float result=0;

	if (knob1.w >= 1.5)
	{
		result = scene3(p);
	}
	else if (knob1.w >= 0.5)
	{
		result = scene2(p);
	}
	else
	{
		result = scene1(p);
	}
	
	
	return result;
}

vec3 getNormal(in vec3 p) {
	
	const vec3 NEPS = vec3(2.0, 0.0, 0.0);
	
	vec3 delta = vec3(
					fScene(p + NEPS) - fScene(p - NEPS),
					fScene(p + NEPS.yxz) - fScene(p - NEPS.yxz),
					fScene(p + NEPS.yzx) - fScene(p - NEPS.yzx)
				);
	
	return normalize(delta);
	
}

vec3 getTrigNormal(in vec3 p1, in vec3 p2, in vec3 p3) {
	return normalize(cross(p3-p1,p2-1));
}