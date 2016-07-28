#version 430

in highp vec2 uv;

uniform sampler2D meshColorTex;
uniform sampler2D godrayColorTex;

out vec4 color;

void main( )
{
	
	vec4 meshColor = texture(meshColorTex, uv);
	vec4 godrayColor = texture(godrayColorTex, uv);
	
	vec3 finalColor = meshColor.rgb+godrayColor.rgb;

	color = vec4(finalColor, 1.0);
	
}