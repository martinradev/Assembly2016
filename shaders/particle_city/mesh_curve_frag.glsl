#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;
in float vertIndex;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 positionOUT;


uniform float lastIndex;

void main() {
	
	vec3 difColorIN = vec3(240,60,40)/255.0;
	
	vec3 color = difColorIN;
	
	float falloff = max(0.0, lastIndex- vertIndex);
	
	float r = smoothstep(0.0, 90000.0, falloff);
	
	float q = max(0.0, 1.0-r);
	vec3 lavaColor = vec3(857.0, 0.0, 0.0) / 255.0;
	vec3 addColor = mix(20.0*difColorIN, 20.0*lavaColor, 1.3-q)*q;
	
	color.rgb = color.rgb*max(q, 0.4) + addColor;
	
	diffuseColorOUT = vec4(color, 1.0);
	positionOUT = vec4(positionFrag, 1.0);
	
}