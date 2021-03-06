#version 430

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in float materialIndex;

out vec3 positionFrag;
out vec3 normalFrag;
out vec2 uvFrag;
flat out int materialIndexFrag;


uniform mat4 toScreen;
uniform mat4 posToWorld;
uniform mat4 normalToWorld;

void main() {
	
	positionFrag = (posToWorld * vec4(position.xyz, 1.0)).xyz;
	normalFrag = normalize((normalToWorld * vec4(normal.xyz, 0.0)).xyz);
	uvFrag = vec2(position.w, normal.w);
	materialIndexFrag = int(materialIndex);
	
	
	gl_Position = toScreen * vec4(position.xyz, 1.0);
	
	float depth = gl_Position.z;
	
	const float MAX_POINT_SIZE = 8.0;
	const float MIN_POINT_SIZE = 4.0;
	
	gl_PointSize = mix(MIN_POINT_SIZE, MAX_POINT_SIZE, smoothstep(0.5, 2.5, depth));
	
}