#version 430

in float t;

out vec4 diffuse;

uniform float tLast;
uniform float tFirst;

void main( )
{
	
	float q = smoothstep(tFirst, tLast, t);
	
	vec3 startColor = vec3(1.0, 0.35, 0.01);
	vec3 endColor = vec3(0.85,0.97,0.407);
	
	vec3 color = mix(startColor, endColor, q);
	
    diffuse = vec4(color, 1.0-q);
	
}