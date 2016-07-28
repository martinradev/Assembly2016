#version 430

in vec4 positionAttrib;

out highp vec2 uv;

void main()
{
	
	gl_Position = positionAttrib;
	uv = 0.5 * positionAttrib.xy + vec2(0.5);
	
}
