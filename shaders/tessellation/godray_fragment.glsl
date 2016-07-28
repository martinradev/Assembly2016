#version 430

in GS_FS_VERTEX {
	float dWater;
	float depthVarying;
} vertex_out;

out vec4 diffuse;
uniform vec3 lightColor;

void main( )
{

	const float KA = 0.0122;
	const float KD = 0.038;
	vec3 color = lightColor * ( exp(- KA * vertex_out.dWater) * exp(-KD*vertex_out.depthVarying - 0.5));
	
	diffuse = vec4(color, 0.1);

}