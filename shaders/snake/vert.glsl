#version 430

in vec3 positionAttrib;

uniform mat4 posToClip;

void main()
{
	
	gl_Position = posToClip * vec4(positionAttrib, 1);
	
	
}
