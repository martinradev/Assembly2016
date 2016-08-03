#version 430

in vec4 p;

uniform mat4 toScreen;

void main()
{
	gl_Position = toScreen * vec4(p.xyz,1.0);
	
	float depth = gl_Position.z;
	
	const float MAX_POINT_SIZE = 110.0;
	const float MIN_POINT_SIZE = 60.0;
	
	gl_PointSize = mix(MIN_POINT_SIZE, MAX_POINT_SIZE, smoothstep(1000.0, 5000.0, depth));
}
