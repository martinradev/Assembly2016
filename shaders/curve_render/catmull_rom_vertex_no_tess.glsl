#version 430

in vec4 p;

out float t;

uniform mat4 toScreen;

void main()
{
	gl_Position = toScreen * vec4(p.xyz,1.0);
	t = p.w;
	
}
