
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

float getThreeDiscs(in vec3 p) {
	float bigr = 3;
	float minr = bigr - 0.5;
	
	float h = knob1.x;
	float t = knob1.y;
	if (t > 0) {
		pModPolar(p.xz, t);
		minr -= 0.01*t;
	}
	vec3 pcpy = p;
	
	float f1 = fCylinder(pcpy, bigr, h);
	f1 = fOpDifferenceChamfer(f1, fCylinder(pcpy, minr, h+0.6), 0.0);
	
	pcpy.xz += vec2(bigr, 0.0);
	float f2 = fCylinder(pcpy, bigr, h);
	f2 = fOpDifferenceChamfer(f2, fCylinder(pcpy, minr, h+0.6), 0.0);
	
	pcpy.xz += vec2(-bigr,bigr);
	float f3 = fCylinder(pcpy, bigr, h);
	f3 = fOpDifferenceChamfer(f3, fCylinder(pcpy, minr, h+0.6), 0.0);
	
	float cost = fOpUnionChamfer(f1,f2,0);
	cost = fOpUnionChamfer(f3,cost,0);
	
	
	
	return cost;
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

float fDemo(in vec3 p) {
	
	float cost;
	
	cost = someTest(p);
	
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
	
	vec3 pointLightPos = knob5.xyz;
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
	
	float specExp = 10;
	specular = ks * pow(ndoth, specExp) * lightSpecularColor / lPower;
	
	vec3 ambient = vec3(0.1);
	
	return ambient + diffuse * objectDiffuseColor + specular * objectSpecularColor;

}
