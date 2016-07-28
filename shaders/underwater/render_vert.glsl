#version 430

layout(location = 0) in vec4 position;

out vec3 positionFrag;
out float depthFrag;

uniform mat4 toScreen;

void main() {
	
	positionFrag = position.xyz;
	
	gl_Position = toScreen * vec4(position.xyz, 1.0);
	
	float depth = gl_Position.z;
	
	const float MAX_POINT_SIZE = 4.0;
	const float MIN_POINT_SIZE = 2.0;
	
	gl_PointSize = mix(MIN_POINT_SIZE, MAX_POINT_SIZE, smoothstep(0.5, 200.0, depth));
	depthFrag = depth;
}