#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
flat in int materialIndexFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;

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
uniform vec3 lightPos[3];
uniform vec3 lightColor[3];
uniform vec3 lightDirection[3];
uniform vec3 cameraPos;
uniform samplerCube skyBox;
uniform float AlphaUniform;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 specColorIN) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.008, 1.0) * vec3(0.15, 0.0, 0.01);
	diffuse *= 1.4;
	vec3 specular = vec3(1.0, 0.6, 0.1) * specColorIN * pow(clamp(dot(h, n), 0.06, 1.0), 30.0);
	specular *= 25.0;
	
	/*diffuse = clamp(dot(n, dir), 0.008, 1.0) * vec3(0.15, 0.01, 0.01);
	diffuse *= 1.0;
	vec3 specular = vec3(1.0, 0.6, 0.1) * specColorIN * pow(clamp(dot(h, n), 0.06, 1.0), 30.0);
	specular *= 10.0;*/
	
	//vec3 Li = (diffuse+2.0*specular);
	vec3 Li = intensity * (diffuse+2.0*specular);
	
	return Li;
}

void main() {
	
	//ParticleMaterial mat = materials[materialIndexFrag];
	
	vec3 difColorIN = vec3(70,40,0)/255.0;
	vec3 specColorIN = vec3(255,200,100)/255.0;
	/*if (mat.diffuseTextureIndex == -1)
	{
		difColorIN = mat.diffuseColor;
	}
	else
	{
		difColorIN = texture(diffuseSamplers[mat.diffuseTextureIndex], uvFrag).xyz;
	}
	
	if (mat.specularTextureIndex == -1)
	{
		specColorIN = mat.specularColor;
	}
	else
	{	specColorIN = vec3(0.0);
		
	}*/
	
	
	vec3 V = normalize(cameraPos-positionFrag);

	vec3 color = calcLight(normalize(-lightDirection[0] + V),normalFrag,-lightDirection[0],difColorIN,lightColor[0], specColorIN);
	color += calcLight(normalize(-lightDirection[1] + V),normalFrag,-lightDirection[1],difColorIN,lightColor[1], specColorIN);
	
	diffuseColorOUT = vec4(1.0*color, AlphaUniform);
	
	normalOUT = vec4(normalFrag, uvFrag.s);
	positionOUT = vec4(positionFrag, uvFrag.t);
}