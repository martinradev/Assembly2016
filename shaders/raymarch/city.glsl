
uniform sampler2D hullSampler;
uniform sampler2D brokenGlass;
uniform sampler2D procTex[10];

uniform vec4 volumeStart; // (x,y,z) -> volume's start, (w) cube length 
uniform vec4 volumeSize; // (x,y,z) -> volume's size, volumeSize, (w) -> 1.0 draw volume, 0.0 not draw volume

#define ROOM 0
#define SIDES 1
#define PIEDISTAL 2

#define VOLUME -1
#define SHPERE_TEST -2

float raymarch2(vec3 p0, vec3 d0, float tmin, float tmax);

int foundPart;

float getTeeth(in vec3 p, in float r, float idx) {
	
	float cost = 0;
	
	pModPolar(p.xz, 12);
	
	p.z += abs(sin(idx+knob1.x));
	p.x = -abs(p.x) + r;
	
	
	cost = fCone(-p.zxy, 0.25, 0.6);
	
	return cost;
}

float getChain(in vec3 p) {

	float cost = 0;
	float idx = pModPolar(p.xz, 6);
	
	p.x = -abs(p.x) + 6;
	
	pR(p.xy, knob5.w);
	
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
	
	pR(p.xy, knob5.w);
	
	cost = fTorus(p.zxy, 0.6, 2.25);
	
	float f = getTeeth(p.zxy, 2.85, idx);
	
	cost = fOpUnionChamfer(cost, f, 0.2);
	
	return cost;
}

float getChain3(in vec3 p) {
	float f1 = getChain(p);
	
	float f2 = getChain2(p);
	
	return fOpUnionChamfer(f1,f2,0);
}

float someTest(in vec3 p) {
	
	float cost = 0;
	pR(p.xz, knob3.z);
	pModPolar(p.yz, knob1.x);
	
	//p.x = -abs(p.x) + p.y;
	p.z = -abs(p.z) + p.y;
	float f1 = fBox(p, vec3(2,4,2));
	
	pR(p.xz, knob3.x);
	pR(p.yx, knob3.y);
	float f2 = fCylinder(p,2.0, 3.0);
	
	cost = fOpUnionSoft(f1,f2,0.1);
	cost = fOpPipe(f1,f2,0.6);
	
	return cost;
}

float fCity(in vec3 p) {
	
	float cost = 0.0;
	
	
	vec2 gridIdx = pMod2(p.xz, vec2(20));
	
	float boxHeight = 12.0 * (abs(noise(gridIdx.x*10)) + 0.5);
	float boxWidth = 4.0 * (abs(noise(gridIdx.y*4)) + 0.5);
	
	float rotAngle = noise(gridIdx.x * 2.0 + 87.0 * gridIdx.y);
	
	pR(p.xz, rotAngle);
	
	if (mod(boxHeight, 4) + mod(boxWidth, 4) > 4) {
		cost = fBox(p, vec3(boxWidth,boxHeight,boxWidth));
	} else {
		cost = fCylinder(p, boxWidth, boxHeight);
	}
	
	if (boxHeight > 8.0) {
		vec3 pcpy = p;
		pcpy.y -= boxHeight;
		float tt = fTorus(pcpy, 1.0, boxWidth);
		cost = fOpUnionChamfer(cost, tt, 1.0);
	}
	
	
	
	return cost;
	
}

float myCol(in vec3 p) {
	return fCapsule(p, 2.0, 12.0);
}

vec3 shapeNormal(in vec3 p) {
	vec3 NEPS = vec3(0.0001, 0.0, 0.0);
	
	vec3 delta = vec3(
					myCol(p + NEPS) - myCol(p - NEPS),
					myCol(p + NEPS.yxz) - myCol(p - NEPS.yxz),
					myCol(p + NEPS.yzx) - myCol(p - NEPS.yzx)
				);
	
	return normalize(delta);
}


float getCrossBars(in vec3 p) {
	
	vec3 pc = p;
	
	pModInterval1(pc.x, 3.0, 2, 8.0);
	
	float cost = fCylinder(pc, 0.5, 9.0);
	
	pc = p-vec3(15,-15,0);
	
	pModInterval1(pc.y, 3.0, 2, 8.0);
	
	pR45(pc.xy);
	pR45(pc.xy);
	
	
	
	float cost2 = fCylinder(pc, 0.5, 9.0);
	cost = fOpUnionChamfer(cost, cost2, 0.0);
	
	return cost;
	
}

float shape1(in vec3 p) {
	
	//vec3 n = shapeNormal(p);
	
	/*vec3 pc = p;
	
	p = p.xzy;
	
	pR(p.yx, p.y * 0.05);
	pR(p.zx, p.x * 0.02);
	float cost = fCylinder(p, 4.0, 20.0);
	cost += hashNoise3D(p*0.3);
	
	vec3 p2 = pc + vec3(-10, 0, -13);
	pR(p2.zx, 10.8);
	pR(p2.xz, p2.y * 0.2);
	pR(p2.yz, p2.x * 0.02);
	
	
	float cost2 = fCylinder(p2, 4.0, 20.0);
	
	pModInterval1(p2.x, 3.0, 2, 10.0);
	
	float cost3 = fCapsule(p2, 3.0, 6.0);
	
	cost = fOpUnionSoft(cost, cost2, 6.0);
	cost = fOpUnionSoft(cost, cost3, 0.0);*/
	
	float cost = getCrossBars(p);
	
	return cost;
	
}

vec3 opTwist( vec3 p )
{
    float  c = cos(0.01*p.y+10.0);
    float  s = sin(0.01*p.y+10.0);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}

float shape2(in vec3 p) {
	
	vec3 pp = p;
	
	pp.x += 3*hashNoise3D(p.yxz*0.7);
	
	
	pp.z += 4.0*hashNoise3D(p.xxz*0.2);
	pR(pp.yz, pp.x*0.2);
	pR(pp.yx, pp.z*0.2);
	
	float minR = 4.0*(0.5 + smoothstep(0.0, 15.0, p.y));
	
	float q = fTorus(pp, minR, 12.0);

	return q;
	
}

float fDemo(in vec3 p) {
	
	float cost;
	
	cost = shape2(p);
	
	foundPart = SIDES;
	
	return cost;
	
}

float fScene(in vec3 p) {
	
	
	float sceneValue = fDemo(p);
	
	
	
	
	if (volumeSize.w == 1.0) {
		// draw volume
		
		float aabb = fBox(p - volumeStart.xyz - volumeSize.xyz*0.5, volumeSize.xyz*0.5);
	
		if (aabb < sceneValue) {
			foundPart = VOLUME;
			sceneValue = aabb;
		}
	}
	
	
		
	return sceneValue;
}

/*
	compute uv coordinates for each found part
*/
vec2 getUV(in vec3 p, in vec3 n, int part) {
	
	vec2 uv = vec2(0.0);

	if (part == ROOM) {
		
		if (dot(n, vec3(0,0,1)) > 0.99) {
			uv = (p.xy+vec2(19,15)) / vec2(38, 30); 
		} else if (dot(n, vec3(0,0,-1)) > 0.99) {
			uv = (p.xy+vec2(19,15)) / vec2(38, 30); 
		} else if (dot(n, vec3(1,0,0)) > 0.99) {
			uv = (p.zy+vec2(19,15)) / vec2(38, 30); 
		} else if (dot(n, vec3(-1,0,0)) > 0.99) {
			uv = (p.zy+vec2(19,15)) / vec2(38, 30); 
		} else if (dot(n, vec3(0,1,0)) > 0.99) {
			uv = (p.xz+vec2(19,19)) / vec2(38, 38); 
		} else if (dot(n, vec3(0,-1,0)) > 0.99) {
			uv = (p.xz+vec2(19,19)) / vec2(38, 38); 
		}
		
	} else if (part == PIEDISTAL) {
		float r;
		if (p.y+14 > 1.4) {
			r = 3.5;
		} else if (p.y+14 > 0.9) {
			r = 4.5;
		} else {
			r = 5.5;
		}
		uv.x = ( PI + acos(p.x / r) ) / (2 * PI);
		uv.y = ( PI + asin(p.z / r) ) / (2 * PI);
	}
	
	uv = mod(uv, vec2(1.0));
	if (uv.x < 0.0) uv.x += 1.0;
	if (uv.y < 0.0) uv.y += 1.0;
	
	return uv;
}


vec3 computeColor(in vec3 p, in vec3 n) {
	
	vec3 objectDiffuseColor, objectSpecularColor;
	
	vec3 newN = n;
	
	float kd = 0.5;
	float ks = 0.5;
	
	if (foundPart == ROOM) {
	
		objectDiffuseColor = vec3(0.2,0.2,0.2);
		vec2 uv = getUV(p, n, ROOM);

		objectSpecularColor = texture(hullSampler, uv).xyz;
		
		
	} else if (foundPart == SIDES) {
	
		objectDiffuseColor = vec3(0,0,0);
		objectSpecularColor = vec3(1);
		
	} else if (foundPart == PIEDISTAL) {
	
		vec2 uv = getUV(p, n, PIEDISTAL);
		objectDiffuseColor = texture(brokenGlass, uv).xyz;
		objectSpecularColor = vec3(0);
		
	} else if(foundPart == VOLUME) {
		return mod(p - volumeStart.xyz, vec3(2.0 * volumeStart.w));
	}
	
	vec3 diffuse, specular;
	
	vec3 pointLightPos = vec3(0,20,0) + knob1.xyz;
	vec3 lightDiffuseColor = vec3(1);
	vec3 lightSpecularColor = vec3(1);
	
	
	
	vec3 lightDir = (p-pointLightPos);
	float r = length(lightDir);
	lightDir /= r;
	
	float ndotl = clamp(dot(newN,-lightDir), 0, 1);
	
	float lPower = (0.001 * r * r + 0.008 * r + 0.1);
	diffuse = ndotl * lightDiffuseColor * kd / lPower;
	
	
	vec3 viewDir = normalize(p-center);
	vec3 H = -normalize(lightDir+viewDir);
	float ndoth = clamp(dot(newN,H), 0, 1);
	
	float specExp = 2;
	specular = ks * pow(ndoth, specExp) * lightSpecularColor / lPower;
	
	vec3 ambient = vec3(0.1);
	
	return ambient + diffuse * objectDiffuseColor + specular * objectSpecularColor;

}
