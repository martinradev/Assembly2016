#version 450

uniform vec3 colorUniform;

layout(location=0) out vec4 color;

void main() {
	color = vec4(colorUniform, 1.0);
}