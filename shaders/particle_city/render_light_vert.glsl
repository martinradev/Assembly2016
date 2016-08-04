#version 430

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in float materialIndex;

out vec3 positionFrag;

uniform mat4 toScreen;
uniform mat4 toWorld;

void main() {
	
	positionFrag = (toWorld * vec4(position.xyz, 1.0)).xyz;

	gl_Position = toScreen * vec4(position.xyz, 1.0);
	
	float depth = gl_Position.z;
	
	const float MAX_POINT_SIZE = 12.0;
	const float MIN_POINT_SIZE = 6.0;
	
	gl_PointSize = mix(MIN_POINT_SIZE, MAX_POINT_SIZE, smoothstep(1000.0, 6000.0, depth));
}