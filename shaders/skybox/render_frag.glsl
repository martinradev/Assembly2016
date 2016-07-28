#version 450

uniform samplerCube skyBox;

in vec3 pos;

out vec4 color;

void main() {
	color = texture(skyBox, pos);
	color.xyz *= 0.015;
}