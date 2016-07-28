#version 430

uniform vec3 transDirection;

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

void main() {
   
 
	uint threadIndex = gl_GlobalInvocationID.x;
	if (threadIndex >= numParticles) return;
	
	particles[threadIndex].position.xyz = particles[threadIndex].orig_position.xyz+transDirection;
	


}