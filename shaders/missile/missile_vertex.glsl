#version 430

layout(location = 0) in vec4 p;
layout(location = 1) in vec4 n;

out vec3 posFrag;
out vec3 nFrag;
out vec2 uvFrag;
out float depthFrag;

uniform mat4 toScreen;
uniform mat4 toWorld;
uniform mat4 normalToWorld;

void main()
{
	gl_Position = toScreen * vec4(p.xyz,1.0);
	posFrag = (toWorld * vec4(p.xyz, 1.0)).xyz;
	nFrag = normalize((normalToWorld*vec4(n.xyz,0.0)).xyz);
	uvFrag = vec2(p.w,n.w);
	depthFrag = gl_Position.z;
}
