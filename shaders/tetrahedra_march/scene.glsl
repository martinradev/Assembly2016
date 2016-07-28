
uniform vec4 sync1;
uniform vec4 sync2;
uniform vec4 sync3;
uniform vec4 sync4;

float fHeart(in vec3 p) {
	float a = pow(p.x*p.x + 2.25 * p.y*p.y +p.z*p.z - 1.0, 3.0);
	float b = p.x*p.x *p.z*p.z*p.z;
	float c = p.z*p.z*p.z * p.y*p.y * 9.0/80.0;
	return a -b-c;
}

float getPlanet(in vec3 p) {
	

	p.yz -= vec2(sync1.x);
	
	float tNoise1 = hashNoise3D(p*2)*sync1.y;
	
	float sph1 = fSphere(p, sync1.z)+tNoise1;
	
	return sph1;
}



float getTeeth(in vec3 p, in float r, float idx) {
	
	float cost = 0;
	
	pModPolar(p.xz, 12);
	
	p.z += abs(sin(idx));
	p.x = -abs(p.x) + r;
	
	
	cost = fCone(-p.zxy, 0.25, 0.6);
	
	return cost;
}

float getChain(in vec3 p) {

	float cost = 0;
	float idx = pModPolar(p.xz, 6);
	
	p.x = -abs(p.x) + 6;
	
	pR(p.xy, sync1.x);
	
	cost = fTorus(p, 0.6, 2.25);
	
	float f = getTeeth(p, 2.85, idx);
	
	cost = fOpUnionChamfer(cost, f, 0.2);
	
	return cost;
}

float getChain2(in vec3 p) {

	vec3 off = vec3(0,0,0);
	float cost = 0;
	pR(p.xz, 0.55);
	float idx = pModPolar(p.xz, 6);

	p.x = -abs(p.x) + 5.5;
	
	pR(p.xy, sync1.x);
	
	cost = fTorus(p.zxy, 0.6, 2.25);
	
	float f = getTeeth(p.zxy, 2.85, idx);
	
	cost = fOpUnionChamfer(cost, f, 0.2);
	
	return cost;
}


float scene1(in vec3 p) {

	float result; 
	
	float sph1 = getPlanet(p-vec3(0.0,sync1.x,sync1.x));
	float sph2 = getPlanet(p+vec3(sync1.x,0.0,sync1.x));
	
	result = fOpUnionChamfer(sph1,sph2,0.2);
	
	return result;
}

float scene2(in vec3 p) {
	
	float result; 
	
	vec3 pcpy = p;
	

	pR(pcpy.xz, sync2.y);
	pR(pcpy.xy, sync2.z);
	
	result = fBox(pcpy, vec3(6*sync3.x));
	
	if (sync1.w > 0) {
		pMod2(pcpy.xz, vec2(sync1.w));
		float remBox = fBox(pcpy, vec3(0.8, 30, 0.8));
		result = fOpDifferenceColumns(result, remBox, 1, 2);
	}
	
	return result;
}

float scene3(in vec3 p){
	float result = 0;

	result = fHeart(p.xzy*sync3.x);
	
	if (sync2.w > 1.5) {
		
		pR(p.yx, 0.9);
		p.x -= sync1.x;
		float fv = fBox(p, vec3(1,20,4));
		
		result = fOpDifferenceChamfer(result, fv, 0.7);

	}
	
	return result;
}

float scene4(in vec3 p) {
	float result = 0;
	
	pR(p.xz, sync1.y);
	pR(p.yz, sync1.z);
	
	pModPolar(p.xz, sync2.z);
	pModPolar(p.yx, sync2.z);

	result = fCapsule(p, 0.5, sync2.y);
	
	
	return result;
}

float scene5(in vec3 p) {
	float result = 0;
	
	float f1 = getChain(p);
	
	float f2 = getChain2(p);
	
	result = fOpUnionChamfer(f1,f2,0);
	
	return result;
}

float scene6(in vec3 p) {
	float bigr = 3.0;
	float minr = bigr - 0.5;
	
	float t = sync1.w;
	if (t > 0) {
		pModPolar(p.xz, t);
		minr -= 0.01*t;
	}
	
	pR(p.xz, sync1.y);
	pR(p.yz, sync1.z);
	
	vec3 pcpy = p;
	float f1 = fCylinder(pcpy, bigr, 0.5);
	f1 = fOpDifferenceChamfer(f1, fCylinder(pcpy, minr, 0.6), 0.0);
	
	pcpy.xz += vec2(sync2.z, 0.0);
	float f2 = fCylinder(pcpy, bigr, 0.5);
	f2 = fOpDifferenceChamfer(f2, fCylinder(pcpy, minr, 0.6), 0.0);
	
	pcpy.xz += vec2(-sync2.z,sync2.z);
	float f3 = fCylinder(pcpy, bigr, 0.5);
	f3 = fOpDifferenceChamfer(f3, fCylinder(pcpy, minr, 0.6), 0.0);
	
	float cost = fOpUnionChamfer(f1,f2,0);
	cost = fOpUnionChamfer(f3,cost,0);
	return cost;
}

float scene7(in vec3 p) {
	float cost = 0;
	float myScale = sync3.z;
	pR(p.xy, sync4.w);
	pR(p.yz, sync3.w);
	pModPolar(p.yz, sync4.x);
	
	//p.x = -abs(p.x) + p.y;
	p.z = -abs(p.z) + p.y;
	float f1 = fBox(p, vec3(2,4,2)*myScale);
	
	pR(p.xz, sync4.y);
	pR(p.yx, sync4.z);
	float f2 = fCylinder(p,2.0*myScale, 3.0*myScale);
	
	//cost = fOpUnionSoft(f1,f2,0.1);
	cost = fOpPipe(f1,f2,0.5);
	
	return cost;
}

float scene8(in vec3 p) {
	float cost = 0;
	float myScale = sync3.z;
	pR(p.xy, sync4.w);
	pR(p.yz, sync3.w);
	pModPolar(p.yz, sync4.x);
	
	//p.x = -abs(p.x) + p.y;
	p.z = -abs(p.z) + p.y;
	float f1 = fBox(p, vec3(2,4,2)*myScale);
	
	pR(p.xz, sync4.y);
	pR(p.yx, sync4.z);
	float f2 = fCylinder(p,2.0*myScale, 3.0*myScale);
	
	//cost = fOpUnionSoft(f1,f2,0.1);
	cost = fOpPipe(f1,f2,0.5);
	
	return cost;
}

float fScene(in vec3 p) {


	float result=0;
	
	if (sync2.w >= 4.6) {
		return scene7(p);
	}
	else if (sync2.w >= 4.6) {
		float f1 = scene7(p);
		float f2 = scene6(p);
		result = fOpUnionChamfer(f1, f2, sync2.x);
	}
	else if (sync2.w >= 4) {
		return scene6(p);
	}
	else if (sync2.w >= 3.6) {
		float f1 = scene6(p);
		float f2 = scene5(p);
		result = fOpUnionChamfer(f1, f2, sync2.x);
	}
	else if (sync2.w >= 3) {
		return scene5(p);
	}else if (sync2.w > 2.6) {
		float f1 = scene5(p);
		float f2 = scene4(p);
		result = fOpUnionChamfer(f1, f2, sync2.x);
	} 
	else if (sync2.w > 2) {
		return scene4(p);
	} else if (sync2.w >= 1.9) {
		float f2 = scene4(p);
		float f1 = scene3(p);
		result = fOpUnionChamfer(f1, f2, sync2.x);
	}
	else if (sync2.w >= 1.1) {
		return scene3(p);
	}
	else if (sync2.w > 1.001) {
		// transition from scene 2 to scene 3
		float f2 = scene3(p);
		float f1 = scene2(p);
		result = fOpUnionChamfer(f1, f2, sync2.x);
	}
	else if (sync2.w >= 1) {
		// scene 2
		result = scene2(p);
		
	} else if (sync2.w > 0) {
		// transition from scene 1 to scene 2
		float f1 = scene1(p);
		float f2 = scene2(p);
		
		result = fOpUnionChamfer(f1, f2, sync2.x);
		
	} else {
		// scene 1
		
		result = scene1(p);
	}
	
	
	//result = fBlob(p);
	
	return result;
}

vec3 getNormal(in vec3 p) {
	
	const vec3 NEPS = vec3(0.0001, 0.0, 0.0);
	
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