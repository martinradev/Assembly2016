#version 430

layout(triangles, equal_spacing, ccw) in;

in vec3 tcPositionVarying[];

uniform mat4 toScreen;
uniform float time;

out vec3 eval_positionVarying;
out float eval_depthVarying;

vec3 interpolate3(in vec3 v1, in vec3 v2, in vec3 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}

float terrainHeight(in vec2 inP) {
	
	return 60.0*sin(dot(inP, inP) - time);

}

void main( )
{
    vec3 p = interpolate3(tcPositionVarying[0], tcPositionVarying[1], tcPositionVarying[2]);
	
	vec2 inP = 0.0016*p.xz;
	p.y = terrainHeight(inP)+1550.0;
	
	gl_Position = toScreen * vec4(p, 1);
	eval_positionVarying = p;
	eval_depthVarying = gl_Position.z;
}