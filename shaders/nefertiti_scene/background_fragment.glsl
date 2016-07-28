#version 430

in vec2 uv;

uniform sampler2D inTex;

out vec4 color;

void main() {
	vec3 q = texture(inTex, uv).xyz;
	q*=q;
	color = vec4(q, 0.3);

}