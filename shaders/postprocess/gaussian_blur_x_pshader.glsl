#version 330

uniform sampler2D blurSampler;
uniform float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
uniform float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);
out vec4 outColor;
uniform vec2 windowSize;
void main()
{
	outColor = texture(blurSampler, gl_FragCoord.xy / windowSize) * weight[0];
	for (int i = 1; i < 3; ++i) {
		outColor += texture(blurSampler, (gl_FragCoord.xy + vec2(offset[i], 0)) / windowSize) * weight[i];
		outColor += texture(blurSampler, (gl_FragCoord.xy - vec2(offset[i], 0)) / windowSize) * weight[i];
	}
}
