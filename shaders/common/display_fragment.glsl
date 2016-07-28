#version 430

in vec2 uv;

uniform sampler2D inTex;

out vec4 color;

void main() {
	
	color = vec4(texture(inTex, uv).xyz, 1.0);

}