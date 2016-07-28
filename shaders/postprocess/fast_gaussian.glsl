

varying vec2 uv;

uniform sampler2D inImage;
uniform vec2 dir; // guassian blur direction
uniform float step;

float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);

out vec4 color;

void main() {
	

  vec2 off1 = vec2(1.411764705882353) * dir;
  vec2 off2 = vec2(3.2941176470588234) * dir;
  vec2 off3 = vec2(5.176470588235294) * dir;
  color += texture2D(inImage, uv) * 0.1964825501511404;
  color += texture2D(inImage, uv + (off1 * step)) * 0.2969069646728344;
  color += texture2D(inImage, uv - (off1 * step)) * 0.2969069646728344;
  color += texture2D(inImage, uv + (off2 * step)) * 0.09447039785044732;
  color += texture2D(inImage, uv - (off2 * step)) * 0.09447039785044732;
  color += texture2D(inImage, uv + (off3 * step)) * 0.010381362401148057;
  color += texture2D(inImage, uv - (off3 * step)) * 0.010381362401148057;
	
	
	
}