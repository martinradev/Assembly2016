#version 430

in vec4 inPos;

out vec2 uv;

void main() {
	
	gl_Position = inPos;
	
	uv = 0.5 * inPos.xy + vec2(0.5);
	
}