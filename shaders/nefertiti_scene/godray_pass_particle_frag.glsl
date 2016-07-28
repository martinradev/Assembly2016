#version 450

layout(location = 0) out vec4 colorOUT;

uniform vec3 color;

void main() {
	
	colorOUT = vec4(color, 1.0);
	
}