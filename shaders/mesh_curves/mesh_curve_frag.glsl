#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;
in float vertIndex;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform vec3 seaColor;
uniform float lastIndex;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 spec) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.0, 1.0) * difColor;
	vec3 specular = spec * pow(max(0, dot(h, n)), 100.0);
	vec3 Li = intensity * (130.0*diffuse+450.0*specular);
	
	return Li;
}

void main() {

	vec3 difColorIN = vec3(240,20,40)/255.0;
	vec3 specColorIN = vec3(255,80,100)/255.0;

	vec3 V = normalize(cameraPosition-positionFrag);
	vec3 h = normalize(-lightDirection + V);
	
	float s = exp(-0.00055*depthFrag);
	
	vec3 color = calcLight(h,-normalFrag,-lightDirection,difColorIN,lightColor,specColorIN);
	color.rgb = mix(seaColor, color.rgb, s);
	
	float falloff = max(0.0, lastIndex- vertIndex);
	
	float r = smoothstep(0.0, 80000.0, falloff);
	
	float q = max(0.0, 1.0-r);
	vec3 lavaColor = vec3(207.0, 0.0, 0.0) / 255.0;
	vec3 addColor = mix(6.0*difColorIN, 20.0*lavaColor, 1.3-q)*q;
	
	color.rgb = color.rgb*max(q, 0.4) + addColor;
	
	diffuseColorOUT = vec4(color, 1.0);
	
	normalOUT = vec4(normalFrag, 0);
	positionOUT = vec4(positionFrag, 0);
}