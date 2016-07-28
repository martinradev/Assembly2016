

layout (local_size_x = 8, local_size_y = 8) in;

uniform uint numThreads;
uniform vec2 planeStart;
uniform vec2 planeStep; 

struct Vertex {
	vec4 position;
	vec4 normal;
};

layout (std430, binding = 1) buffer VertexBuffer_Declaration {
	Vertex vertices[];
} vertex_buffer;



void main() {
   
	uint WIDTH = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
	uint threadIndex = gl_GlobalInvocationID.x + WIDTH * gl_GlobalInvocationID.y;
	
	if (threadIndex >= numThreads) return;
	
	vec2 p1 = planeStart + planeStep * vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y);
	vec2 p2 = planeStart + planeStep * vec2(gl_GlobalInvocationID.x+1, gl_GlobalInvocationID.y);
	vec2 p3 = planeStart + planeStep * vec2(gl_GlobalInvocationID.x+1, gl_GlobalInvocationID.y+1);
	vec2 p4 = planeStart + planeStep * vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y+1);
	
	vec3 n1 = getNormalTerrain(p1.x, p1.y);
	vec3 n2 = getNormalTerrain(p2.x, p2.y);
	vec3 n3 = getNormalTerrain(p3.x, p3.y);
	vec3 n4 = getNormalTerrain(p4.x, p4.y);
	
	vec4 h = vec4(fHeight(p1.x, p1.y), fHeight(p2.x,p2.y), fHeight(p3.x,p3.y), fHeight(p4.x,p4.y));
	
	uint pos = threadIndex * 6;
	
	// triangle 1
	vertex_buffer.vertices[pos].position = vec4(p1.x,h.x,p1.y,0.0);
	vertex_buffer.vertices[pos].normal = vec4(n1,0);
	++pos;
	vertex_buffer.vertices[pos].position = vec4(p2.x,h.y,p2.y,0);
	vertex_buffer.vertices[pos].normal = vec4(n2,0);
	++pos;
	vertex_buffer.vertices[pos].position = vec4(p3.x,h.z,p3.y,0);
	vertex_buffer.vertices[pos].normal = vec4(n3,0);
	++pos;
	
	// triangle 2
	vertex_buffer.vertices[pos].position = vec4(p1.x,h.x,p1.y,0);
	vertex_buffer.vertices[pos].normal = vec4(n1,0);
	++pos;
	vertex_buffer.vertices[pos].position = vec4(p3.x,h.z,p3.y,0);
	vertex_buffer.vertices[pos].normal = vec4(n3,0);
	++pos;
	vertex_buffer.vertices[pos].position = vec4(p4.x,h.w,p4.y,0);
	vertex_buffer.vertices[pos].normal = vec4(n4,0);
	
	
}