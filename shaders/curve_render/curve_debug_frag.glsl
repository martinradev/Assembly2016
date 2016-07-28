#version 430

in float t;

out vec4 diffuse;

uniform vec3 color;

void main( )
{
	diffuse = vec4(color, 1.0);
}