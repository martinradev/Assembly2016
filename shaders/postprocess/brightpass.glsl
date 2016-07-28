#version 430

// copied from https://github.com/AnalyticalGraphicsInc/cesium/

vec3 czm_XYZToRGB(vec3 Yxy)
{
    const mat3 XYZ2RGB = mat3( 3.2405, -0.9693,  0.0556,
                              -1.5371,  1.8760, -0.2040,
                              -0.4985,  0.0416,  1.0572);
    vec3 xyz;
    xyz.r = Yxy.r * Yxy.g / Yxy.b;
    xyz.g = Yxy.r;
    xyz.b = Yxy.r * (1.0 - Yxy.g - Yxy.b) / Yxy.b;
    
    return XYZ2RGB * xyz;
}

vec3 czm_RGBToXYZ(vec3 rgb)
{
    const mat3 RGB2XYZ = mat3(0.4124, 0.2126, 0.0193,
                              0.3576, 0.7152, 0.1192,
                              0.1805, 0.0722, 0.9505);
    vec3 xyz = RGB2XYZ * rgb;
    vec3 Yxy;
    Yxy.r = xyz.g;
    float temp = dot(vec3(1.0), xyz);
    Yxy.gb = xyz.rg / temp;
    return Yxy;
}

out vec4 color;
uniform sampler2D inImage;
uniform float LUMINANCE;
uniform float THRESHOLD;
uniform float OFFSET;
in vec2 uv;

float key(float avg)
{
    float guess = 1.5 - (1.5 / (avg * 0.1 + 1.0));
    return max(0.0, guess) + 0.1;
}

// See section 9. "The bright-pass filter" of Realtime HDR Rendering
// http://www.cg.tuwien.ac.at/research/publications/2007/Luksch_2007_RHR/Luksch_2007_RHR-RealtimeHDR%20.pdf

void main()
{
	
	float u_avgLuminance = LUMINANCE;
	float u_threshold = THRESHOLD;
	float u_offset = OFFSET;
	
    vec4 cl = texture2D(inImage, uv);
    vec3 xyz = czm_RGBToXYZ(cl.rgb);
    float luminance = xyz.r;
    
    float scaledLum = key(u_avgLuminance) * luminance / u_avgLuminance;
    float brightLum = max(scaledLum - u_threshold, 0.0);
    float brightness = brightLum / (u_offset + brightLum);
    
    xyz.r = brightness;
    color = vec4(czm_XYZToRGB(xyz), 1.0);
	color = clamp(color, vec4(0), vec4(1));
	
}