#version 430

in vec3 posFrag;
in vec3 nFrag;
in vec2 uvFrag;
in float depthFrag;

uniform sampler2D diffuseTex;
uniform vec4 specularColor;

uniform vec3 lightPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 seaColor;
uniform vec3 cameraPosition;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;
layout(location = 3) out float depthOUT;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.0, 1.0) * difColor;
	vec3 specular = vec3(0.5) * pow(max(0, dot(h, n)), 100.0);
	vec3 Li = intensity * (30.0*diffuse+100.0*specular);
	
	return Li;
}

void main( )
{
	
	vec3 dif = texture(diffuseTex, uvFrag).xyz;
	
	vec3 V = normalize(cameraPosition-posFrag);
	vec3 h = normalize(-lightDir + V);
	
	float s = exp(-0.00055*depthFrag);
	
	dif = calcLight(h,nFrag,-lightDir,dif,lightColor);
	
	diffuseColorOUT = vec4(dif, 0.0);
	normalOUT = vec4(nFrag, 0.0);
	positionOUT = vec4(posFrag, 0.0);
	depthOUT = depthFrag;
}