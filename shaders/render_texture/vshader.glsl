#version 330

layout(location = 0) in vec2 coords;

void main()
{
	gl_Position = vec4(coords,0,1);
}
