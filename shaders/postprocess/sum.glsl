#version 430

varying vec2 uv;

uniform sampler2D inImage1;
uniform sampler2D inImage2;

uniform bool firstPass;
uniform bool lastPass;

out vec4 color;

void main() {
	vec3 col1 = texture(inImage1,uv).xyz;
	vec3 col2 = texture(inImage2,uv).xyz;
	if (firstPass) col1 = vec3(0.0);
	
	if (lastPass)  {
		color = vec4(mix(col2, col1, vec3(0.4)), 1.0);
	} else {
		color = vec4(col1 + col2, 1.0);
	}
}