
layout(triangles, equal_spacing, ccw) in;

in vec3 tcPositionVarying[];
in vec3 tcNormalVarying[];
in vec4 tcColorVarying[];
in vec2 tcTexCoordVarying[];

uniform mat4 toCamera;
uniform mat4 normalToCamera;
uniform mat4 toScreen;

uniform vec3 dispUniform;

out vec4 eval_colorVarying;
out vec3 eval_positionVarying; // world space
out vec3 eval_normalVarying; // world space
out vec2 eval_texCoordVarying;

vec3 interpolate3(in vec3 v1, in vec3 v2, in vec3 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}
	
vec2 interpolate2(in vec2 v1, in vec2 v2, in vec2 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}

void main( )
{
    vec3 p = interpolate3(tcPositionVarying[0], tcPositionVarying[1], tcPositionVarying[2]);
	vec3 n = normalize(interpolate3(tcNormalVarying[0], tcNormalVarying[1], tcNormalVarying[2]));
	
	
	float disp = tetraVoronoi(p+dispUniform)+dispUniform.z;
	p += disp * n;
	

	eval_colorVarying = vec4(1);
	eval_positionVarying = p;
	eval_normalVarying = n;
	eval_texCoordVarying = interpolate2(tcTexCoordVarying[0], tcTexCoordVarying[1], tcTexCoordVarying[2]);
	
	gl_Position = toScreen * vec4(p, 1);
}