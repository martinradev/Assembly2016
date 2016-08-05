#version 430

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;
flat in int materialIndexFrag;

layout(location = 0) out vec4 colorOUT;
layout(location = 1) out vec4 positionOUT;

struct ParticleMaterial {
	
	vec3 diffuseColor;
	int diffuseTextureIndex;
	vec3 specularColor;
	int specularTextureIndex;
};

layout(binding = 0, std430) buffer MaterialDeclaration {
	
	ParticleMaterial materials[];
	
};

uniform sampler2D diffuseSamplers[64];
uniform vec3 lightPos;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 fogColor;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 specularMatColor) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.0, 1.0) * difColor;
	vec3 specular = vec3(1.0) * pow(max(0, dot(h, n)), 50.0);
	vec3 Li = intensity*(6.0*diffuse+6.0*specular);
	
	return Li;
}

void main() {
   
   ParticleMaterial mat = materials[materialIndexFrag];
	vec3 difColorIN;
	difColorIN = mat.diffuseColor;
	
	/*
	vec3 specularMatColor;
	if (mat.diffuseTextureIndex == -1)
	{
		
	}
	else
	{
		difColorIN = texture(diffuseSamplers[mat.diffuseTextureIndex], uvFrag).xyz;
	}
	
	specularMatColor = mat.specularColor;
	
	vec3 V = normalize(cameraPosition-positionFrag);
	vec3 h = normalize(-lightDirection + V);
	
	float s = exp(-0.0000295*depthFrag);
	
	//vec3 color = calcLight(h,normalFrag,-lightDirection,difColorIN,lightColor,specularMatColor );
	
	vec3 color = difColorIN;
	color.rgb = mix(fogColor, color.rgb, s);
	*/
	
	vec3 color = difColorIN * 1.0;

	colorOUT = vec4(color, 0.01);
	positionOUT = vec4(positionFrag, uvFrag.t);
}