#version 450

layout(local_size_x = 128) in;

struct ParticleInfo {
	
	vec4 position; // modify only the position;
	vec4 normal;
	vec4 data;
	
};

struct Attractor {
	
	vec4 data; // xyz -> position, w -> mass
	
};

layout(binding = 0, std430) buffer ParticleArray {
	
	ParticleInfo particles[];
	
};

uniform int numParticles;
uniform float dtUniform;


vec3 verlet(in vec3 a, in vec3 x, in vec3 xOld, in float dt) {
	
	return 2.0*x - x + 0.5* a * dt*dt;
	
}

float hash( float n ){

    return fract(sin(n)*43758.5453);
}

float hashNoise3D( in vec3 x ){

    vec3 p = floor(x);
    vec3 f = fract(x);//smoothstep(0., 1., fract(x));

    f = f*f*(3.0-2.0*f);//Smoothstep
    //f = f*f*f*(10.0+f*(6.0*f-15.0));//Smootherstep
   
    float n = p.x*7.0 + p.y*57.0 + 111.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  7.0),f.x),
                   mix( hash(n+ 57.0), hash(n+ 64.0),f.x),f.y),
               mix(mix( hash(n+111.0), hash(n+118.0),f.x),
                   mix( hash(n+168.0), hash(n+175.0),f.x),f.y),f.z);

    
    /*
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+ 157.0), hash(n+ 158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
    */ 
    
}

vec3 curlNoise(in vec3 v)
{

	vec3 curl;
	
	float eps = 0.5;
	float eps2 = 2.0*eps;
	vec3 delta = vec3(eps,0,0);
	float n1,n2;
	float a,b;
	
	n1 = hashNoise3D(v + delta.yxz);
	n2 = hashNoise3D(v - delta.yxz);
	a = (n1-n2) /eps2;
	
	n1 = hashNoise3D(v + delta.zyx);
	n2 = hashNoise3D(v - delta.zyx);
	b = (n1-n2) /eps2;
	
	curl.x = a-b;
	
	n1 = hashNoise3D(v + delta.zyx);
	n2 = hashNoise3D(v - delta.zyx);
	a = (n1-n2) /eps2;
	
	n1 = hashNoise3D(v + delta.xyz);
	n2 = hashNoise3D(v - delta.xyz);
	b = (n1-n2) /eps2;
	
	curl.y = a-b;
	
	n1 = hashNoise3D(v + delta.xyz);
	n2 = hashNoise3D(v - delta.xyz);
	a = (n1-n2) /eps2;
	
	n1 = hashNoise3D(v + delta.yxz);
	n2 = hashNoise3D(v - delta.yxz);
	b = (n1-n2) /eps2;
	
	curl.z = a-b;
	
	return normalize(curl);
}

float fTorus(vec3 p, float smallRadius, float largeRadius) {
	return length(vec2(length(p.xz) - largeRadius, p.y)) - smallRadius;
}

float fScene(in vec3 p) 
{
	return fTorus(p, 30.0, 300.0);
}

vec3 getNormal(in vec3 p) {
	
	vec3 NEPS = vec3(0.0001, 0.0, 0.0);
	
	vec3 delta = vec3(
					fScene(p + NEPS) - fScene(p - NEPS),
					fScene(p + NEPS.yxz) - fScene(p - NEPS.yxz),
					fScene(p + NEPS.yzx) - fScene(p - NEPS.yzx)
				);
	
	return normalize(delta);
	
}

void main() {

	const int PARTICLES_PER_THREAD = 1;

	int index = PARTICLES_PER_THREAD * int(gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x);
	
	if (index >= numParticles) return;

	
	const float G = 9.8;
	
	for (int i = 0; i < PARTICLES_PER_THREAD; ++i)
	{
		
		ParticleInfo particle = particles[i + index];
		
		const float STEP_CONST = 100.0;
		vec3 acc = -0.5*getNormal(particle.position.xyz);
		acc += 0.1*curlNoise(acc);
		vec3 newPos = verlet(acc.zxy, particle.position.xyz, particle.data.yzw, STEP_CONST*dtUniform);
		
		particles[i+index].data.yzw = particle.position.xyz;
		particles[i+index].position.xyz = newPos;

	}

	

}