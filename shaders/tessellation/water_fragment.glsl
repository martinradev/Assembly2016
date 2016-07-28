#version 430

uniform vec3 lightColor;
uniform vec3 seaColor;
in vec3 eval_positionVarying;
in float eval_depthVarying;

out vec4 color;


void main( )
{
	
	color = vec4(lightColor, 1.0);
	color.xyz = mix(vec3(1,0,1), color.xyz, abs(sin(0.01*eval_positionVarying.y)));
	float s = exp(-0.00059*eval_depthVarying);
	color.rgb = mix(seaColor, color.rgb, s);
}