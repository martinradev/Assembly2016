#version 430

layout(triangles, equal_spacing, ccw, point_mode) in;

in vec2 tcPositionVarying[];
in vec2 tcUvVarying[];

uniform mat4 lightScreenToWorld;
uniform mat4 worldToLightScreen;
uniform mat4 toScreen;

uniform sampler2D terrainPosTex;
uniform sampler2D waterPosTex;
uniform sampler2D waterNormalTex;
uniform vec3 lightPosition;
uniform float znear;
uniform float zfar;


out vec3 eval_positionVarying;
out float eval_dWater;
out float eval_depthVarying;

vec3 interpolate3(in vec3 v1, in vec3 v2, in vec3 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}
	
vec2 interpolate2(in vec2 v1, in vec2 v2, in vec2 v3) {
	return v1 * gl_TessCoord.x + v2 * gl_TessCoord.y + v3 * gl_TessCoord.z;
}

vec3 estimateIntersection( in vec3 v, in vec3 r) {
	
	vec3 p1 = v + r;
	vec4 texPt = worldToLightScreen * vec4(p1, 1);
	vec2 tc = vec2(0.5 * texPt.xy / texPt.w) + vec2(0.5);
	//tc.y = 1.0 - tc.y; // ?
	vec4 recPos = texture(terrainPosTex, tc);
	vec3 p2 = v + distance(recPos.xyz, v) * r;
	texPt = worldToLightScreen * vec4(p2, 1);
	tc = vec2(0.5 * texPt.xy / texPt.w) + vec2(0.5);
	//tc.y = 1.0 - tc.y; // ?
	return texture(terrainPosTex, tc).xyz;
}

vec3 rayGeoNP(in vec3 v, in vec3 r) {
	
	const float eps = 0.1;

	float xk = 0.1;
	vec2 tc;
	vec4 texPt;
		
	for (int i = 0; i < 8; ++i) {
		
		vec3 p1 = v + r * xk;
		vec4 texPt = worldToLightScreen * vec4(p1, 1);
		tc = vec2(0.5 * texPt.xy / texPt.w) + vec2(0.5);
		//tc.y = 1.0 - tc.y; // ?
		vec4 recPos = texture(terrainPosTex, tc);
		float fxk = distance(p1, recPos.xyz);
		
		vec3 p2 = v + r * (xk + eps);
		texPt = worldToLightScreen * vec4(p2, 1);
		tc = vec2(0.5 * texPt.xy / texPt.w) + vec2(0.5);
		//tc.y = 1.0 - tc.y; // ?
		vec3 newPos = texture(terrainPosTex, tc).xyz;
		float fxk_eps = distance(newPos, p2);
		
		float deriv = (fxk_eps - fxk) / eps;
		xk = xk - (fxk / deriv);
		
	}

	/*texPt = worldToLightScreen * vec4(v + r * xk, 1);
	tc = vec2(0.5 * texPt.xy / texPt.w) + vec2(0.5);
	tc.y = 1.0 - tc.y; // ?
	return texture(terrainPosTex, tc).xyz;*/
	
	return v + r * xk;
}

float getParticleSize(in float d) {
	const float smin = 5.0;
	const float smax = 30.0;
	float a = smax - zfar * (smax - smin) / (zfar - znear);
	float b = znear * zfar * (smax - smin) / (zfar - znear);
	return a + b / d;
}

void main( )
{
    vec2 p = interpolate2(tcPositionVarying[0], tcPositionVarying[1], tcPositionVarying[2]);
	vec2 uv = interpolate2(tcUvVarying[0], tcUvVarying[1], tcUvVarying[2]);
	
	vec3 v = texture(waterPosTex, uv).rgb;
	vec3 n = texture(waterNormalTex, uv).rgb;
	vec3 lightDir = normalize(v - lightPosition);
	const float waterRefrIndex = 1.3807;
	const float airRefrIndex = 1.0;
	const float refrRatio = airRefrIndex / waterRefrIndex;
	vec3 r = normalize(refract(lightDir, n, refrRatio));
	
	
	//vec3 pointPos = rayGeoNP(v, r);
	vec3 pointPos = estimateIntersection(v,r );
	gl_Position = toScreen * vec4(pointPos, 1);
	gl_PointSize = clamp(getParticleSize(0.01*gl_Position.z), 5.0, 30.0);
	
	eval_positionVarying = pointPos;
	eval_dWater = distance(v, pointPos);
	eval_depthVarying = gl_Position.z;
}