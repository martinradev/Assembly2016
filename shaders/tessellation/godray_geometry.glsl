#version 430

layout(points) in;
layout(line_strip, max_vertices=2) out;

in TE_GS_VERTEX {
	vec3 terrainPos;
	vec3 waterPos;
} vertex_in[];

out GS_FS_VERTEX {
	float dWater;
	float depthVarying;
} vertex_out;

uniform mat4 toScreen;

void main( )
{

   gl_Position = toScreen * vec4(vertex_in[0].waterPos, 1);
   vertex_out.depthVarying = gl_Position.z;
   vertex_out.dWater = 0.0;
   EmitVertex();
   
   gl_Position = toScreen * vec4(vertex_in[0].terrainPos, 1);
   vertex_out.depthVarying = gl_Position.z;
   vertex_out.dWater = distance(vertex_in[0].waterPos, vertex_in[0].terrainPos);
   EmitVertex();
   
   EndPrimitive();
}