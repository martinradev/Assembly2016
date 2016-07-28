

vec3 getNormal(in vec3 p) {
	
	vec3 NEPS = vec3(0.0001, 0.0, 0.0);
	
	vec3 delta = vec3(
					fScene(p + NEPS) - fScene(p - NEPS),
					fScene(p + NEPS.yxz) - fScene(p - NEPS.yxz),
					fScene(p + NEPS.yzx) - fScene(p - NEPS.yzx)
				);
	
	return normalize(delta);
	
}

vec3 getNormalTerrain(in float x, in float z) {
	const float eps = 0.6;
	const vec3 n = vec3( fHeight(x-eps,z) - fHeight(x+eps,z),
                         2.0f*eps,
                         fHeight(x,z-eps) - fHeight(x,z+eps) );
    return normalize( n );
}

float raymarch(vec3 p0, vec3 d0, float tmin, float tmax) {
	float t = tmin;
	for (int i = 0; i < 256; ++i) {
		vec3 p = p0 + d0 * t;
		float h = fScene(p);
		if (h <= EPS || t > tmax) {
			break;
		}
		t += h*0.1 ; // not taking full steps due to noise
	}
	return t;
}

float terrain_raymarch(vec3 p0, vec3 d0, float tmin, float tmax, float step) {

	float t = tmin;
	
	for (; t < tmax; t+=step) {
		vec3 p = p0 + d0 * t;
		float h = fHeight(p.x, p.z);
		
		if (p.y < h) {
			// go back
			
			return t - 0.5*step;
		}
	
	}
	
	return t;
}

// only for debugging
float raymarch_Debug(vec3 p0, vec3 d0, float tmin, float tmax) {
	float t = tmin;
	for (int i = 0; i < 256; ++i) {
		vec3 p = p0 + d0 * t;
		float h = fPlane(p, vec3(0,1,0), 0.0);
		if (h <= EPS || t > tmax) {
			break;
		}
		t += h;
	}
	return t;
}
