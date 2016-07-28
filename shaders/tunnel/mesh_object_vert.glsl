#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 positionFrag;
out vec3 normalFrag;
out vec2 oldUVFrag;

uniform mat4 prevToScreen;
uniform mat4 toScreen;
uniform mat4 posToWorld;
uniform mat4 normalToWorld;
uniform vec3 cameraPos;

void main() {
	
	positionFrag = (posToWorld * vec4(position.xyz, 1.0)).xyz;
	normalFrag = normalize((normalToWorld * vec4(normal.xyz, 0.0)).xyz);
	
	vec4 oldScreenPos = prevToScreen * vec4(position.xyz, 1.0);
	oldScreenPos.xyz /= oldScreenPos.w;
	oldUVFrag = 0.5 * oldScreenPos.xy + vec2(0.5);
	
	gl_Position = toScreen * vec4(position.xyz, 1.0);

}