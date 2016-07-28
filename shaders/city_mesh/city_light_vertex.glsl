#version 430

in vec3 positionAttrib;
in vec3 normalAttrib;
in vec4 vcolorAttrib; // Workaround. "colorAttrib" appears to confuse certain ATI drivers.
in vec2 texCoordAttrib;

uniform mat4 toScreen;
uniform mat4 toWorld;

out vec3 positionFrag;

void main() {
	gl_Position = toScreen * vec4(positionAttrib, 1.0);
	positionFrag = (toWorld * vec4(positionAttrib, 1.0)).xyz;
}