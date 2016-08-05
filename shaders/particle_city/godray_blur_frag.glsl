#version 430

in vec2 uv;

uniform vec2 lightPos;
uniform sampler2D bwSmapler;

out vec4 outColor;

const int SAMPLES = 90;
const float DENSITY = 0.86;
uniform float WEIGHT;
uniform float time;
const float EXPOSURE = 0.001;

#define HASHSCALE1 1232.1031

float rand(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 n) {
    const vec2 d = vec2(0.0, 1.0);
  vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

float hash13(vec3 p3)
{
	p3  = fract(p3 * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

void main()
{
	vec2 texCoord = uv;
	vec2 texDirection = (uv-lightPos)*(1.0 + 0.1*hash13(vec3(uv, time)))*1.32;
	texDirection /= float(SAMPLES) * DENSITY;
	float illuminationDecay = 1.0;
	vec3 color = texture(bwSmapler, texCoord).xyz;
	for (int i = 0; i < SAMPLES; ++i) {
		texCoord -= texDirection;
		vec4 sampleColor = texture(bwSmapler, texCoord);
		sampleColor *= illuminationDecay * WEIGHT;
		color += sampleColor.xyz;
		illuminationDecay *= 0.965;
	}
	outColor = vec4(EXPOSURE*color,1.0);
	
	
}