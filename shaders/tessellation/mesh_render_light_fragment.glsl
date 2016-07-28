#version 430

in vec3 positionOUT;

out vec4 position;


void main( )
{
	
	position = vec4(positionOUT, 0);
}