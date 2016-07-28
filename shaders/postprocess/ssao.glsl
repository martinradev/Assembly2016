#version 430

out vec4 color;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap; // world space
uniform sampler2D depthMap;
uniform sampler2D positionMap; // world space
uniform sampler2D shadowMapSampler; 
varying vec2 uv;

uniform mat4 toCamera;
uniform mat4 normalToCamera;
uniform mat4 toScreen;
uniform mat4 toLightClip;
uniform int numSSAOSamples;

uniform vec2 ldSamples[16];
uniform float blurOut;

layout(std430, binding = 0) buffer SSAOKernel {
	vec4 ssaoSamples[];
};

mat3 formBasis(in vec3 n) {

	vec3 tmpn = abs(n);
	
	if (tmpn.x <= tmpn.y && tmpn.x <= tmpn.z) {
		tmpn.x = 1.0;
	} else if(tmpn.y <= tmpn.x && tmpn.y <= tmpn.z) {
		tmpn.y = 1.0;
	} else {
		tmpn.z = 1.0;
	}
	
	tmpn = normalize(cross(tmpn, n));
	
	mat3 basis;
	basis[0] = tmpn;
	basis[1] = cross(n, tmpn);
	basis[2] = n;
	
	return basis;
}

void main()
{
	vec3 pos = texture(positionMap, uv).xyz;
	vec4 n = texture(normalMap, uv);
	vec3 difColor = texture(diffuseMap, uv).xyz;
	
	vec4 posInLightClip = toLightClip*vec4(pos, 1);
	posInLightClip.xy /= posInLightClip.w;
	posInLightClip.xy = 0.5 * posInLightClip.xy + vec2(0.5);
	
	
	float pcf = 0.0;
	
	float rnd = fract(sin(dot(pos, vec3(18.9898,99.233,41.164))) * 43758.5453);
	int off = int(rnd * 12.0);
	
	for (int i = 0; i < 4; ++i) {
		float texLightDist = texture(shadowMapSampler, posInLightClip.xy+ldSamples[off+i]/600.0).r;
	
		float realDist = posInLightClip.z;
		
		float bias = 0.3;
		if (texLightDist+bias < realDist) {
			pcf += 1.0;
		}
	}
	pcf /= 4.0;
	difColor = mix(difColor, difColor*0.6, pcf);
	
	vec3 finalColor = difColor;
	if (n.w > 0.0) {
		vec3 nFromMap = normalize(2.0 * n.xyz - vec3(1.0));
		
		float occ = 0.0;
		
		

		float posDepth = texture(depthMap, uv).r;
		
		for (int i = 0; i < numSSAOSamples; ++i) {
			
			
			vec3 rvec = normalize(ssaoSamples[i].xyz);
			mat3 tbn = formBasis(nFromMap);

			
			vec3 cSample = pos + tbn*rvec;
			
			vec4 screenPos = toScreen * vec4(cSample, 1);
			screenPos.xy /= screenPos.w;
			
			screenPos.xy = 0.5 * screenPos.xy + vec2(0.5);
			
			float sampleDepth = texture(depthMap, screenPos.xy).r;
			if (sampleDepth <= screenPos.z+0.1) {
				occ += 0.5;
			}
		}
		occ /= numSSAOSamples;
		occ = 1.0 - occ;
		

		vec3 ao = vec3(occ);
		
		
		finalColor = mix(ao,difColor, 0.8);
		
		
	}
	
	float distFromCenter = length(uv - vec2(0.5));
	
	
	// blur out 
	//if (blurOut>0.0) {
		finalColor *= (1.0 - blurOut)*pow((1.3 - distFromCenter), 1.9);
	//}
	
	color = vec4(finalColor, 1);
	//color = vec4(pos, 1);
}