
in vec3 positionAttrib;
in vec3 normalAttrib;
in vec4 vcolorAttrib;
in vec2 texCoordAttrib;

out vec3 positionVarying;
out vec3 normalVarying;
out vec4 colorVarying;
out vec2 texCoordVarying;

uniform mat4 toCamera;
uniform mat4 normalToCamera;
uniform mat4 toScreen;

void main()
{
	

	positionVarying = positionAttrib;
	normalVarying = normalize(normalAttrib);
	colorVarying = vcolorAttrib;
	texCoordVarying = texCoordAttrib;
	
	
	gl_Position = toScreen * vec4(positionAttrib, 1);
	
	
}
