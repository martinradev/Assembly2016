#version 450

in vec3 positionFrag;

out vec4 positionOUT;

void main() {

	positionOUT = vec4(positionFrag, 0);
}