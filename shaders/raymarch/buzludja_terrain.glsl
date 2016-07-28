#undef TMAX
#define TMAX 250.0
uniform sampler2D terrainSampler;


/*
float fHeight(float x, float z) {
	//if (x > 100.0 || x < 0.0) return TMAX;
	//if (z > 100.0 || z < 0.0) return TMAX;
	
	float fbm = 0.0;
	fbm += noise(vec2(x,z));
	fbm += 0.25*noise(vec2(x,z)*2.0);
	float f = 1.0;
	if (x > 62 || x < 40 || z > 60 || z < 48) {
		f = 0.25;
	}
	
	fbm = mix(fbm, 0.0, f);
	return 42.0*texture(terrainSampler, vec2(x,z)*0.01).r - fbm;
}*/

float fHeight(float x, float z) {
	
	if (x > 510.0 || x < -1.0) return 0.0;
	if (z > 510.0 || z < -1.0) return 0.0;
	
	
	
	/*float fbm = 0.0;
	
	fbm += 42.0*noise(0.01 * vec2(x,z));
	fbm += 5.0*noise(0.001 * vec2(x,z));
	fbm += 11.0*tri(0.005 * vec2(x,z));
	fbm += 21.0*noise(0.02 * vec2(z,x));
	fbm += 1.0 * noise(0.2 * vec2(z , x));
	
	
	return fbm*2;*/
	
	

	const mat2 m2 = mat2(1.8,-1.8,1.2,1.6);
	vec2 p = vec2(x,z) * 0.0023;

    float s = 1.0;
	float t = 0.0;
	for( int i=0; i<6; ++i )
	{
        t += s*tri(p);
		s *= 0.445 + 0.108*t; // 0.445, 0.108
        p = 0.916*m2*p + (t-0.5)*0.324; // 0.916 0.324
		t += 0.438*s*tri(vec2(p.y,p.y));
	}
 
	
    return t*65.0;
	
}


float fScene(in vec3 p) {
	return fHeight(p.x, p.z);
}

vec3 getMaterial(in vec3 p, in vec3 n) {
	
	vec3 mat;
	vec3 m;
	
	float f = clamp(noise(0.05*p.xz), 0.0, 1.0);
	f += noise(0.1 * p.xz + n.yz * 0.3)*0.85;
	f *= 0.85;

	m = mix(vec3(0.6*f + 0.11, 0.5*f + 0.13, 0.7*f + 0.09), vec3(0.4*f+0.12, 0.49*f+0.14, 0.33*f+0.08), f*0.8);
	mat = m*vec3(f*m.x+0.28, f*m.y+.20, f*m.z+.22);
	

	
	
	
		// have rock
		float fRock;
		fRock = noise(0.237 * p.zx) * 0.77;
		fRock = clamp(fRock, 0.0, 1.0);
		fRock += noise(1.777 * p.xz) * 0.37;
		fRock = mix(fRock, noise(0.388 * p.zx + n.xy * 0.9), 0.33);
		vec3 rockColor = vec3(0.564, 0.282, 0.062);
		m = mix(vec3(fRock), rockColor, 0.386);
		
		vec3 rock = mix(mat, m, 0.7);
		
	
	
	
		// have grass
		// add some noise only to the green parts
		f = clamp(noise(0.6*p.zx)*0.6, 0.0, 1.0);
		f += noise(2.14 * p.xy + 3.17*p.zx) * 0.7;
		
		m = mix(vec3(0.051,0.096,0.034), vec3(0.131,0.273,0.055), f*0.7);
		
		
		vec3 grass = mix(mat,m,0.7);
		
	
	
	mat = mix(rock, grass, smoothstep(0.65,0.8,n.y));
	
	float h = noise(p.xz*0.3)*7 + 3.0 * noise(p.zx*0.3);
	if (p.y + h > 55.0 && n.y > 0.13) {

		float snow = clamp((p.y - 40.0 - noise(p.xz * .1)*8.0) * 0.041, 0.0, 1.0);
		mat = mix(mat, vec3(.7,.7,.8), snow);

	}

	return mat;
	
}

vec3 computeColor(in vec3 p, in vec3 n) {
	
	vec3 objectDiffuseColor = getMaterial(p,n);
	return objectDiffuseColor;
	
	vec3 lightDir1 = vec3(0,-1,0);
	vec3 lightColor1 = vec3(1);
	
	vec3 lightDir2 = normalize(knob6.xyz);
	vec3 lightColor2 = knob7.xyz;
	
	vec3 ambient = vec3(0.5);
	
	
	float lFactor1 = clamp(dot(n, -lightDir1), 0.0, 1.0);
	float lFactor2 = clamp(dot(n, -lightDir2), 0.0, 1.0);
	
	vec3 diffuse = (lightColor1 * lFactor1 + lightColor2 * lFactor2);
	
	return (diffuse + ambient) * objectDiffuseColor;

}
