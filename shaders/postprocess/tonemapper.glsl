#version 430

out vec4 color;
uniform sampler2D inImage;
uniform sampler2D bloomImage;
varying vec2 uv;

uniform float exposure;
uniform float blurOut;

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

void main()
{
	float distFromCenter = length(uv - vec2(0.5));
	
	vec3 cl = texture2D(inImage, uv).xyz + texture2D(bloomImage, uv).xyz;

	cl *= exposure;
	
	float m = max(cl.r, max(cl.g, cl.b));
	if (m > 1.0) cl += 0.2*vec3(m - 1.0);
	
	cl = cl / (vec3(1.0) + cl);
	
	
	
	cl = pow(cl, vec3(1.0 / 2.2));
	
	cl += hashNoise3D(cl)/6.0;
	
	float q = smoothstep(0.0, blurOut, distFromCenter);
	cl = mix(cl, vec3(0.0), q);

	color = vec4(cl, 1.0);

	
}