#version 430

in vec2 uv;

uniform vec2 lightPos;
uniform sampler2D bwSmapler;

out vec4 outColor;

const int SAMPLES = 100;
const float DENSITY = 0.36;
const float WEIGHT = 1.24;
const float DECAY = 0.99;
const float EXPOSURE = 0.001;

void main()
{
	vec2 texCoord = uv;
	vec2 texDirection = normalize(lightPos-uv);

	texDirection /= float(SAMPLES) * DENSITY;
	float illuminationDecay = 0.1;
	vec3 color = texture(bwSmapler, texCoord).xyz;
	for (int i = 0; i < SAMPLES; ++i) {
		texCoord -= texDirection;
		vec4 sampleColor = texture(bwSmapler, texCoord);
		sampleColor *= illuminationDecay * WEIGHT;
		color += sampleColor.xyz;
		illuminationDecay *= DECAY;
	}
	outColor = vec4(EXPOSURE*color,1.0);
}