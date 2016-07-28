#version 430

layout(vertices = 2) out;

in float tControl[];

out float tEval[];

void main( )
{
    
	tControl[gl_InvocationID] = tEval[gl_InvocationID];

	if (gl_InvocationID == 0) {
		gl_TessLevelInner[0] = 8;
		gl_TessLevelOuter[0] = 8;
	}
	
}