#version 430

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in float depthFrag;
flat in int materialIndexFrag;

layout(location = 0) out vec4 colorOUT;
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

uniform sampler2D diffuseSamplers[64];
uniform vec3 lightPos;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 seaColor;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 specularMatColor) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.0, 1.0) * difColor;
	vec3 specular = vec3(1.0) * pow(max(0, dot(h, n)), 50.0);
	vec3 Li = intensity*(40.0*diffuse+50.0*specular);
	
	return Li;
}

void main() {
   
	
	ParticleMaterial mat = materials[materialIndexFrag];
	
	vec3 difColorIN;
	vec3 specularMatColor;
	if (mat.diffuseTextureIndex == -1)
	{
		difColorIN = mat.diffuseColor;
	}
	else
	{
		difColorIN = texture(diffuseSamplers[mat.diffuseTextureIndex], uvFrag).xyz;
	}
	
	specularMatColor = mat.specularColor;
	
	vec3 V = normalize(cameraPosition-positionFrag);
	vec3 h = normalize(-lightDirection + V);
	
	float s = exp(-0.00055*depthFrag);
	
	vec3 color = calcLight(h,normalFrag,-lightDirection,difColorIN,lightColor,specularMatColor );
	color.rgb = mix(seaColor, color.rgb, s);
	
	colorOUT = vec4(color, 1.0);
	normalOUT = vec4(normalFrag, uvFrag.s);
	positionOUT = vec4(positionFrag, uvFrag.t);
	depthOUT = depthFrag;
	/*float pcf = getShadowI(positionFrag);
	color.rgb = mix(color.rgb, 0.05*color.rgb,  pcf);
	*/
}