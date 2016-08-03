#version 430

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;

out vec3 positionFrag;

uniform mat4 toScreen;
uniform mat4 toWorld;

void main() {
	
	positionFrag = (toWorld * vec4(position.xyz, 1.0)).xyz;
	gl_Position = toScreen * vec4(position.xyz, 1.0);

}