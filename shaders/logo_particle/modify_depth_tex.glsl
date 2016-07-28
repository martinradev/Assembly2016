#version 430

in highp vec2 uv;

out vec4 position;

uniform sampler2D depthTex;

void main( )
{
	vec2 q = 2.0 * uv - vec2(1.0);
	q *= 1000.0;
	
	float l = distance(uv, vec2(0.5));

	vec3 origPos = texture(depthTex, uv).xyz;
	
	float r = step(0.08, l);
	if (r == 0.0)
	{
		position = vec4(origPos, 0.0);
	}
	else
	{
		position = vec4(q.x, 600.0, q.y, 0.0);
	}
	
	
}