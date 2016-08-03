#version 430

out vec4 color;
uniform sampler2D inImage;
uniform sampler2D bloomImage;
uniform sampler3D colorGradingLUT;
uniform bool useOverlay;
uniform float overlayAlpha;
uniform sampler2D overlay;
uniform float time;
varying vec2 uv;

uniform float exposure;
uniform float fadeMix;
uniform float fadeColor;
uniform float blurOut;

/* 
	This FXAA implementation was done by mudlord.
	https://code.google.com/p/dolphin-emu/source/browse/Data/Sys/Shaders/FXAA.glsl
 */

#define FXAA_REDUCE_MIN (1.0/ 128.0)
#define FXAA_REDUCE_MUL (1.0 / 8.0)
#define FXAA_SPAN_MAX 8.0

vec4 applyFXAA(vec2 fragCoord, sampler2D tex, vec2 resolution)
{
	vec4 color;
	vec2 inverseVP = vec2(1.0/resolution.x, 1.0/resolution.y);
	vec3 rgbNW = texture(tex, (fragCoord + vec2(-1.0, -1.0)) * inverseVP).xyz;
	vec3 rgbNE = texture(tex, (fragCoord + vec2(1.0, -1.0)) * inverseVP).xyz;
	vec3 rgbSW = texture(tex, (fragCoord + vec2(-1.0, 1.0)) * inverseVP).xyz;
	vec3 rgbSE = texture(tex, (fragCoord + vec2(1.0, 1.0)) * inverseVP).xyz;
	vec3 rgbM = texture(tex, fragCoord * inverseVP).xyz;
	vec3 luma = vec3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM = dot(rgbM, luma);
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
	(0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
	max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
	dir * rcpDirMin)) * inverseVP;

	vec3 rgbA = 0.5 * (
	texture(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
	texture(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
	vec3 rgbB = rgbA * 0.5 + 0.25 * (
	texture(tex, fragCoord * inverseVP + dir * -0.5).xyz +
	texture(tex, fragCoord * inverseVP + dir * 0.5).xyz);

	float lumaB = dot(rgbB, luma);
	
	if ((lumaB < lumaMin) || (lumaB > lumaMax))
		color = vec4(rgbA, 1.0);
	else
		color = vec4(rgbB, 1.0);
		
	return color;
}

float hash( float n ){

    return fract(sin(n)*43758.5453);
}

float hashNoise3D( in vec3 x ){

    vec3 p = floor(x);
    vec3 f = fract(x);//smoothstep(0., 1., fract(x));

    f = f*f*(3.0-2.0*f);//Smoothstep
    //f = f*f*f*(10.0+f*(6.0*f-15.0));//Smootherstep
   
    float n = p.x*7.0 + p.y*57.0 + 111.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  7.0),f.x),
                   mix( hash(n+ 57.0), hash(n+ 64.0),f.x),f.y),
               mix(mix( hash(n+111.0), hash(n+118.0),f.x),
                   mix( hash(n+168.0), hash(n+175.0),f.x),f.y),f.z);

    
    /*
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+ 157.0), hash(n+ 158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
    */ 
    
}

#define HASHSCALE1 .1031

float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

void main()
{
	float distFromCenter = length(uv - vec2(0.5));
	
	vec3 col = applyFXAA(gl_FragCoord.xy, inImage, vec2(textureSize(inImage, 0))).rgb;
	vec3 cl = texture2D(inImage, uv).xyz + texture2D(bloomImage, uv).xyz;
	
	cl *= exposure;
	
	float m = max(cl.r, max(cl.g, cl.b));
	if (m > 1.0) cl += 0.2*vec3(m - 1.0);
	
	cl = cl / (vec3(1.0) + cl);
	
	
	
	cl = pow(cl, vec3(1.0 / 2.2));
	
	
	
	float q = smoothstep(0.0, blurOut, distFromCenter);
	q += hash12(160.0*uv + vec2(time))/20.0;
	cl = mix(cl, vec3(0.0), q);
	
	cl = mix(cl, vec3(fadeColor), fadeMix);
	
	if (useOverlay)
	{
		vec2 newUV = uv;
		newUV.y = 1.0 - uv.y;
		vec4 overlayValue = texture(overlay, newUV);
		overlayValue.a = step(overlayAlpha, overlayValue.a);
		cl += overlayValue.xyz * overlayValue.a;
	}
	
	vec3 newColor = texture(colorGradingLUT, cl).rbg;
	
	newColor += vec3(hash12(160.0*uv)/255.0);
	
	color = vec4(newColor, 1.0);
	
}