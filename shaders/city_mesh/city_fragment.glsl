#version 430

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 normal; // normal -> (x y z), w -> u
layout(location = 2) out vec4 position; // (x y z) -> position, v -> w
layout(location = 3) out float outDepth;

uniform sampler2D diffuseSampler;
uniform sampler2D normalSampler;
uniform sampler2D specularSampler;

uniform vec4 diffuseUniform;
uniform vec3 specularUniform;
uniform float glossiness;

uniform bool useDiffuseTexture;
uniform bool useNormalMap;
uniform bool useSpecularMap;

uniform vec3 seaColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 lightDirection;

uniform sampler2D depthMap;
uniform mat4 toLightScreen;
uniform vec2 screenSize;

uniform float sineT;

uniform vec3 cameraPosition;

uniform vec2 ldSamples[36];

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, float specularMask) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.0, 1.0) * difColor;
	vec3 specular = vec3(1.0) * pow(max(0, dot(h, n)), 100.0);
	vec3 Li = intensity * (130.0*diffuse+450.0*specular);
	
	return Li;
}

float getShadowI(in vec3 p) {
	vec4 lwPos = toLightScreen * vec4(p, 1.0);
	lwPos /= lwPos.w;
	lwPos.xy = 0.5* lwPos.xy + vec2(0.5);
	
	float pcf = 0.0;
	
	float rnd = fract(sin(dot(p, vec3(18.9898,99.233,41.164))) * 43758.5453);
	int off = int(rnd * 27.0);
	
	for (int i = 0; i < 9; ++i) {
		vec3 nearestPos = texture(depthMap, lwPos.xy+ldSamples[off+i]/screenSize).xyz;
	
		float l1 = distance(nearestPos, lightPosition);
		float l2 = distance(p, lightPosition);
		
		float bias =15.0;
		if (l2 < l1+bias) {
			pcf += 1.0;
		}
	}
	pcf /= 9.0;

	return pcf;
}

void main() {

	vec3 difColor;
	
	if (useDiffuseTexture)
	{
		difColor = texture(diffuseSampler, uvFrag).xyz;
	}
	else
	{
		difColor = diffuseUniform.xyz;
	}
	
	vec3 V = normalize(cameraPosition-positionFrag);
	vec3 h = normalize(-lightDirection + V);
	
	float s = exp(-0.00055*depthFrag);
	
	float specMask = texture(specularSampler, uvFrag).r;
	
	vec3 color = calcLight(h,normalFrag,-lightDirection,difColor,lightColor, specMask);
	float pcf = getShadowI(positionFrag);
	color.rgb = mix(color.rgb, 0.05*color.rgb,  pcf);
	color.rgb = mix(seaColor, color.rgb, s);
	
	
	if (useSpecularMap)
	{
		vec3 specColor = texture(specularSampler, uvFrag).xyz;
		
		float a = abs(sin(-sineT + positionFrag.y*0.0005));
		a = smoothstep(0.8, 2.5, a);
		a = step(2000.0, positionFrag.y)*a;
		color.rgb += 60.0*a*specColor;
	}
	
	fragColor = vec4(color, 1.0);
	normal = vec4(normalFrag, uvFrag.s);
	position = vec4(positionFrag, uvFrag.t);
	outDepth = depthFrag;
}