#version 430

in vec3 positionOUT;
in vec3 normalOUT;
in float depthOUT;

out vec4 diffuseColor;
out vec4 normal; // normal -> (x y z), w -> u
out vec4 position; // (x y z) -> position, v -> w
out float outDepth;

uniform vec3 seaColor;
uniform vec3 lightColorUniform;
uniform vec3 lightDirectionUniform;
uniform vec3 lightPosUniform;
uniform vec3 cameraPosUniform;
uniform vec3 cameraDirUniform;

uniform sampler2D depthMap;
uniform mat4 toLightScreen;
uniform vec2 screenSize;
uniform vec2 ldSamples[36];

vec3 calcLight( in vec3 p, in vec3 n, in vec3 lightDirection, in vec3 lightPosition, in vec3 lightColor )
{
	
	float q = clamp(dot(n, -lightDirection), 0.2, 1.0);
	float l = distance(lightDirection, p);
	float l2 = l*l;
	
	
	vec3 h = normalize(cameraDirUniform-lightDirection);
	float ndoth = clamp(dot(n,h), 0.0, 1.0);
	float intensity = pow(ndoth, 2.0);
	
	vec3 specColor = vec3(2.0);
	
	return ( q * lightColor + intensity *specColor ) / (0.0000015 * l2 + 0.0001 * l + 0.003);
	
}

float getShadowI(in vec3 p) {
	vec4 lwPos = toLightScreen * vec4(p, 1.0);
	lwPos /= lwPos.w;
	lwPos.xy = 0.5* lwPos.xy + vec2(0.5);
	
	float pcf = 0.0;
	
	float rnd = fract(sin(dot(p, vec3(18.9898,99.233,41.164))) * 43758.5453);
	int off = int(rnd * 27.0);
	
	for (int i = 0; i < 9; ++i) {
		vec3 nearestPos = texture(depthMap, lwPos.xy+ldSamples[off+i]/screenSize).xyz;
	
		float l1 = distance(nearestPos, lightPosUniform);
		float l2 = distance(p, lightPosUniform);
		
		float bias =15.0;
		if (l2 < l1+bias) {
			pcf += 1.0;
		}
	}
	pcf /= 9.0;

	return pcf;
}

void main( )
{
	
	diffuseColor = 10.0*vec4(0.4, 0.4, 0.1, 1.0);
	
	
	diffuseColor.rgb *= calcLight(positionOUT, normalOUT, lightDirectionUniform, lightPosUniform, lightColorUniform);
	float pcf = getShadowI(positionOUT);
	diffuseColor.rgb = mix(diffuseColor.rgb, 0.1*diffuseColor.rgb, 1.0 - pcf);
	float s = exp(-0.0029*depthOUT);
	diffuseColor.rgb = mix(seaColor, diffuseColor.rgb, s);
	
	normal = vec4(normalOUT,0);
	position = vec4(positionOUT,0);
	outDepth = depthOUT;
}