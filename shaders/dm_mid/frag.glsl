#version 430

uniform vec3 cameraPos;

uniform mat4 toCamera;

uniform sampler2D diffuseSampler;
uniform sampler2D normalSampler;
uniform sampler2D specularSampler;

uniform vec4 diffuseUniform;
uniform vec3 specularUniform;
uniform float glossiness;
uniform bool useDiffuseTexture;
uniform bool useNormalMap;
uniform bool useSpecularMap;

in vec3 eval_positionVarying; // world space
in vec3 eval_normalVarying; // world space
in vec4 eval_colorVarying;
in vec2 eval_texCoordVarying;

out vec4 color;
out vec4 normal; // normal -> (x y z), w -> ssao mask
out vec4 position; // (x y z) -> position, v -> w
out float depth;

uniform float ssaoMask;

void main( )
{
	
	vec3 objDiffuseColor = diffuseUniform.xyz;
	vec3 objSpecularColor = specularUniform;
	vec3 N = eval_normalVarying;
	
	if (useDiffuseTexture) {
		objDiffuseColor = texture(diffuseSampler, eval_texCoordVarying).xyz;
	}
	
	if (useSpecularMap) {
		objSpecularColor = texture(specularSampler, eval_texCoordVarying).xyz;
	}
	
	// light computation
	
	float kd = 0.5;
	float ks = 0.5;
	
	vec3 diffuse, specular;
	

	
	vec3 pointLightPos = vec3(0,20, 0);
	vec3 lightDiffuseColor = vec3(1);
	vec3 lightSpecularColor = vec3(1);
	
	vec3 lightDir = (eval_positionVarying-pointLightPos);
	float r = length(lightDir);
	lightDir /= r;
	float ndotl = clamp(dot(N,-lightDir), 0, 1);
	
	float lPower = (0.001 * r * r + 0.008 * r + 0.1);
	diffuse = ndotl * lightDiffuseColor * kd ;
	
	vec3 viewDir = normalize(eval_positionVarying-cameraPos);
	
	vec3 H = -normalize(lightDir+viewDir);
	float ndoth = clamp(dot(N,H), 0, 1);
	specular = ks * pow(ndoth, glossiness) * lightSpecularColor / lPower;
	
	vec3 ambient = vec3(0.1);
	
	vec3 total = ambient + diffuse * objDiffuseColor + specular * objSpecularColor;

    color = vec4(total, 1);
	normal = vec4(0.5 * eval_normalVarying + vec3(0.5), ssaoMask);
	position = vec4(eval_positionVarying, eval_texCoordVarying.t);
	depth = gl_FragCoord.z / gl_FragCoord.w;
}