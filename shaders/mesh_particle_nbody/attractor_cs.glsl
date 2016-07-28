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

layout(binding = 1, std430) buffer AttractorArray {
	
	Attractor attractors[];
	
};

uniform int numParticles;
uniform vec4 knob1;
uniform float dtUniform;
uniform float dtPrevUniform;


vec3 verlet(in vec3 a, in vec3 x, in vec3 xOld, in float dt, in float dtprev) {
	
	return x + (x - xOld) * (dt / dtprev) + a * dt*dt;
	
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
	return fTorus(p, 3.0, 10.0);
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

	int index = int(gl_LocalInvocationIndex)*PARTICLES_PER_THREAD ;
	
	index = PARTICLES_PER_THREAD * int(gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x);
	
	if (index >= numParticles) return;
	
	Attractor attr = attractors[0];
	
	const float G = 9.8;
	
	for (int i = 0; i < PARTICLES_PER_THREAD; ++i)
	{
		
		ParticleInfo particle = particles[i + index];
		
		vec3 acc = vec3(0.0);

		vec3 dirToParticle = (attr.data.xyz-particle.position.xyz);
			
		float r = length(dirToParticle);
		
		acc += dirToParticle * attr.data.w / pow(dot(dirToParticle,dirToParticle) + 1.0, 1.5);
		
		acc += knob1.x*curlNoise(particle.position.xyz)*sign(dtUniform);
		
		const float STEP_CONST = 0.003;
		//acc = -0.5*getNormal(particle.position.xyz);
		vec3 newPos = verlet(acc, particle.position.xyz, particle.data.yzw, STEP_CONST*dtUniform, STEP_CONST*dtPrevUniform);
		
		particles[i+index].data.yzw = particle.position.xyz;
		particles[i+index].position.xyz = newPos;

	}

	

}