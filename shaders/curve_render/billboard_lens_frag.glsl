#version 430

out vec4 diffuse;

uniform sampler2D flareTex;

void main( )
{

	diffuse = texture(flareTex, gl_PointCoord.st)*2.0;
	
}