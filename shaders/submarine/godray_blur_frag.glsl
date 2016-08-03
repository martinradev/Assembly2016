#version 430

in vec2 uv;

uniform vec2 lightPos;
uniform sampler2D bwSmapler;

out vec4 outColor;

const int SAMPLES = 200;
const float DENSITY = 0.86;
const float WEIGHT = 30.74;
const float EXPOSURE = 0.002;
uniform float DECAY;

void main()
{
	vec2 texCoord = uv;
	vec2 texDirection = (uv-lightPos);
	texDirection /= float(SAMPLES) * DENSITY;
	float illuminationDecay = 1.0;
	vec3 color = texture(bwSmapler, texCoord).xyz;
	
	
	for (int i = 0; i < SAMPLES; ++i) {
		texCoord -= texDirection;
		float l = dot(uv, texCoord);
		vec4 sampleColor = texture(bwSmapler, texCoord)*(1.5-l);
		sampleColor *= illuminationDecay * WEIGHT;
		color += sampleColor.xyz;
		illuminationDecay *= DECAY;
	}
	outColor = vec4(EXPOSURE*color*DECAY,1.0);
	
	
}