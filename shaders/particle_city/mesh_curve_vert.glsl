#version 430

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;

out vec3 positionFrag;
out vec3 normalFrag;
out vec2 uvFrag;
out float depthFrag;
out float vertIndex;

uniform mat4 toScreen;
uniform vec3 cameraPosition;

void main() {
	
	positionFrag = position.xyz;
	normalFrag = normal.xyz;
	uvFrag = vec2(position.w, normal.w);
	
	gl_Position = toScreen * vec4(position.xyz, 1.0);
	depthFrag = distance(cameraPosition, positionFrag);
	vertIndex = gl_VertexID;
}