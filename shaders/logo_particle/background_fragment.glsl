#version 430

in highp vec2 uv;

out vec4 color;

float vmax(vec2 v) {
	return max(v.x, v.y);
}

float vmax(vec3 v) {
	return max(max(v.x, v.y), v.z);
}

float vmax(vec4 v) {
	return max(max(v.x, v.y), max(v.z, v.w));
}

float vmin(vec2 v) {
	return min(v.x, v.y);
}

float vmin(vec3 v) {
	return min(min(v.x, v.y), v.z);
}

float vmin(vec4 v) {
	return min(min(v.x, v.y), min(v.z, v.w));
}

float fBox(vec3 p, vec3 b) {
	vec3 d = abs(p) - b;
	return length(max(d, vec3(0))) + vmax(min(d, vec3(0)));
}

uniform vec4 knob1;
uniform vec4 knob2;

void main( )
{
	const vec2 center = vec2(0.5);
	float f = fBox(vec3(uv-center, 0.0), vec3(0.1,0.1,0));
	float q = smoothstep(0.3, 0.9, f);
	vec3 cl1 = knob1.xyz;
	vec3 cl2 = knob2.xyz;
	vec3 cl = mix(cl1,cl2,q);
	
	color = vec4(cl, 1.0);
	
}