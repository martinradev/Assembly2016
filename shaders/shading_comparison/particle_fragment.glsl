
in vec3 positionVarying;
in vec3 normalVarying;
in float ageVarying;

out vec4 color;
out vec4 normal; // normal -> (x y z), w -> u
out vec4 position; // (x y z) -> position, v -> w
out float depth;

uniform float ageUniform;
uniform float ssaoMask;

#define PI 3.14159265

void main() {
	
	
	
	normal = vec4(0.5 * normalVarying + vec3(0.5), 0);
	
	
	
	vec3 pointLightPos = vec3(0, 20, 10);
	
	float ndotl = clamp(dot(normalVarying, normalize(pointLightPos-positionVarying)), 0.1, 1.0);
	
	vec3 cl = mix(vec3(0.7, 0.4, 0.), normal.xyz, 0.4);
	
	color = mix(vec4(ndotl * cl, 1.5), vec4(0), vec4(smoothstep(0.0,1.0,ageUniform)));
	
	//color = vec4(1,0.7,0.6,1);
	
	//color.xyz = ;
	position = vec4(positionVarying, 0);
	normal.w = ssaoMask;
	depth = gl_FragCoord.z / gl_FragCoord.w;
}