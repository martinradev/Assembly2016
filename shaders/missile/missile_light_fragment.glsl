#version 430

out vec4 position;

in vec3 posFrag;

void main( )
{
	
	position = vec4(posFrag, 0.0);
}