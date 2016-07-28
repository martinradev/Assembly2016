


float rand(float n){return fract(sin(n) * 43758.5453123);}

float noise(float p){
    float fl = floor(p);
  float fc = fract(p);
    return mix(rand(fl), rand(fl + 1.0), fc);
}

float rand(vec2 n) { 
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

float noise(vec2 n) {
    const vec2 d = vec2(0.0, 1.0);
  vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

// noise stuff
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

    
}


/*
	terrain noise
*/

#define MOD2 vec2(3.07965, 7.4235)

vec2 add = vec2(1.0, 0.0);
float Hash12(vec2 p)
{
	p  = fract(p / MOD2);
    p += dot(p.xy, p.yx+19.19);
    return fract(p.x * p.y);
}

float Noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
    f = f*f*(3.0-2.0*f);
    
    float res = mix(mix( Hash12(p),          Hash12(p + add.xy),f.x),
                    mix( Hash12(p + add.yx), Hash12(p + add.xx),f.x),f.y);
    return res;
}

float tri( in vec2 p )
{

    vec2 q = 2.0*abs(fract(p)-0.5);
    q = q*q*(3.0-2.0*q);
    return -1.0 + q.x + q.y;
 
}