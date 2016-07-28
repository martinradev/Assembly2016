
in vec3 positionAttrib;
in vec3 normalAttrib;
in vec4 vcolorAttrib;
in vec2 texCoordAttrib;

out vec3 positionVarying;
out vec3 normalVarying;
out vec4 colorVarying;
out vec2 texCoordVarying;

uniform mat4 toCamera;
uniform mat4 normalToCamera;
uniform mat4 toScreen;
uniform mat4 toWorld;

uniform bool bounceEffect;
uniform float bounceTime;
uniform float bouncePeriod;
uniform float bounceScale;
uniform float bounceColorBoost;

void main()
{
	
	normalVarying = normalize(normalAttrib);
	
	vec3 dp;
	if (bounceEffect && dot(normalVarying, vec3(0,1,0)) > 0.5) {
		vec3 p = positionAttrib;
		
		float sMax = 1-smoothstep(0.9, 1.0, texCoordAttrib.s);
		float tMax = 1-smoothstep(0.9, 1.0, texCoordAttrib.t);
		float sMax2 = smoothstep(0.0, 0.1, texCoordAttrib.s);
		float tMax2 = smoothstep(0.0, 0.1, texCoordAttrib.t);
		
		float l = length(p);
		float h = bounceScale;
		float lmax = step(31.0, l);
		dp = positionAttrib + normalVarying*(h*sin((l + bounceTime) / bouncePeriod)*sMax*tMax*sMax2*tMax2*lmax);
	} else {
		dp = positionAttrib;
	}
	positionVarying = (toWorld*vec4(dp,1)).xyz;
	colorVarying = vcolorAttrib;
	texCoordVarying = texCoordAttrib;
	
	
	gl_Position = toScreen * vec4(dp, 1);
	
	
}
