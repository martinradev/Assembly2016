#version 430

in vec3 positionFrag;


layout(location = 0) out vec4 positionOUT;


void main() {

	positionOUT = vec4(positionFrag, 0.0);

}