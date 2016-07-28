#version 430

layout(isolines, equal_spacing) in;

in float tEval[];

uniform mat4 normalToCamera;
uniform mat4 toScreen;


vec3 interpolate3(in vec3 v1, in vec3 v2, in vec3 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}
	
vec2 interpolate2(in vec2 v1, in vec2 v2, in vec2 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}

vec3 catmullRom

void main( )
{
    vec3 p = interpolate3(tcPositionVarying[0], tcPositionVarying[1], tcPositionVarying[2]);
	vec2 uv = interpolate2(tcUvVarying[0], tcUvVarying[1], tcUvVarying[2]);
	
	p.y = terrainHeight(p.xz, uv);
	gl_Position = toScreen * vec4(p, 1);
	
	eval_positionVarying = p;
	eval_uvVarying = uv;
	eval_depthVarying = gl_Position.z;
	eval_normalVarying = vec3(0.0);
}