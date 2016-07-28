#version 430

in vec3 eval_positionVarying;
in float eval_dWater;
in float eval_depthVarying;

out vec4 diffuse;


void main( )
{

	const float LIGHT_I = 0.3;
	const float KA = 0.001;
	const float KD = 0.005;
	vec3 color = vec3(LIGHT_I * exp(- KA * eval_dWater) * exp(-KD * eval_depthVarying));

	vec3 seaColor = vec3(0.0, 0.1607, 0.227);
	
	float s = exp(-0.01*eval_depthVarying);

	color = mix(color, seaColor, s);

	diffuse = vec4(color, 0.1);
	
}