
layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

struct Particle {
	vec4 position; 
	vec4 direction; // position.w age
	vec4 normal;
	vec4 orig_position;
};

layout(std430, binding = 0) buffer Particles {
	Particle particles[];
};


uniform uint numParticles;
uniform float sceneType;

void main() {
   
 
	uint threadIndex = gl_GlobalInvocationID.x;
	if (threadIndex >= numParticles) return;
	
	Particle particle = particles[threadIndex];
	
	vec3 orig = particle.position.xyz;
	vec3 origDir = particle.direction.xyz;
	vec3 dir =2.2*origDir;
	vec3 particleN = particle.normal.xyz;
	float age = particle.position.w;
	
	float u,v,t=1.0;
	int idx;
	bool isOccluded = bvh_intersect_bitstack(orig, dir, u, v, t, idx, true);
	if (!isOccluded) {
		
		// update position
		particles[threadIndex].position.xyz += dir;

	} else {
		// bounce
		Triangle trig = triangle_buffer.triangles[idx];
		vec3 n = getTrigNormal(trig);
		if (dot(origDir, n) > 0.0) n = -n;
		
		particles[threadIndex].direction.xyz = reflect(origDir, n);
		particles[threadIndex].normal.xyz = reflect(particleN, n);
	}
	


}