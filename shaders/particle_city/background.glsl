#version 430

in highp vec2 uv;

out vec4 color;

uniform mat4 invClip;

vec3 sky(float a, float b) {
    vec2 uv = vec2(a, b);
    vec3 c = vec3(0.0);
    float bb = b;
    bb += sin(a*4.0)*0.05;
	
    
    c += vec3(max(0., sign(b)) * 1.0 - max(0.0, bb)) * vec3(0.4, 0.45, 0.6)*0.1;
    vec3 low = vec3(0.4, 0.35, 0.35)*0.05*0.5;
    c += vec3(max(0.0, sign(-b)) * max(0.0, 1.0+bb -0.08)) * low;
	
	c = mix(c, low, 0.5*(1.0+cos(clamp(b*3.141*16.0, -3.14, 3.14))));
    
    return c;
}

void main( )
{
	const vec2 center = vec2(0.5);
	
	vec2 uvConverted = 2.0 * uv - vec2(1.0);

	vec4 p0 = vec4(uvConverted.x, uvConverted.y, 0.0, 1.0);
	vec4 p1 = vec4(uvConverted.x, uvConverted.y, 1.0, 1.0);
		
	vec4 orig = invClip * p0;
	orig /= orig.w;
		
	vec4 dir = invClip * p1;
	dir /= dir.w;
		
	dir = dir - orig;
	dir.xyz = normalize(dir.xyz);
	
	
	
	vec3 cl = 0.1*sky(dir.x, dir.y);
	
	
	color = vec4(cl, 1.0);
	
}