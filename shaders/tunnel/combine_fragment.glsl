#version 430

in highp vec2 uv;

uniform sampler2D meshColorTex;
uniform sampler2D godrayColorTex;

uniform bool useGodray; 

out vec4 color;

void main( )
{
	
	vec4 meshColor = texture(meshColorTex, uv);
	vec4 godrayColor;
	
	if (useGodray)
	{
		godrayColor = texture(godrayColorTex, uv);
	}
	else
	{
		godrayColor = vec4(0,0,0,1);
	}
	
	vec3 finalColor = meshColor.rgb+godrayColor.rgb;

	color = vec4(finalColor, 1.0);
	
}