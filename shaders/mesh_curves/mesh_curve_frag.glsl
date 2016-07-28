#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;
uniform vec3 seaColor;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 spec) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.0, 1.0) * difColor;
	vec3 specular = spec * pow(max(0, dot(h, n)), 100.0);
	vec3 Li = intensity * (130.0*diffuse+450.0*specular);
	
	return Li;
}

void main() {

	vec3 difColorIN = vec3(70,40,0)/255.0;
	vec3 specColorIN = vec3(255,200,100)/255.0;

	vec3 V = normalize(cameraPosition-positionFrag);
	vec3 h = normalize(-lightDirection + V);
	
	float s = exp(-0.00055*depthFrag);
	
	vec3 color = calcLight(h,-normalFrag,-lightDirection,difColorIN,lightColor,specColorIN);
	color.rgb = mix(seaColor, color.rgb, s);
	
	//color = vec3(1.0);
	
	diffuseColorOUT = vec4(color, 1.0);
	
	normalOUT = vec4(normalFrag, 0);
	positionOUT = vec4(positionFrag, 0);
}