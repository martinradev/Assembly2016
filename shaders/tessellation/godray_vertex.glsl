#version 430

in vec2 positionAttrib;

out vec2 positionVarying;

void main()
{

	const int numInstancesX = 10;
	const float sideLength = 0.2;
	
	int locationY = gl_InstanceID / numInstancesX;
	int locationX = gl_InstanceID - locationY * numInstancesX;
	
	vec2 pz = (0.5 * positionAttrib + vec2(0.5)) / float(numInstancesX); // in [0,1]
	
	pz += vec2(locationX, locationY) / float(numInstancesX);
	
	pz = 2.0 * pz - vec2(1.0);
	
	positionVarying = pz;
	
	gl_Position = vec4(pz, 0.0, 1);
	
	
}

