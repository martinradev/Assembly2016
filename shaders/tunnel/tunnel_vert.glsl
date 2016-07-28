#version 430

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;

out vec3 positionFrag;
out vec3 normalFrag;
out vec2 uvFrag;
out vec2 oldUVFrag;

uniform mat4 prevToScreen;
uniform mat4 toScreen;
uniform mat4 posToWorld;
uniform mat4 normalToWorld;

void main() {
	
	positionFrag = (posToWorld * vec4(position.xyz, 1.0)).xyz;
	normalFrag = -normalize((normalToWorld * vec4(normal.xyz, 0.0)).xyz);
	uvFrag = vec2(position.w, normal.w) * vec2(235.0,5.0);
	
	vec4 oldScreenPos = prevToScreen * vec4(position.xyz, 1.0);
	oldScreenPos.xyz /= oldScreenPos.w;
	oldUVFrag = 0.5 * oldScreenPos.xy + vec2(0.5);

	gl_Position = toScreen * vec4(position.xyz, 1.0);
}