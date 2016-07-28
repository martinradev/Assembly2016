#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;
flat in int materialIndexFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;
layout(location = 3) out float depthOUT;

struct ParticleMaterial {
	
	vec3 diffuseColor;
	int diffuseTextureIndex;
	vec3 specularColor;
	int specularTextureIndex;
};

layout(binding = 0, std430) buffer MaterialDeclaration {
	
	ParticleMaterial materials[];
	
};

//uniform sampler2D diffuseSamplers[64];
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform vec3 cameraDir;

uniform sampler2D shadowMapTex;
uniform mat4 toLightClip;
uniform vec2 shadowMapSize;

uniform vec2 ldSamples[36];

vec3 calcColor(in vec3 pParam, in vec3 nParam, in vec3 difColorParam, in vec3 specColorParam, in vec3 lightPosParam, in vec3 lightDirParam, in vec3 lightColorParam, in vec3 viewDir) {
	
	float ld = distance(pParam, -lightPosParam);
	float ndotl = clamp(dot(nParam, -lightDirParam), 0.01, 1.0);
	
	vec3 dif = lightColor * difColorParam * (ndotl / (0.0001 * ld * ld + 0.001 * ld + 0.01));

	return dif;
}

float getShadowI(in vec3 p) {
	vec4 lwPos = toLightClip * vec4(p, 1.0);
	lwPos /= lwPos.w;
	lwPos.xy = 0.5* lwPos.xy + vec2(0.5);
	
	float pcf = 0.0;
	
	float rnd = fract(sin(dot(p, vec3(18.9898,99.233,41.164))) * 43758.5453);
	int off = int(rnd * 27.0);
	
	for (int i = 0; i < 9; ++i) {
		float l1 = distance(texture(shadowMapTex, lwPos.xy+ldSamples[off+i]/shadowMapSize).xyz, lightPos);

		float l2 = distance(p, lightPos);
		
		float bias = 1.0;
		if (l2 > l1+bias) {
			pcf += 1.0;
		}
	}
	pcf /= 9.0;
	return pcf;
}

void main() {
	
	ParticleMaterial mat = materials[materialIndexFrag];
	
	vec3 difColorIN;
	vec3 specColorIN;
	if (mat.diffuseTextureIndex == -1)
	{
		difColorIN = mat.diffuseColor;
	}
	else
	{
		//difColorIN = texture(diffuseSamplers[mat.diffuseTextureIndex], uvFrag).xyz;
	}
	
	if (mat.specularTextureIndex == -1)
	{
		specColorIN = mat.specularColor;
	}
	else
	{	specColorIN = vec3(0.0);
		
	}
	
	vec3 lightDirection = normalize(-lightPos);
	
	
	vec3 color = calcColor(positionFrag, normalFrag, difColorIN, specColorIN, lightPos, lightDirection, lightColor, cameraDir);
	
	float pcf = getShadowI(positionFrag);
	color = mix(color, 0.1*color, pcf);
	
	//color = vec3(pcf);
	
	diffuseColorOUT = vec4(color, 1.0);
	
	normalOUT = vec4(normalFrag, uvFrag.s);
	positionOUT = vec4(positionFrag, uvFrag.t);
	depthOUT = 0.0;
}