
uniform sampler2D hullSampler;
uniform sampler2D procTex[10];

uniform vec4 volumeStart; // (x,y,z) -> volume's start, (w) cube length 
uniform vec4 volumeSize; // (x,y,z) -> volume's size, volumeSize, (w) -> 1.0 draw volume, 0.0 not draw volume

#define HULL 0
#define ROOF 1
#define BASE 2
#define COLUMN 3
#define STAR 4
#define FLOOR 5
#define STAIRS 6
#define DOOR 7

#define VOLUME -1
#define SHPERE_TEST -2


int foundPart;

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
	
	fbm += 0.04*cnoise(p);
	fbm += 0.02*cnoise(p*0.2);
	fbm += 0.01*cnoise(p*0.4);
	
	return fbm;
}

float fUFO(in vec3 p) {

	// column
	float column = fUFOColumnAndStar(p + vec3(columnNoise(p), 0, -0.64)) + cnoise(p*20.0)*0.002;
	float columnStar = fUFOStar(p + vec3(0, 0, -0.64)) + bumpFunction(p)*0.06;
	
	
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
	float topCone = fSphere(p + vec3(0, 23.6055 - tetraVoronoi(p)*0.15, 0.0), 25.0);
	topCone = fOpIntersectionChamfer(topCone, fSphere(p + vec3(0, 22.35, 0.0), 23.8), 0.0);
	float topConeBox = fBox(p + vec3(0, 23.6055 + 0.74, 0.0), vec3(25));
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
	base += cnoise(p*20.0)*0.003;
	
	foundPart = HULL;

	if (ufo > topCone) {
		ufo = topCone;
		foundPart = ROOF;
	}
	
	if (ufo > sphereFloor) {
		ufo = sphereFloor;
		foundPart = FLOOR;
	}
	
	if (ufo > column) {
		ufo = column;
		foundPart = COLUMN;
	}
	
	if (ufo > base) {
		ufo = base;
		foundPart = BASE;
	}
	
	if (ufo > columnStar) {
		ufo = columnStar;
		foundPart = STAR;
	}
	
	if (ufo > door) {
		ufo = door;
		foundPart = DOOR;
	}

	//ufo += mybump;
	
	return ufo;
	
}

float fUFOSimple(in vec3 p) {
	foundPart = COLUMN;
	
	p.y -= 8.0;
	return fSphere(p, 10.0);
}

/*
float fHeight(in vec3 p) {
	
	return 25.0*texture(concreteSampler, p.xz*0.01).r;
}*/

float fScene(in vec3 p) {
	
	
	float sceneValue = fUFO(p);
	
	
	
	
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

vec3 roofDiffuse(vec3 p) {
	return vec3(0.5+0.5*bumpFunction(p));
}

/*
	compute uv coordinates for each found part
*/
vec2 getUV(in vec3 p, in vec3 n, int part) {
	
	vec2 uv = vec2(0.0);

	if (part == HULL) {
		vec3 pCopy = normalize(p);
		
		const float U_SCALE = 6.0;
		const float V_SCALE = 5.0;
		
		float brickDz = pCopy.z;
		float brickDx = pCopy.x;
		float brickDy = pCopy.y;
		float brickU = U_SCALE * (0.5 + (atan(brickDz,brickDx)) / (2.0 * PI));
		float brickV = V_SCALE * (0.5 + asin(brickDy) / PI);
		
		uv = vec2(brickU, brickV);
	} else if(part == ROOF) {
	
		
		const float U_SCALE = 6.0;
		const float V_SCALE = 5.0;
		
		float r = 6.0;

		uv = vec2(p.x + 6.0, p.z+6.0) * vec2(0.1, 0.1) / r;
		
	} else if (part == BASE) {
		vec3 pCopy = normalize(p);
		
		const float U_SCALE = 0.16;
		const float V_SCALE = 0.1;
		
		uv = vec2(p.z, p.y) * vec2(U_SCALE, V_SCALE);
		
	} else if(part == COLUMN) {
		
		uv = p.xy*0.2;
		
	} else if(part == DOOR) {
		
		uv = (p.zy+vec2(0.1,-0.22)) * 0.6;
	
	}
	
	// make sure uv is in [0,1]^2
	
	uv = mod(uv, vec2(1.0));
	if (uv.x < 0.0) uv.x += 1.0;
	if (uv.y < 0.0) uv.y += 1.0;
	
	return uv;
}

vec3 computeColor(in vec3 p, in vec3 n) {
	
	vec3 objectDiffuseColor;
	
	if (foundPart == HULL) {
		objectDiffuseColor = vec3(1,0,0);
		
		
		vec3 brickColor = texture(hullSampler, getUV(p, n, HULL)).xyz;
		
		
		float texNoise = cnoise(p * 0.03);
		texNoise += cnoise(p * 0.5 + vec3(2.0, 2.884, 0.0));
		
		texNoise = clamp(0.5*texNoise + 0.5, 0.0, 1.0);

		vec3 dirtyColor = vec3(1.0 - texNoise);
		
		objectDiffuseColor = mix(dirtyColor, brickColor, 0.5);
		
	} else if(foundPart == ROOF) {
		//objectDiffuseColor = roofDiffuse(p);
		
		objectDiffuseColor = texture(procTex[4], getUV(p, n, ROOF)).xyz;
		
	} else if(foundPart == BASE) {
		objectDiffuseColor = texture(procTex[3], getUV(p, n, BASE)).xyz;
	} else if(foundPart == FLOOR) {
		objectDiffuseColor = vec3(0,0,0);
	} else if(foundPart == COLUMN) {
		objectDiffuseColor = texture(procTex[0], getUV(p, n, COLUMN)).xyz;
	} else if(foundPart == STAR) {
		objectDiffuseColor = vec3(1,0.0,0.0);
	} else if(foundPart == DOOR) {
		objectDiffuseColor = texture(procTex[5], getUV(p, n, DOOR)).xyz;
	} else if(foundPart == VOLUME) {
		return mod(p - volumeStart.xyz, vec3(2.0 * volumeStart.w));
	} else if(foundPart == SHPERE_TEST) {
		float perlin = 0.5 * pnoise(p*10.0, knob1.xyz) + 0.5;
		objectDiffuseColor = vec3(1.0, 1.0, 1.0) * perlin;
		
	} else {
		objectDiffuseColor = vec3(1.0);
	}
	
	vec3 lightDir1 = vec3(0,-1,0);
	vec3 lightColor1 = vec3(1);
	
	vec3 lightDir2 = normalize(knob6.xyz);
	vec3 lightColor2 = knob7.xyz;
	
	vec3 ambient = vec3(0.5);
	
	
	float lFactor1 = clamp(dot(n, -lightDir1), 0.0, 1.0);
	float lFactor2 = clamp(dot(n, -lightDir2), 0.0, 1.0);
	
	vec3 diffuse = (lightColor1 * lFactor1 + lightColor2 * lFactor2);
	
	return (diffuse + ambient) * objectDiffuseColor;

}
