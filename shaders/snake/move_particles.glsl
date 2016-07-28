

layout (local_size_x = 8, local_size_y = 8) in;

struct Particle {
	vec4 pos;
	vec4 n;
};

layout(std430, binding = 0) buffer Particles {
	Particle particles[];
};


mat3 formBasis(in vec3 n, float cv) {

	vec3 tmpn = abs(n);
	
	if (tmpn.x <= tmpn.y && tmpn.x <= tmpn.z) {
		tmpn.x = 1.0;
	} else if(tmpn.y <= tmpn.x && tmpn.y <= tmpn.z) {
		tmpn.y = 1.0;
	} else {
		tmpn.z = 1.0;
	}
	
	tmpn += vec3(time, cv, cv);
	tmpn = normalize(cross(tmpn, n));
	
	mat3 basis;
	basis[0] = cross(n, tmpn);
	basis[1] = n;
	basis[2] = tmpn;
	
	return basis;
}

vec3 getNormal(in Triangle trig) {
	vec3 e1 = trig.v3.xyz - trig.v1.xyz;
	vec3 e2 = trig.v2.xyz - trig.v1.xyz;
	return normalize(cross(e1, e2));
}

void main() {
   
   const float off = 1.0;
	
	uint threadIndex = gl_GlobalInvocationID.x;
	if (threadIndex >= 1) return;
	
	Particle p = particles[threadIndex];
	
	float f = hashNoise3D(p.pos.xyz)*10.0;
	
	mat3 B = formBasis(p.n.xyz, f);
	vec3 dir = normalize(B * vec3(0,0,1));
	
	float u,v,t=1.0;
	int idx;
	bool isOccluded = bvh_intersect_bitstack(p.pos.xyz, 0.1*dir, u, v, t, idx, false);
	
	if (isOccluded) {
		Triangle trig = triangle_buffer.triangles[idx];
		
		vec3 n = getNormal(trig);
		particles[threadIndex].n.xyz = n;
		particles[threadIndex].pos.xyz += 0.01 * n;
	} else {
		particles[threadIndex].pos.xyz += 0.05*normalize(dir);
	}
	
	
	
	
}