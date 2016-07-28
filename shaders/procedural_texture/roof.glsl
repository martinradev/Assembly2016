


// required for generating rays
uniform vec2 windowSize;

varying vec2 uv;



void main()
{

		
		vec2 p = uv * 4.137;
		
		float fbm = 0.0;
		
		fbm += 0.7 * noise(p);
		fbm += 0.3 * noise(p*2.0);
		fbm -= 0.55 * noise(p*4.0);
		fbm += 0.125 * noise(p*0.04);
		fbm += 0.125 * noise(p*1245.0);
		
		//fbm = bumpFunction(vec3(p,0.0));
		float f = clamp(0.5 * fbm + 0.5, 0.0, 1.0);
		
		gl_FragColor = vec4(vec3(f), 1.0);

	
	
}