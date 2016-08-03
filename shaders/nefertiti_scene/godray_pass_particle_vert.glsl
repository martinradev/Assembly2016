#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in float materialIndex;

uniform mat4 toScreen;

void main() {
	
	gl_Position = toScreen * vec4(position.xyz, 1.0);
	

	gl_PointSize = 5.0;
}