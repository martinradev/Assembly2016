#version 430

in vec3 positionAttrib;
in vec3 normalAttrib;
in vec4 vcolorAttrib;
in vec2 texCoordAttrib;

out float depthVarying;

uniform mat4 toScreen;

uniform bool bounceEffect;
uniform float bounceTime;
uniform float bouncePeriod;
uniform float bounceScale;
uniform float bounceColorBoost;

void main()
{
	
	vec3 n = normalize(normalAttrib);
	
	vec3 dp;
	if (bounceEffect && dot(n, vec3(0,1,0)) > 0.5) {
		vec3 p = positionAttrib;
		
		float sMax = 1-smoothstep(0.9, 1.0, texCoordAttrib.s);
		float tMax = 1-smoothstep(0.9, 1.0, texCoordAttrib.t);
		float sMax2 = smoothstep(0.0, 0.1, texCoordAttrib.s);
		float tMax2 = smoothstep(0.0, 0.1, texCoordAttrib.t);
		
		float l = length(p);
		float h = bounceScale;
		float lmax = step(31.0, l);
		dp = positionAttrib + n*(h*sin((l + bounceTime) / bouncePeriod)*sMax*tMax*sMax2*tMax2*lmax);
	} else {
		dp = positionAttrib;
	}

	gl_Position = toScreen * vec4(dp, 1.0);
	
	depthVarying = gl_Position.z;

}
