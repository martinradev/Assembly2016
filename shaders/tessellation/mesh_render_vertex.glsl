#version 430

uniform mat4 toScreen;
uniform mat4 toCamera;

uniform mat4 toWorld;
uniform mat4 toWorldNormal;

in vec3 positionIN;
in vec3 normalIN;

out vec3 positionOUT;
out vec3 normalOUT;
out float depthOUT;

void main()
{

	
	positionOUT = (toWorld * vec4(positionIN, 1)).xyz;
	normalOUT = normalize((toWorldNormal * vec4(normalIN, 0)).xyz);
	
	gl_Position = toScreen * vec4(positionIN, 1);
	
	depthOUT = gl_Position.z;
}
