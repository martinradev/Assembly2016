

uniform vec4 cubeInfo; // (x,y,z) first cube, (w) cube len
uniform bool isPrefixSumPass; // compute counts

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

struct Vertex {
	vec4 position; // w type
	vec4 normal;
	vec4 data; // xy -> uv
};

layout (std430, binding = 1) buffer VertexBuffer_Declaration {
	Vertex vertices[];
} vertex_buffer;

layout (std430, binding = 2) buffer PrefixSumBuffer_Declaration {
	uint arr[];
} prefix_sum_buffer;

/*
	START TETRAHEDRA MARCHING
*/

#extension GL_ARB_arrays_of_arrays : enable

const int tetraIndices[6][4] = const int[6][4](
		int[4](0, 1, 5, 6),
		int[4]( 0, 5, 6, 4 ),
		int[4]( 0, 1, 6, 2 ),
		int[4]( 0, 4, 6 ,7 ),
		int[4]( 0, 3, 7, 6 ),
		int[4]( 0, 3, 2, 6)
);

bool intersect(in vec3 p1,in vec3 p2, out vec4 intersectionPoint) {
	
	float f1 = fScene(p1);
	float f2 = fScene(p2);
	float s1 = sign(f1);
	float s2 = sign(f2);
	
	if (s1 > s2) {
		float s3 = s1;
		s1 = s2;
		s2 = s3;
		
		vec3 p3 = p1;
		p1 = p2;
		p2 = p3;
		
		float f3 = f1;
		f1 = f2;
		f2 = f3;
	}
	
	if (s1 < 0 && s2 >= 0) {
		vec3 dir = normalize(p1-p2);
		
		vec3 rmPoint = p2 + dir * raymarch(p2, dir, 0.0000001, 10.0);
		
		intersectionPoint = vec4(rmPoint, foundPart);
	
		return true;
	}
	return false;
	
}

/*
	END TETRAHEDRA MARCHING
*/


void main() {
   
   vec3 p = cubeInfo.xyz + vec3(gl_GlobalInvocationID) * cubeInfo.w;
	
   uint WIDTH = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
   uint HEIGHT = gl_NumWorkGroups.y * gl_WorkGroupSize.y;
   
   vec3 cubeVertices[] = vec3[](
			p,
			p + vec3(cubeInfo.w, 0.0, 0.0),
			p + vec3(cubeInfo.w, 0.0, cubeInfo.w),
			p + vec3(0.0, 0.0, cubeInfo.w),
			p + vec3(0.0, cubeInfo.w, 0.0),
			p + vec3(cubeInfo.w, cubeInfo.w, 0.0),
			p + vec3(cubeInfo.w, cubeInfo.w, cubeInfo.w),
			p + vec3(0.0, cubeInfo.w, cubeInfo.w)
	);
	
	uint threadIndex = ( gl_GlobalInvocationID.x + WIDTH * (gl_GlobalInvocationID.z * HEIGHT + gl_GlobalInvocationID.y) );
	uint vertexBufferStartIndex = prefix_sum_buffer.arr[threadIndex];
	prefix_sum_buffer.arr[threadIndex] = 0;

	for (int tetraId = 0; tetraId < 6; ++ tetraId) {
		
			vec3 tetra[] = vec3[](
				cubeVertices[tetraIndices[tetraId][0]],
				cubeVertices[tetraIndices[tetraId][1]],
				cubeVertices[tetraIndices[tetraId][2]],
				cubeVertices[tetraIndices[tetraId][3]]
			);
			
			vec4 points[4];
			int foundPoints=0;
			
			for (int t1 = 0; t1 < 4; ++t1) {
				for (int t2 = t1+1; t2 < 4; ++t2) {
					vec4 intersectionPoint;
					if (intersect(tetra[t1], tetra[t2], intersectionPoint)) {
						points[foundPoints++] = intersectionPoint;
					}
				}
			}
			
			if (isPrefixSumPass) {
				
				if (foundPoints == 3) {
					prefix_sum_buffer.arr[threadIndex] += 3;
				} else if (foundPoints == 4) {
					prefix_sum_buffer.arr[threadIndex] += 6;
				}
				
			} else {
				if (foundPoints == 3) {
				
					// filter types
					
					vec3 midV = (points[0]+points[1] + points[2]).xyz/3.0;
					
					fScene(midV);
					points[0].w = points[1].w = points[2].w = foundPart;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];

					
					++vertexBufferStartIndex;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[1];

					
					++vertexBufferStartIndex;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[2];

					
					++vertexBufferStartIndex;
				} else if (foundPoints == 4) {
					
					vec3 midV = (points[0]+points[2] + points[3]).xyz/3.0;
					
					fScene(midV);
					points[0].w = points[2].w = points[3].w = foundPart;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];

					++vertexBufferStartIndex;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[2];

					++vertexBufferStartIndex;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[3];

					++vertexBufferStartIndex;
					
					midV = (points[0]+points[3] + points[1]).xyz/3.0;
					
					fScene(midV);
					points[0].w = points[3].w = points[1].w = foundPart;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];

					++vertexBufferStartIndex;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[3];

					++vertexBufferStartIndex;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[1];

					++vertexBufferStartIndex;
					
				}
			}
			
			
		}
   
   
   
   
   
   

}