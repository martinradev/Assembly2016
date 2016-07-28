#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 toScreen;

void main() {
	
	gl_Position = toScreen * vec4(position.xyz, 1.0);

}