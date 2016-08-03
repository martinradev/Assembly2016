#version 430

in highp vec2 uv;

uniform sampler2D meshTex;
uniform sampler2D godrayBlurTex;

out vec4 color;

void main( )
{
	
	vec4 meshColor = texture(meshTex, uv);
	vec4 godrayBlurColor = texture(godrayBlurTex, uv);

	vec3 finalColor = meshColor.xyz + godrayBlurColor.xyz;
	
	
	color = vec4(finalColor, 1.0);
	
}