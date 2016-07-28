#version 430

in vec3 eval_Position;

out vec4 position;

void main( )
{
	
	position = vec4(eval_Position,0.0);
	
}