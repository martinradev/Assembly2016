#version 430

in vec2 uv;

uniform sampler2D inTex;
uniform sampler2D inStarTex;
uniform float inScrollX;
uniform float inAspect;
uniform float inBackgroundFade;

out vec4 color;

void main() {
	vec2 staruv =  uv * vec2(16.0/9.0, 1.0);
	vec3 star = texture(inStarTex, 
		vec2(staruv.x + inScrollX * 5.0, staruv.y)).xyz;
	star += texture(inStarTex, 
		vec2(staruv.x + inScrollX * 3.0 + 0.2, staruv.y+0.1)).xyz * 0.5;

	vec2 uv2 = vec2(uv.x*inAspect*1.3, uv.y);
	uv2.x += inScrollX;
	vec3 q = texture(inTex, uv2).xyz;
		q -= vec3(inBackgroundFade);
		q = max(vec3(0.0), q);
	q*=q;
	q *= 0.6;
	star *= star;
	star *= 0.5;
	//q += star;
	//q = max(vec3(0.0), q - vec3(0.05));
	//q *= 0.5;
	
	q += star;

	
	color = vec4(q, 0.3);

}