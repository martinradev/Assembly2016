#version 430

in vec3 positionAttrib;

out vec3 positionVarying;

uniform int numInstancesX;
uniform float sideHalfLength;

uniform vec4 knob1;

void main()
{
	
	int locationY = gl_InstanceID / numInstancesX;
	int locationX = gl_InstanceID - locationY * numInstancesX;
	
	float sideLength = 2.0 * sideHalfLength;
	
	vec3 vertexPosition = sideLength * vec3(locationX, 0, locationY) + positionAttrib * sideHalfLength;

	positionVarying = vertexPosition-vec3(3000,0,3000);
	
	
}
