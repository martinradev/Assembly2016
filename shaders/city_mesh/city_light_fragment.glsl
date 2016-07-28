#version 430

in vec3 positionFrag;

out vec4 positionOUT;
 

void main() {
	positionOUT = vec4(positionFrag, 1.0);
}