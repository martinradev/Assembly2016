#version 430

in vec3 positionAttrib;
in vec3 normalAttrib;
in vec4 vcolorAttrib;
in vec2 texCoordAttrib;

out vec3 positionVarying;
out vec3 normalVarying;
out vec4 colorVarying;
out vec2 texCoordVarying;

uniform mat4 toWorld;

void main()
{
	

	positionVarying = (toWorld * vec4(positionAttrib,1)).xyz;
	normalVarying = normalAttrib;
	colorVarying = vcolorAttrib;
	texCoordVarying = texCoordAttrib;
	gl_Position = vec4(positionVarying, 1);
	
	
}
