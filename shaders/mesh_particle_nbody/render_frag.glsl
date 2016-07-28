#version 430

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
flat in int materialIndexFrag;

out vec4 diffuseColorOUT;

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

void main() {
   
	
	ParticleMaterial mat = materials[materialIndexFrag];
	
	vec3 difColorIN;
	if (mat.diffuseTextureIndex == -1)
	{
		difColorIN = mat.diffuseColor;
	}
	else
	{
		difColorIN = texture(diffuseSamplers[mat.diffuseTextureIndex], uvFrag).xyz;
	}
	

	float l = distance(positionFrag, lightPos);
	
	
	vec3 color = difColorIN * clamp(dot(normalFrag, normalize(lightPos-positionFrag)), 0.05, 1.0) / (0.005 * l * l + 0.003 * l + 0.03);
	
	diffuseColorOUT = vec4(color, 1.0);
}