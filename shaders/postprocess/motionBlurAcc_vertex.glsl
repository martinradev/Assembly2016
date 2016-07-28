#version 450

in vec4 posAttrib;
out vec2 uv;

void main()
{
	gl_Position = posAttrib;
	uv = 0.5 * posAttrib.xy + vec2(0.5);
}
