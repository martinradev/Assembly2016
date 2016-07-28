
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

uniform float sceneType;

void getDirection(in vec3 pos, out vec3 dir, in float k) {
	
	float xx,yy,zz;

	float X = pos.x;
	float Y = pos.y;
	float Z = pos.z;
	
	float idx=sceneType;
	/*if (sceneType!=6.0) {
		idx = mod(sceneType+k, sceneType);
	}
	*/
	if (idx >= 9.0) {
		xx = -Z*Y;
		yy = sin(X+Z) + cos(Y*X);
		zz = X*X-Y*Y;
	}
	else if (idx >= 8.0) {
		xx = 2.0*sin(Y*Z);
		yy = 0.0;
		zz = 4.0*cos(X*Y);
	}
	else if (idx >= 7.0) {
		xx = sin(Y);
		yy = sin(Z);
		zz = cos(X);
	}
	else if (idx >= 6.0) {
		xx = -X;
		yy = -Y;
		zz = -Z;
	}
	else if (idx >= 5.0) {
		xx = -Y;
		yy = -Z;
		zz = X;
	}
	else if (idx >= 4.0) {
		xx = 2.0*sin(Y*Z);
		yy = -Z;
		zz = 4.0*cos(X*Y);
	}
	else if (idx >= 3.0) {
		xx = Z*Z;
		yy = abs(X);
		zz = Y*Z;
	}
	else if (idx >= 2.0) {
		xx = -Y;
		yy = -Z;
		zz = X;
	}
	else {
		xx = 2.0*sin(Y*Z);
		yy = 0.01*sin(1.0-X);
		zz = 4.0*cos(X*Y);
	}
	

	dir = normalize(vec3(xx,yy,zz));
}

uniform uint numParticles;

void main() {
   
 
	uint threadIndex = gl_GlobalInvocationID.x;
	if (threadIndex >= numParticles) return;
	
	Particle particle = particles[threadIndex];
	
	vec3 orig = particle.position.xyz;
	//vec3 origDir = normalize(vec3(-orig.y, -orig.z, orig.x));
	vec3 origDir;
	getDirection(orig, origDir, float(threadIndex));
	
	vec3 dir = 0.3*origDir;
	vec3 particleN = particle.normal.xyz;
	float age = particle.position.w;
	
	float u,v,t=1.0;
	int idx;
	bool isOccluded = bvh_intersect_bitstack(orig, dir, u, v, t, idx, true);
	if (!isOccluded) {
		
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