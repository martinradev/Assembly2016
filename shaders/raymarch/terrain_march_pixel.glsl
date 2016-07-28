


// required for generating rays
uniform vec2 windowSize;
uniform vec3 center;
uniform vec3 horizontal;
uniform vec3 vertical;
uniform vec3 direction;

varying vec2 uv;

void main()
{

	// generate rays
	float px = 2.0 * gl_FragCoord.x / windowSize.x - 1.0;
	float py = 2.0 * gl_FragCoord.y / windowSize.y - 1.0;
	
	vec3 p0 = center + vec3(0,3,0);
	vec3 d0 = normalize(px * horizontal + py * vertical + direction);

	float d = terrain_raymarch(p0, d0, TMIN, TMAX, 0.6);
	
	if (d < TMAX) {
		
		vec3 p = p0 + d * d0;
		vec3 n = getNormalTerrain(p.x, p.z);
		
		vec3 color;
		
		color = computeColor(p, n);
		
		gl_FragColor = vec4(color, 1.0);

	} else {
		
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		
	}
	
	
}