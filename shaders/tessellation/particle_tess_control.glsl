#version 430

layout(vertices = 3) out;

in vec2 positionVarying[];

out vec2 tcPositionVarying[];
out vec2 tcUvVarying[];

void main( )
{
    
	tcPositionVarying[gl_InvocationID] = positionVarying[gl_InvocationID];
	tcUvVarying[gl_InvocationID] = 0.5 * positionVarying[gl_InvocationID] + vec2(0.5);

	if (gl_InvocationID == 0) {
		
		gl_TessLevelInner[0] = 40;
		gl_TessLevelOuter[0] = 40;
		gl_TessLevelOuter[1] = 40;
		gl_TessLevelOuter[2] = 40;
	}
	
}