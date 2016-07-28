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

uniform vec3 cameraPosition;

uniform vec2 ldSamples[36];

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.0, 1.0) * difColor;
	vec3 specular = vec3(1.0) * pow(max(0.1, dot(h, n)), 5.0);
	vec3 Li = intensity * (150.0*diffuse+450.0*specular);
	
	return Li;
}

void main() {

	vec3 difColor = diffuseUniform.xyz;
	
	vec3 V = normalize(cameraPosition-positionFrag);
	vec3 h = normalize(-lightDirection + V);
	
	float s = exp(-0.00055*depthFrag);
	
	vec3 color = calcLight(h,normalFrag,-lightDirection,difColor,lightColor);
	color.rgb = mix(seaColor, color.rgb, s);
	
	fragColor = vec4(color, 1.0);
	normal = vec4(normalFrag, uvFrag.s);
	position = vec4(positionFrag, uvFrag.t);
	outDepth = depthFrag;
}