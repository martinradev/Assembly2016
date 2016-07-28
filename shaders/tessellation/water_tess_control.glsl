#version 430

layout(vertices = 3) out;

in vec3 positionVarying[];

out vec3 tcPositionVarying[];

void main( )
{
    
	tcPositionVarying[gl_InvocationID] = positionVarying[gl_InvocationID];

	if (gl_InvocationID == 0) {
		gl_TessLevelInner[0] = 16;
		gl_TessLevelOuter[0] = 16;
		gl_TessLevelOuter[1] = 16;
		gl_TessLevelOuter[2] = 16;
	}
	
}