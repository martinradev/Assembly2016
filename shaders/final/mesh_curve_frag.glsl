#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;
layout(location = 3) out float depthOUT;

uniform sampler2D depthTex;
uniform sampler2D diffuseTex;
uniform sampler2D specularTex;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;

uniform vec2 ldSamples[36];
uniform sampler2D depthMap;
uniform mat4 toLightScreen;
uniform vec2 screenSize;

float getShadowI(in vec3 p) {
	vec4 lwPos = toLightScreen * vec4(p, 1.0);
	lwPos /= lwPos.w;
	lwPos.xy = 0.5* lwPos.xy + vec2(0.5);
	
	float pcf = 0.0;
	
	float rnd = fract(sin(dot(p, vec3(18.9898,99.233,41.164))) * 43758.5453);
	int off = int(rnd * 27.0);
	
	for (int i = 0; i < 9; ++i) {
		vec3 nearestPos = texture(depthMap, lwPos.xy+ldSamples[off+i]/screenSize).xyz;
	
		float l1 = distance(nearestPos, lightPos);
		float l2 = distance(p, lightPos);
		
		float bias = 50.0;
		if (l2 < l1+bias) {
			pcf += 1.0;
		}
	}
	pcf /= 9.0;

	return pcf;
}

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 spec) {
	vec3 diffuse;
	float q = dot(n, dir);
	if (q < 0.0) q = -q;
	diffuse = clamp(q, 0.1, 1.0) * difColor;
	q = dot(h, n);
	if (q < 0.0) q = -q;
	vec3 specular = spec * pow(q, 10.0);
	vec3 Li = intensity * (0.2*diffuse+0.4*specular);
	
	vec3 dv = positionFrag-lightPos;
	float l1 = distance(positionFrag, lightPos);
	float l2 = l1*l1;
	
	Li = Li / (0.0000000002 * l2 + 0.00008 * l1 + 0.0052);
	
	return Li;
}

void main() {

	vec3 difColorIN = texture(diffuseTex, uvFrag).xyz;
	vec3 specColorIN = texture(specularTex, uvFrag).xyz;

	vec3 V = normalize(cameraPosition-positionFrag);
	vec3 h = normalize(lightDirection + V);
	
	vec3 color = calcLight(h,-normalFrag,-lightDirection,difColorIN,lightColor,specColorIN);
	
	float pcf = getShadowI(positionFrag);
	
	color = mix(color, vec3(0.0), 1.0 - pcf);
	
	
	diffuseColorOUT = vec4(color, 1.0);
	normalOUT = vec4(normalFrag, 0);
	positionOUT = vec4(positionFrag, 0);
	depthOUT = depthFrag;
}