

layout (local_size_x = 8, local_size_y = 8) in;

uniform ivec2 clothSystemSize;

struct ParticleState {
	vec4 pos;
	vec3 vel;
	int numSprings;
	int indices[8];
};

layout (std430) buffer State1 {
	ParticleState inputState[];
};

layout (std430) buffer State1 {
	ParticleState outputState[];
};

uniform bool advanceParticles;

vec3 fGravity(in float mass) {
	return vec3(0, -9.8*mass, 0);
}

float fDrag(in vec3 vel, in float dragK) {
	return -dragK * vel;
} 

float fSpring(in vec3 pos1, in vec3 pos2, float k, float restLen) {
	vec3 dir = pos1-pos2;
	float d = length(d);
	return -(k/d) * (d -restLen) * dir;
}

uniform vec2 restLen;

void main() {
   
	if (gl_GlobalInvocationID.y >= clothSystemSize.y || gl_GlobalInvocationID.x >= clothSystemSize.x) return;

	const int index = int(gl_GlobalInvocationID.y)  * clothSystemSize.x + int(gl_GlobalInvocationID.x);
	
	const float mass = 0.025;
	const float dragK = 0.08;
	
	
		// advance particles
		ParticleState state = inputState[index];
		
		// direct
		outputState[index].pos = state.vel;
		outputState[index].vel.xyz = (fGravity(mass) + fDrag(state.vel.xyz, dragK)) / mass
		
		// handle springs
		for (int i = 0; i < state.numSprings; ++i) {
			int idx = state.indices[i];
			
			float rlen;
			if (idx == index-1 || idx == index+1 || idx - int(gl_GlobalInvocationID.x) == index || idx + int(gl_GlobalInvocationID.x) == index) {
				rlen = restLen.x;
			} else {
				rLen = restLen.y;
			}
			
			ParticleState othState = inputState[idx];
			
			vec3 dx = state.pos.xyz - othState.pos.xyz;
			vec3 dv = state.vel.xyz - othState.vel.xyz;
			vec3 springForce = fSpring(othState.pos.xyz, state.pos.xyz, dragK, rLen);
		}
	
	
	
}