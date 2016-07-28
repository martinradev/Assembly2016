

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

uniform uint numVertices;

struct Vertex {
	vec4 position; // w -> type
	vec4 normal;
	vec4 data; // xy -> uv
};

layout (std430, binding = 1) buffer VertexBuffer_Declaration {
	Vertex vertices[];
} vertex_buffer;



void main() {
   
	uint WIDTH = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	uint HEIGHT = gl_NumWorkGroups.y * gl_WorkGroupSize.y;
	uint threadIndex = ( gl_GlobalInvocationID.x + WIDTH * (gl_GlobalInvocationID.z * HEIGHT + gl_GlobalInvocationID.y) );
	
	if (threadIndex >= numVertices) return;
	
	vec3 pos = vertex_buffer.vertices[threadIndex].position.xyz;
	int type = int(vertex_buffer.vertices[threadIndex].position.w);
	vec3 n = getNormal(pos);
	
	vertex_buffer.vertices[threadIndex].normal = vec4(n, 1.0);
	vec2 uv = getUV(pos, n, type);
	vertex_buffer.vertices[threadIndex].data.xy = uv;

}