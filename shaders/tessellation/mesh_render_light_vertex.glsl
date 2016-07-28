#version 430

uniform mat4 toScreen;

uniform mat4 toWorld;

in vec3 positionIN;
in vec3 normalIN;

out vec3 positionOUT;

void main()
{

	
	positionOUT = (toWorld * vec4(positionIN, 1)).xyz;
	
	gl_Position = toScreen * vec4(positionIN, 1);

}
