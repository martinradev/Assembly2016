#version 430

in vec2 uv;

uniform vec2 lightPos;
uniform sampler2D bwSmapler;

out vec4 outColor;

const int SAMPLES = 500;
const float DENSITY = 0.16;
const float WEIGHT = 30.74;
const float EXPOSURE = 0.002;

uniform float DECAY;

float rand(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 n) {
    const vec2 d = vec2(0.0, 1.0);
  vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

void main()
{
	vec2 texCoord = uv;
	vec2 texDirection = (uv-lightPos);
	texDirection /= float(SAMPLES) * DENSITY;
	float illuminationDecay = 1.0;
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