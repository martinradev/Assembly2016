#version 430

in vec2 uv;

uniform sampler2D inTex;
uniform sampler2D inStarTex;
uniform float inScrollX;
uniform float inAspect;

out vec4 color;

void main() {
	vec2 staruv =  uv * vec2(16.0/9.0, 1.0);
	staruv.x += inScrollX * 5.0;
	vec3 star = texture(inStarTex, staruv).xyz;
	vec2 uv2 = vec2(uv.x*inAspect*1.3, uv.y);
	uv2.x += inScrollX;
	vec3 q = texture(inTex, uv2).xyz;
	q*=q;
	q *= 1.0;
	star *= star;
	//q += star;
	//q = max(vec3(0.0), q - vec3(0.05));
	//q *= 0.5;
	
	q += star;
	color = vec4(q, 0.3);

}