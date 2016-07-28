


uniform vec2 windowSize;

varying vec2 uv;



void main()
{
		
		vec3 color;
		
		float dy = mod(uv.y, 0.33);
		
		float s = clamp(step(0.32,dy) - step(0.33,dy), 0.78,0.89);
		
		color = vec3(s);
		
		color += vec3(0.15 * noise(uv * 12.6));
		
		gl_FragColor = vec4(color, 1.0);

	
	
}