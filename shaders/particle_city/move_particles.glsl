#version 450

layout(local_size_x = 128) in;

struct ParticleInfo {
	
	vec4 position; // modify only the position;
	vec4 normal;
	vec4 data;
	
};

layout(binding = 0, std430) buffer ParticleArray {
	
	ParticleInfo particles[];
	
};

uniform int numParticles;
uniform float dtUniform;
uniform int offset;
uniform float curlStep;
uniform float attractorStep;

uniform float invocationModulate;
uniform float invocationScale;

uniform vec3 attractorPosition;

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

vec3 getCurlF(in vec3 pos) 
{
	return curlStep * curlNoise(pos*0.01);
}

vec3 getAttractorF(in vec3 pos)
{
	
	float hdist = abs(pos.y - attractorPosition.y);
	float raddist = distance(pos.xz, attractorPosition.xz);
	
	float randomMass = 1.0 + invocationScale * abs(sin(invocationModulate*float(gl_GlobalInvocationID.x)));
	
	float attractorMass = -600.0 * raddist * attractorStep * randomMass * exp(-0.03*hdist);
	
	vec3 dirToParticle = (attractorPosition-pos);
			
	float r = length(dirToParticle);
			
	return dirToParticle * attractorMass / pow(dot(dirToParticle,dirToParticle) + 1.0, 1.5);

}

vec3 getGravity(in vec3 pos)
{
	return vec3(0.0, -3.0*attractorStep*smoothstep(0.0, 100.0, pos.y), 0.0);
}

vec3 evalF(in vec3 pos)
{
	//return 100.0*curlNoise(0.01*pos);
	
	return getAttractorF(pos) + getGravity(pos) + getCurlF(pos);
}

vec3 euler(in vec3 pos)
{
	return pos + evalF(pos)*dtUniform;
}

vec3 rk(in vec3 x0)
{
	float dt = dtUniform*0.1;
	vec3 k1 = evalF(x0);
	vec3 x1 = x0 + (0.5*dt)*k1;
	vec3 k2 = evalF(x1);
	vec3 x2 = x0 + (0.5*dt)*k2;
	vec3 k3 = evalF(x2);
	vec3 x3 = x0 + dt *k3;
	vec3 k4 = evalF(x3);
	return x0 + dt*(k1+2.0*k2+2.0*k3+k4)/ 6.0;
}

vec3 midpoint(in vec3 x0)
{
	float dt = dtUniform*0.15;
	vec3 f0 = evalF(x0);
	vec3 xm = x0 + (0.5*dt)*f0;
	vec3 fm = evalF(xm);
	return x0 + dt*fm;
}

vec3 integrate(in vec3 pos)
{
	return rk(pos);
}

void main() {

	const int PARTICLES_PER_THREAD = 1;

	int index = offset + PARTICLES_PER_THREAD * int(gl_WorkGroupSize.x * gl_WorkGroupID.x + gl_LocalInvocationID.x);
	
	if (index >= numParticles || index < 0) return;

	
	const float G = 9.8;
	
	for (int i = 0; i < PARTICLES_PER_THREAD; ++i)
	{
		
		ParticleInfo particle = particles[i + index];
	
		
		vec3 newPos = integrate(particle.position.xyz);
		
		particles[i+index].data.yzw = particle.position.xyz;
		particles[i+index].position.xyz = newPos;

	}

	

}