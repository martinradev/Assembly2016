#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 uvFrag;
in vec2 oldUVFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec2 oldUVOut;

uniform vec3 lightPos;
uniform vec3 lightSpecularColor;
uniform vec3 lightDiffuseColor;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

void main() {
	
	vec3 diffuseColor = texture(diffuseTexture, uvFrag).xyz;
	vec3 specularColor = texture(specularTexture, uvFrag).xyz*2.0;
	
	vec3 lighrPos = cameraPosition;
	
	vec3 lightDir = (lighrPos-positionFrag);
	float dist = length(lightDir);
	lightDir /= dist;
	
	float ndotl = dot(normalFrag, lightDir);

	vec3 diffuse = clamp(ndotl, 0.01, 1.0) * lightDiffuseColor;
	
	vec3 viewDir = normalize(cameraPosition -positionFrag);
	vec3 h = normalize(lightDir + viewDir);
	float ndoth = dot(normalFrag, h);

	vec3 specular = pow(clamp(ndoth, 0.2, 1.0), 30.0) * lightSpecularColor;
	

	vec3 color = (diffuse * diffuseColor / (0.0001 * dist * dist + 0.001*dist + 0.01) + specular * specularColor) ;

	diffuseColorOUT = vec4(color, 1.0);
	normalOUT = vec4(normalFrag, 0);
	oldUVOut = oldUVFrag;
}