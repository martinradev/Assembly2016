#version 430

in vec2 uv;

uniform sampler2D positionMap;
uniform sampler2D depthMap;

uniform vec3 lightPos;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 camPos;

uniform mat4 toLightClip;
uniform mat4 toInvClip;
out vec4 outColor;

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
	
	const int NUM_STEPS = 300;
	
	
	vec4 pos = texture(positionMap, uv);
	
	vec3 dir;
	if (pos.w == 1.0) {

		vec2 xy = 2.0*uv - vec2(1.0);
			
		vec4 p0 = vec4(xy.x, xy.y, 0.0, 1.0);
		vec4 p1 = vec4(xy.x, xy.y, 1.0, 1.0);
			
		vec4 orig = (toInvClip) * p0;
		orig /= orig.w;
			
		vec4 cdir = (toInvClip) * p1;
		cdir /= cdir.w;
			
		dir = cdir.xyz - orig.xyz;
		dir *= 0.69;
	} else {
		dir = (pos.xyz-camPos);
	}
	float dd = length(dir);
	
	vec3 cPos;
	
	float numShadowed = 0.0;
	
	vec3 lv = vec3(0.0);
	vec3 startPos = camPos + (0.5 * hashNoise3D(0.0001*dir) + 0.5) * dir / float(NUM_STEPS) ;
	
	for (int i = 0; i < NUM_STEPS; ++i)
	{
		float off = smoothstep(0.0, float(NUM_STEPS), float(i));
		off = float(i) / float(NUM_STEPS);
		cPos = startPos + off * dir;
		
		
		vec4 posInLightClip = toLightClip * vec4(cPos, 1.0);
		posInLightClip /= posInLightClip.w;
		posInLightClip.xy = 0.5*posInLightClip.xy + vec2(0.5);
		
		if (posInLightClip.x < 0.0 || posInLightClip.x > 1.0 || posInLightClip.y < 0.0 || posInLightClip.y > 1.0 || posInLightClip.z < 0.0 || posInLightClip.z > 1.0) continue;
		
		posInLightClip.xy = clamp(posInLightClip.xy, vec2(0.1), vec2(0.9));
		
		vec3 realPosDepthTex = texture(depthMap, posInLightClip.xy).xyz;
		float l1 = distance(cPos, lightPos);
		float l2 = distance(realPosDepthTex, lightPos);
		if (l1 < l2+20.0) {
			vec3 lDir = (cPos-lightPos);
			lDir /= l1;
			float cosD = clamp(dot(lDir, lightDir), 0.0, 1.0);
			float q1 = exp(-0.0000001*off*dd);
			float q2 = exp(-0.045*sqrt(l1));
			lv += cosD * lightColor * q2 * q1 ;
		}
		else {
			//lv -= vec3(0.000007);
		}

	}

	outColor = vec4(lv, 1.0);
	
}