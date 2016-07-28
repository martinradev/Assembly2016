
layout(triangles, equal_spacing, ccw) in;

in vec3 tcPositionVarying[];
in vec2 tcUvVarying[];

uniform mat4 normalToCamera;
uniform mat4 toScreen;


out vec3 eval_positionVarying;
out vec2 eval_uvVarying;
out float eval_depthVarying;
out vec3 eval_normalVarying;

vec3 interpolate3(in vec3 v1, in vec3 v2, in vec3 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}
	
vec2 interpolate2(in vec2 v1, in vec2 v2, in vec2 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}

// Simple 2d noise algorithm contributed by Trisomie21 (Thanks!)
float snoise( vec2 p ) {
	vec2 f = fract(p);
	p = floor(p);
	float v = p.x+p.y*1000.0;
	vec4 r = vec4(v, v+1.0, v+1000.0, v+1001.0);
	r = fract(100000.0*sin(r*.001));
	f = f*f*(3.0-2.0*f);
	return 2.0*(mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y))-1.0;
}

float terrainHeight(in vec2 inP) {
	const int octaves = 6;
	float h = 0.0; // height
	float w = 0.5; // octave weight
	float m = 0.4; // octave multiplier
	for (int i=0; i<12; i++) {
		if (i<octaves) {
			h += w * snoise((inP * m));
		}
		else break;
		w *= 0.5;
		m *= 2.0;
	}
	h += smoothstep(-0.6,1.2,h);
	return 120.0*h - 500.0;

}

vec3 getTerrainNormalNaive(in vec2 inP) {
	const float eps = 0.1;
	const vec3 n = vec3( terrainHeight(vec2(inP.x-eps,inP.y)) - terrainHeight(vec2(inP.x+eps,inP.y)),
                         2.0f*eps,
                         terrainHeight(vec2(inP.x,inP.y-eps)) - terrainHeight( vec2(inP.x,inP.y+eps))) ;
    return normalize( n );
}

void main( )
{
    vec3 p = interpolate3(tcPositionVarying[0], tcPositionVarying[1], tcPositionVarying[2]);
	vec2 uv = interpolate2(tcUvVarying[0], tcUvVarying[1], tcUvVarying[2]);
	
	p.y = terrainHeight(0.01*p.xz);
	gl_Position = toScreen * vec4(p, 1);
	
	eval_positionVarying = p;
	eval_uvVarying = uv;
	eval_depthVarying = gl_Position.z;
	eval_normalVarying = getTerrainNormalNaive(0.01*p.xz);
}