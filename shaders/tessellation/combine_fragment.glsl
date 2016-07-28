#version 430

in highp vec2 uv;

uniform sampler2D particleColorTex;
uniform sampler2D terrainColorTex;
uniform sampler2D godrayBlurTex;
uniform float gradUniform;
uniform vec3 gradBottom;
out vec4 color;

void main( )
{
	
	vec4 particleColor = texture(particleColorTex, uv);
	vec4 terrainColor = texture(terrainColorTex, uv);
	vec4 godrayBlurColor = texture(godrayBlurTex, uv);
	
	vec3 finalColor = terrainColor.xyz + particleColor.xyz + godrayBlurColor.xyz;
	
	vec3 addColor = mix(gradBottom, vec3(0.0), uv.y);
	
	finalColor += addColor;
	
	color = vec4(finalColor, 1.0);
	
}