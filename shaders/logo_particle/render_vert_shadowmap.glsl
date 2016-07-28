#version 430

layout(location = 0) in vec4 position;

out vec3 positionFrag;


uniform mat4 toScreen;
uniform mat4 posToWorld;

void main() {
	
	positionFrag = (posToWorld * vec4(position.xyz, 1.0)).xyz;

	gl_Position = toScreen * vec4(position.xyz, 1.0);
	
	float depth = gl_Position.z;
	const float MAX_POINT_SIZE = 10.0;
	const float MIN_POINT_SIZE = 5.0;
	
	gl_PointSize = mix(MIN_POINT_SIZE, MAX_POINT_SIZE, smoothstep(5.0, 1300.0, depth));

}