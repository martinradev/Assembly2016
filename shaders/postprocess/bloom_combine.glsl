#version 430

in vec2 uv;

out vec4 color;
uniform int numLevels;
uniform sampler2D blurLevels[5];


void main()
{
	
	vec3 c = vec3(0.0);
	
	for (int i = 0; i < numLevels; ++i) c += texture(blurLevels[i], uv).xyz;
	
	color = vec4(c, 1.0);
	
}