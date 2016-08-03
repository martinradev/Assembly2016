
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
	float cost = 0;
	
	pR(p.xy, knob0.w);
	pR(p.yz, knob1.x);
	
	pModPolar(p.xy, knob0.z);
	
	float f1 = fTorus(p, knob0.y, knob0.x);
	
	cost = f1;
	
	return cost;
}

float scene2(in vec3 p)
{
	vec3 pCopy = p;
	float cost = 0;
	
	pR(p.xz, knob0.w);
	pR(p.yz, knob1.x);
	
	pModPolar(p.xz, knob0.z);
	pModPolar(p.yx, knob0.z);
	
	float f1 = fCapsule(p, 1.0, knob0.x);
	
	float f2 = fSphere(p, knob0.x);
	float f3 = fCapsule(p, 2.5, knob0.x);
	f2 = fOpDifferenceRound(f2,f3,0.5);
	
	f2 = fOpUnionChamfer(f1,f2, 0.0);
	
	float fCube = fBox(p, vec3(knob0.y));
	
	fCube = fOpDifferenceRound(fCube, f3, 0.5);
	fCube = fOpUnionChamfer(f2,fCube, 0.2);
	
	float cubeIn = fBox(p, vec3(knob1.y)*vec3(0.3,1.5,0.3));
	cubeIn = fOpUnionChamfer(cubeIn, fBox(p, vec3(knob1.y)*vec3(0.3,0.3,1.5)), 0.0);
	cubeIn = fOpUnionChamfer(cubeIn, fBox(p, vec3(knob1.y)*vec3(1.5,0.3,0.3)), 0.0);
	fCube = fOpDifferenceRound(fCube, cubeIn, 1.0);
	
	
	cost = fCube;
	
	return cost;
	
}

float scene3(in vec3 p)
{
	
	pR(p.xz, knob0.w);
	pR(p.yz, knob1.x);
	vec3 pCopy = p;
	vec3 pCopy2 = p;
	pR(p.xz, p.y*knob2.x);
	
	p.y = -abs(p.y)+knob2.y;
	pModPolar(p.xz, knob0.z);
	pModPolar(p.yx, knob0.z);
	
	
	
	float cost = 0.0;
	
	float fCube = fBox(p, vec3(knob0.y));
	
	float cubeIn = fBox(p, vec3(knob1.y)*vec3(0.3,1.5,0.3));
	cubeIn = fOpUnionChamfer(cubeIn, fBox(p, vec3(knob1.y)*vec3(0.3,0.3,1.5)), 0.0);
	cubeIn = fOpUnionChamfer(cubeIn, fBox(p, vec3(knob1.y)*vec3(1.5,0.3,0.3)), 0.0);
	fCube = fOpDifferenceRound(fCube, cubeIn, 1.0);

	//pR(pCopy.xz, 0.05*pCopy.y);
	//pR(pCopy.xy, 0.05*pCopy.z);

	pModPolar(pCopy.yx, knob0.z);
	
	
	float f3 = fCapsule(pCopy, knob2.w*(3.0+1.1*abs(sin(pCopy.y*knob2.z))), knob1.z);
	
	cost = fOpUnionChamfer(fCube, f3, 1.0);
	

	cost += 1.5*hashNoise3D(0.5*(p +vec3(knob0.x)));
	
	return cost;
	
}

float fScene(in vec3 p) {

	float result=0;
	p *= knob3.x;
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