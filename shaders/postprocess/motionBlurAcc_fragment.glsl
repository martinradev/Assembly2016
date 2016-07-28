#version 430

out layout(location = 0) vec4 color;

uniform sampler2D colorMap;
in vec2 uv;

void main()
{
	color = vec4(texture(colorMap, uv).rgb, 0.2);
}