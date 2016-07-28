


// required for generating rays
uniform vec2 windowSize;

varying vec2 uv;



void main()
{

		
		
		float p = mod(uv.x, 0.25);
		
		
		float s = step(0.24, p) - step(0.25, p);
		float t = 1.0 - step(0.05, uv.y);
		t = max(step(0.95, uv.y), t);
		
		s = max(s,t);
		
		float q = (1.0-t)*smoothstep(0.23,0.25, p-0.01);
		
		s = clamp(s, 0.88, 1.0);
		
		vec3 color = vec3(0.96,0.98,0.98) * s;
		
		color = mix(color, vec3(q), 0.4);
		
		gl_FragColor = vec4(color, 1.0);

	
	
}