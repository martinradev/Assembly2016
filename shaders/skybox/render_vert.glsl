#version 430

layout(location = 0) in vec3 posIN;

out vec3 pos;

uniform mat4 toScreen;

void main() {
	
	gl_Position = toScreen * vec4(posIN, 1.0);
	
	pos = posIN;
	
}