#version 430

layout(vertices = 3) out;

in vec3 positionVarying[];
in vec2 uvVarying[];

out vec3 tcPositionVarying[];
out vec2 tcUvVarying[];

void main( )
{
    
	tcPositionVarying[gl_InvocationID] = positionVarying[gl_InvocationID];
	tcUvVarying[gl_InvocationID] = uvVarying[gl_InvocationID];

	if (gl_InvocationID == 0) {
		gl_TessLevelInner[0] = 8;
		gl_TessLevelOuter[0] = 8;
		gl_TessLevelOuter[1] = 8;
		gl_TessLevelOuter[2] = 8;
	}
	
}