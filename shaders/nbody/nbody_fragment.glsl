#version 430

uniform float Alpha;

in float depth;
in vec3 pos;

out layout(location=0) vec4 diffuseOut;
out layout(location=2) vec4 positionOut;
out layout(location=3) float depthOut;

uniform vec4 knob2;


void main() {
   
	diffuseOut = vec4(knob2.xyz,1.0);
	positionOut = vec4(pos, 0);
	depthOut = depth;
}