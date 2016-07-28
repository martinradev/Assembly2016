
uniform mat4 posToClip;

in vec3 positionAttrib;
in vec3 normalAttrib;
in float ageAttrib;

out vec3 positionVarying;
out vec3 normalVarying;
out float ageVarying;

void main() {
	gl_Position = posToClip * vec4(positionAttrib,1);
	
	positionVarying = positionAttrib;
	normalVarying = normalAttrib;
	ageVarying = ageAttrib;
}