


uniform vec4 diffuseUniform;
uniform vec3 specularUniform;
uniform vec3 cameraPos;
uniform float glossiness;

in vec3 eval_positionVarying;
in vec3 eval_normalVarying;
in vec2 eval_uvVarying;
in float eval_depthVarying;

out vec4 diffuseColor;
out vec4 normal; // normal -> (x y z), w -> u
out vec4 position; // (x y z) -> position, v -> w
out float outDepth;

uniform sampler2D envMap;

uniform vec3 seaColor;


uniform vec3 lightColorUniform;
uniform vec3 lightDirectionUniform;
uniform vec3 lightPosUniform;

uniform sampler2D depthMap;
uniform mat4 toLightScreen;
uniform vec2 screenSize;

uniform vec2 ldSamples[36];

vec3 getMaterial(in vec3 p, in vec3 n) {
	
		vec3 mat;
		vec3 m;
		
		float f = clamp(noise(0.05*p.xz), 0.0, 1.0);
		f += noise(0.1 * p.xz + n.yz * 0.3)*0.85;
		f *= 0.85;

		m = mix(vec3(0.2*f + 0.11, 0.1*f + 0.13, 0.7*f + 0.09), vec3(0.2*f+0.29, 0.3*f+0.27, 0.33*f+0.08), f*0.8);
		mat = m*vec3(f*m.x+0.1, f*m.y+.1, f*m.z+.1);

		// have rock
		float fRock;
		fRock = noise(0.107 * p.zx) * 0.77;
		fRock = clamp(fRock, 0.0, 1.0);
		fRock += noise(1.777 * p.xz) * 0.37;
		fRock = mix(fRock, noise(0.388 * p.zx + n.xy * 0.9), 0.33);
		vec3 rockColor = vec3(0.29, 0.27, 0.19);
		m = mix(vec3(fRock), rockColor, 0.386);
		
		vec3 rock = mix(mat, m, 0.7);
		
	
	
	
		// have algae
		// add some noise only to the green parts
		f = clamp(noise(0.6*p.zx)*0.6, 0.0, 1.0);
		f += noise(2.14 * p.xy + 3.17*p.zx) * 0.5;
		
		//m = mix(vec3(0.29, 0.27, 0.19), vec3(0.29, 0.27, 0.19), f*0.7);
		
		
		vec3 grass = mix(mat,m,0.8);
		
	
	
	mat = mix(rock, grass, smoothstep(0.7,0.8,n.y));

	return 10.0*mat;
	
}

vec3 calcLight( in vec3 p, in vec3 n, in vec3 lightDirection, in vec3 lightPosition, in vec3 lightColor )
{
	
	float q = clamp(dot(n, -lightDirection), 0.2, 1.0);
	float l = distance(lightDirection, p);
	float l2 = l*l;
	return q * lightColor / (0.0000015 * l2 + 0.0001 * l + 0.003);
	
}

float getShadowI(in vec3 p) {
	vec4 lwPos = toLightScreen * vec4(p, 1.0);
	lwPos /= lwPos.w;
	lwPos.xy = 0.5* lwPos.xy + vec2(0.5);
	
	float pcf = 0.0;
	
	float rnd = fract(sin(dot(p, vec3(18.9898,99.233,41.164))) * 43758.5453);
	int off = int(rnd * 27.0);
	
	for (int i = 0; i < 9; ++i) {
		vec3 nearestPos = texture(depthMap, lwPos.xy+ldSamples[off+i]/screenSize).xyz;
	
		float l1 = distance(nearestPos, lightPosUniform);
		float l2 = distance(p, lightPosUniform);
		
		float bias =15.0;
		if (l2 < l1+bias) {
			pcf += 1.0;
		}
	}
	pcf /= 9.0;

	return pcf;
}

void main( )
{
	
    vec4 color = vec4(getMaterial(eval_positionVarying, vec3(0)), 1);
	

	float s = exp(-0.0029*eval_depthVarying);
	
	
	
	color.rgb *= calcLight(eval_positionVarying, eval_normalVarying, lightDirectionUniform, lightPosUniform, lightColorUniform);
	float pcf = getShadowI(eval_positionVarying);
	color.rgb = mix(color.rgb, 0.1*color.rgb, 1.0 - pcf);
	color.rgb = mix(seaColor, color.rgb, s);
	diffuseColor = color;
	
	
	
	position = vec4(eval_positionVarying, eval_uvVarying.s);
	normal = vec4(eval_normalVarying, eval_uvVarying.t);
	outDepth = eval_depthVarying;
	
}