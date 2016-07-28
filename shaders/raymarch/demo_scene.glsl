
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


float fDemo(in vec3 p) {
	
	float cost;
	
	float inRoom = fBox(p, vec3(19, 15, 19));
	float outRoom = fBox(p, vec3(20, 16, 20));
	float room = fOpDifferenceChamfer(outRoom, inRoom, 0.0);
	room = -inRoom;
	float floor = fBox(p+vec3(0,30.2,0), vec3(20, 16, 20));
	float roomNoFloor = fOpDifferenceChamfer(room, floor, 0.0);
	foundPart = ROOM;
	
	vec3 piedistalP = p;
	piedistalP.y += 14;
	float piedistal = fCylinder(piedistalP, 1.5, 0.5);
	
	
	float roomOrig = room;
	room = fOpUnionStairs(piedistal, room, 5, 5);
	if (room+0.005 < roomOrig) {
		foundPart = PIEDISTAL;
	}
	
	float sides1 = fBox(p, vec3(1, 22, 22));
	sides1 = fOpIntersectionStairs(roomNoFloor, sides1, 2, 3);
	sides1 = fOpUnionChamfer(roomNoFloor, sides1, 2);
	
	float sides2 = fBox(p, vec3(22, 22, 1));
	sides2 = fOpIntersectionStairs(roomNoFloor, sides2, 2, 3);
	sides2 = fOpUnionChamfer(roomNoFloor, sides2, 2);
	
	if (sides1 < room || sides2 < room) {
		foundPart = SIDES;
	}
	
	room = fOpUnionChamfer(room, sides1, 0);
	room = fOpUnionChamfer(room, sides2, 0);
	
	
	
	
	
	cost = room;
	
	
	
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
