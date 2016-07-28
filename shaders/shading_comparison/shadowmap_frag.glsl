#version 430

in float depthVarying;

out float depth;


void main( )
{
	depth = depthVarying;
}