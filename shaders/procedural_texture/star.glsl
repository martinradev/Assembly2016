


// required for generating rays
uniform vec2 windowSize;

varying vec2 uv;



void main()
{

		
		vec2 p = uv * knob1.x;
		
		float fbm = 0.0;
		
		fbm += 0.5 * noise(p);
		fbm += 0.25 * noise(p*0.2);
		fbm -= 0.55 * noise(p*0.02);
		fbm += 0.125 * noise(p*0.04);
		fbm += 0.125 * noise(p*1245.0);
		float f = clamp(0.5 * fbm + 0.5, 0.0, 1.0);
		
		gl_FragColor = vec4(vec3(f), 1.0);

	
	
}