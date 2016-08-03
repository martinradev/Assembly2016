#version 430

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;

out vec3 positionFrag;
out vec3 normalFrag;
out vec2 uvFrag;
out float depthFrag;
out float vertIndex;

uniform mat4 toScreen;
uniform mat4 posToWorld;
uniform mat4 normalToWorld;
uniform vec3 cameraPosition;

void main() {
	
	positionFrag = (posToWorld * vec4(position.xyz, 1.0)).xyz;
	normalFrag = normalize((normalToWorld * vec4(normal.xyz, 0.0)).xyz);
	uvFrag = vec2(position.w, normal.w);
	
	gl_Position = toScreen * vec4(position.xyz, 1.0);
	depthFrag = distance(cameraPosition, positionFrag);
	vertIndex = gl_VertexID;
}