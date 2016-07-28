#version 450

in vec2 uv;

out vec4 colorOUT;

uniform sampler2D colorTex;
uniform sampler2D velocityTex;



void main() {
	
	vec3 color = vec3(0.0);
	
	vec2 targetUV = texture(velocityTex, uv).rg;
	
	const int NUM_SAMPLES = 60;
	
	vec2 delta = (targetUV-uv) / float(25.0);
	
	vec2 startUV = uv;
	for (int i = 0; i < NUM_SAMPLES; ++i)
	{
		color += texture(colorTex, startUV).rgb;
		startUV += delta;
	}
	
	colorOUT = vec4(color / float(NUM_SAMPLES), 0.0);
}