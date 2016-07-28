#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;

uniform vec3 lightPos[3];
uniform vec3 lightColor[3];
uniform vec3 lightDirection[3];
uniform vec3 cameraPos;
uniform samplerCube skyBox;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 specColorIN) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.008, 1.0) * difColor;
	vec3 specular = specColorIN * pow(clamp(dot(h, n), 0.06, 1.0), 50.0);
	vec3 Li = intensity * (10.0*diffuse+10.0*specular);
	
	return Li;
}

void main() {

	vec3 difColorIN = vec3(70,40,0)/255.0;
	vec3 specColorIN = vec3(255,200,100)/255.0;

	vec3 V = normalize(cameraPos-positionFrag);

	vec3 color = calcLight(normalize(lightDirection[0] + V),normalFrag,lightDirection[0],difColorIN,lightColor[0], specColorIN);
	color += calcLight(normalize(lightDirection[1] + V),normalFrag,lightDirection[1],difColorIN,lightColor[1], specColorIN);
	

	diffuseColorOUT = vec4(color, 1.0);
	
	normalOUT = vec4(normalFrag, 0);
	positionOUT = vec4(positionFrag, 0);
}