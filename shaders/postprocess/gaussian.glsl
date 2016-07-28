

varying vec2 uv;

uniform sampler2D inImage;
uniform vec2 dir; // guassian blur direction
uniform float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
uniform float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);
uniform float step;

out vec4 color;

void main() {
	
	int kernelSize = intKnob.x;
	
	vec3 acc = texture(inImage,uv).xyz * weight[0];
	for (int i = 1; i < 3; ++i) {
		vec2 off = vec2(offset[i]) * dir * step;
		vec2 coordFW = uv + off;
		vec2 coordBW = uv - off;
		if (coordFW.x < 0.0 || coordFW.x > 1.0f || coordFW.y < 0.0 || coordFW.y > 1.0f) continue;
		if (coordBW.x < 0.0 || coordBW.x > 1.0f || coordBW.y < 0.0 || coordBW.y > 1.0f) continue;
		acc += texture(inImage, coordFW).xyz * weight[i];
		acc += texture(inImage, coordBW).xyz * weight[i];
	}
	color = vec4(acc, 1.0);
	
	
}