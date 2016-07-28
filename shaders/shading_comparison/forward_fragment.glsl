
uniform vec3 cameraPos;

in vec3 positionVarying; // world space
in vec3 normalVarying; // world space
in vec4 colorVarying;
in vec2 texCoordVarying;

uniform sampler2D diffuseSampler;
uniform sampler2D normalSampler;
uniform sampler2D specularSampler;

uniform vec4 diffuseUniform;
uniform vec3 specularUniform;
uniform float glossiness;
uniform bool useDiffuseTexture;
uniform bool useNormalMap;
uniform bool useSpecularMap;

out vec4 color;
out vec4 normal; // normal -> (x y z), ssao mask
out vec4 position; // (x y z) -> position, v -> w
out float depth;

// DEBUG STUFF
uniform mat4 toCamera;
uniform float ssaoMask;

mat3 formBasis(in vec3 n) {

	vec3 tmpn = abs(n);
	
	if (tmpn.x <= tmpn.y && tmpn.x <= tmpn.z) {
		tmpn.x = 1.0;
	} else if(tmpn.y <= tmpn.x && tmpn.y <= tmpn.z) {
		tmpn.y = 1.0;
	} else {
		tmpn.z = 1.0;
	}
	
	tmpn = normalize(cross(tmpn, n));
	
	mat3 basis;
	basis[0] = tmpn;
	basis[1] = cross(n, tmpn);
	basis[2] = n;
	
	return basis;
}

void main( )
{
	
	vec3 pointLightPos = vec3(0,25,0); // world space
	
	vec3 objDiffuseColor = diffuseUniform.xyz;
	vec3 objSpecularColor = specularUniform;
	vec3 N = normalVarying;
	
	if (useDiffuseTexture) {
		objDiffuseColor = texture(diffuseSampler, texCoordVarying).xyz;
	}
	
	if (useSpecularMap) {
		objSpecularColor = texture(specularSampler, texCoordVarying).xyz;
	}
	
	
	// light computation
	
	float kd = 0.5;
	float ks = 0.5;
	
	vec3 diffuse, specular;
	
	
	vec3 lightDiffuseColor = vec3(1);
	vec3 lightSpecularColor = vec3(1);
	
	vec3 lightDir = (positionVarying-pointLightPos);
	float r = length(lightDir);
	lightDir /= r;
	float ndotl = clamp(dot(N,-lightDir), 0.4, 1.0);
	
	float lPower = (0.0003 * r * r + 0.016 * r + 0.01);
	diffuse = ndotl * lightDiffuseColor * kd / lPower;
	
	vec3 viewDir = normalize(positionVarying-cameraPos);
	
	vec3 H = -normalize(lightDir+viewDir);
	float ndoth = clamp(dot(N,H), 0.3, 1.0);
	specular = ks * pow(ndoth, glossiness) * lightSpecularColor / lPower;
	
	vec3 ambient = vec3(0.1);
	
	vec3 total = ambient + diffuse * objDiffuseColor + specular * objSpecularColor;
	
	

	//total = 0.5 * normalize(N) + vec3(0.5);
	color = vec4(total, 1);
	normal = vec4(0.5 * normalVarying + vec3(0.5), ssaoMask);
	position = vec4(positionVarying, texCoordVarying.t);
	depth = gl_FragCoord.z / gl_FragCoord.w;
}