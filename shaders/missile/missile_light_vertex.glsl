#version 430

layout(location = 0) in vec4 p;
layout(location = 1) in vec4 n;

out vec3 posFrag;

uniform mat4 toScreen;
uniform mat4 toWorld;

void main()
{
	gl_Position = toScreen * vec4(p.xyz,1.0);
	
	posFrag = (toWorld * vec4(p.xyz, 1.0)).xyz;
	
}
