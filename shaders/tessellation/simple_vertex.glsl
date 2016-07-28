#version 430

in vec3 positionAttrib;

out vec3 positionVarying;
out vec2 uvVarying;

uniform int numInstancesX;
uniform float sideHalfLength;

void main()
{
	
	int locationY = gl_InstanceID / numInstancesX;
	int locationX = gl_InstanceID - locationY * numInstancesX;
	
	float sideLength = 2.0 * sideHalfLength;
	
	vec3 vertexPosition = sideLength * vec3(locationX, 0, locationY) + positionAttrib * sideHalfLength;
	
	float sideLengthUVMulti = 0.5 / (sideHalfLength * float(numInstancesX));
	uvVarying = sideLengthUVMulti * vertexPosition.xz + vec2(sideLengthUVMulti);

	vertexPosition-=vec3(20,0,20);
	positionVarying = vertexPosition;
	gl_Position = vec4(vertexPosition, 1);
	
	
}
