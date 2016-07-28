#version 430

in vec3 positionAttrib;
in vec3 normalAttrib;
in vec4 vcolorAttrib; // Workaround. "colorAttrib" appears to confuse certain ATI drivers.
in vec2 texCoordAttrib;

out vec3 positionFrag;
out vec3 normalFrag;
out vec2 uvFrag;
out float depthFrag;

uniform mat4 toScreen;
uniform mat4 toWorld;
uniform mat4 normalToWorld;

uniform vec3 cameraPos;

void main() {
	

	gl_Position = toScreen * vec4(positionAttrib, 1.0);
	
	positionFrag = (toWorld * vec4(positionAttrib, 1.0)).xyz;
	
	normalFrag = normalize((normalToWorld * vec4(normalAttrib, 0.0)).xyz);
	
	uvFrag = texCoordAttrib*2.0;
	
	depthFrag = distance(cameraPos, positionFrag);
	
}