#version 430

in vec3 positionFrag;
in float depthFrag;

uniform vec3 lightPos;
uniform sampler2D bokehTexture;
uniform vec3 seaColor;

out vec4 diffuseColorOUT;
 

void main() {
	diffuseColorOUT = texture(bokehTexture, gl_PointCoord.st);
	
	float s = exp(-0.0039*depthFrag);
	diffuseColorOUT.rgb = mix(seaColor, diffuseColorOUT.rgb, s);
}