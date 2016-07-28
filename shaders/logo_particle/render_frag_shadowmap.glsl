#version 450

in vec3 positionFrag;

layout(location = 0) out vec4 positionOUT;


uniform vec3 lightPos;



void main() {
   
	positionOUT = vec4(positionFrag,0.0);
}