

varying vec2 uv;

uniform sampler2D inImage;

out vec4 color;

void main() {
	
	color = texture(inImage,uv);
}