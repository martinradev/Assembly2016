#version 430

layout(vertices = 3) out;

in vec3 positionVarying[];
in vec3 normalVarying[];
in vec4 colorVarying[];
in vec2 texCoordVarying[];

out vec3 tcPositionVarying[];
out vec3 tcNormalVarying[];
out vec4 tcColorVarying[];
out vec2 tcTexCoordVarying[];

void main( )
{
    
	tcPositionVarying[gl_InvocationID] = positionVarying[gl_InvocationID];
	tcNormalVarying[gl_InvocationID] = normalVarying[gl_InvocationID];
	tcColorVarying[gl_InvocationID] = colorVarying[gl_InvocationID];
	tcTexCoordVarying[gl_InvocationID] = texCoordVarying[gl_InvocationID];
	
	if (gl_InvocationID == 0) {
		gl_TessLevelInner[0] = 50;
		gl_TessLevelOuter[0] = 50;
		gl_TessLevelOuter[1] = 50;
		gl_TessLevelOuter[2] = 50;
	}
	
}