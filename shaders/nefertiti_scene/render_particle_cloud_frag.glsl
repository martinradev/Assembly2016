#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;
flat in int materialIndexFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;

uniform vec3 lightPos[3];
uniform vec3 lightColor[3];
uniform vec3 lightDirection[3];
uniform vec3 cameraPos;

vec3 calcLight(in vec3 dif, in vec3 pos, in vec3 lightPos, in vec3 intensity)
{
	
	float l = distance(pos, lightPos);
	
	return  dif * intensity / (l*l*0.00000003 + l*0.000002 + 0.03);
	
}

void main() {

	vec3 difColorIN = vec3(140,157,15)/255.0;
	
	vec3 color = calcLight(difColorIN,positionFrag, lightPos[0], lightColor[0]);
	color += 1.2*calcLight(difColorIN,positionFrag, lightPos[1], lightColor[1]);

	diffuseColorOUT = vec4(color, 0.962);
	
	normalOUT = vec4(normalFrag, uvFrag.s);
	positionOUT = vec4(positionFrag, uvFrag.t);
}