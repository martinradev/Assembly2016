

uniform vec4 cubeInfo; // (x,y,z) first cube, (w) cube len
uniform bool isPrefixSumPass; // compute counts

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

struct Vertex {
	vec4 position; // w type
	vec4 direction;
	vec4 normal;
	vec4 orig_position;
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


const ivec4 tetraIndices[6] = const ivec4[6] (
		ivec4(0, 5, 6, 1),
		ivec4( 4, 6 ,0, 5 ), // ok
		ivec4( 1, 0, 6, 2 ), // ~
		ivec4( 0, 4, 6 ,7 ),
		ivec4( 0, 3, 7, 6 ),
		ivec4( 0, 3, 2, 6)
);

bool intersect(in float f1, in float f2, in vec3 p1,in vec3 p2, out vec4 intersectionPoint) {

	float s1 = sign(f1);
	float s2 = sign(f2);
	
	vec3 pF = p1;
	vec3 pE = p2;
	
	if (s1 > s2) {
		float s3 = s1;
		s1 = s2;
		s2 = s3;
		vec3 p3 = pF;
		pF = pE;
		pE = p3;
	}
	
	if (s1 < 0.0 && s2 >= 0.0) {
		
		const int NUM_STEPS = 2;
		
		float t = 0.5;
		vec3 dir = pE-pF;
		
		for (int i = 0; i < NUM_STEPS; ++i) {
			
			vec3 ptmp = pF + dir*t;
			float value = fScene(ptmp);
			
			if (value < 0.0) {
				pF = ptmp;
			}
			
			t *= 0.5;
		}

		intersectionPoint = vec4(pF, 1);
	
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
			vec3(cubeInfo.w+p.x, p.y, p.z),
			vec3(cubeInfo.w+p.x, p.y, cubeInfo.w+p.z),
			vec3(p.x, p.y, p.z+cubeInfo.w),
			vec3(p.x, p.y+cubeInfo.w, p.z),
			vec3(p.x+cubeInfo.w, p.y+cubeInfo.w, p.z),
			vec3(p.x+cubeInfo.w, p.y+cubeInfo.w, p.z+cubeInfo.w),
			vec3(p.x, p.y+cubeInfo.w, p.z+cubeInfo.w)
	);
	
	float cubeF[] = float[](
		fScene(cubeVertices[0]),
		fScene(cubeVertices[1]),
		fScene(cubeVertices[2]),
		fScene(cubeVertices[3]),
		fScene(cubeVertices[4]),
		fScene(cubeVertices[5]),
		fScene(cubeVertices[6]),
		fScene(cubeVertices[7])
	);
	
	uint threadIndex = ( gl_GlobalInvocationID.x + WIDTH * (gl_GlobalInvocationID.z * HEIGHT + gl_GlobalInvocationID.y) );
	uint vertexBufferStartIndex = prefix_sum_buffer.arr[threadIndex];
	prefix_sum_buffer.arr[threadIndex] = 0;
	int numTetras = 6;
	for (int tetraId = 0; tetraId < numTetras; ++ tetraId) {
		
			vec3 tetra[] = vec3[](
				cubeVertices[tetraIndices[tetraId].x],
				cubeVertices[tetraIndices[tetraId].y],
				cubeVertices[tetraIndices[tetraId].z],
				cubeVertices[tetraIndices[tetraId].w]
			);
			
			float tetraF[] = float[](
				cubeF[tetraIndices[tetraId].x],
				cubeF[tetraIndices[tetraId].y],
				cubeF[tetraIndices[tetraId].z],
				cubeF[tetraIndices[tetraId].w]
			);
			
			vec4 points[4];
			int foundPoints=0;
			
			for (int t1 = 0; t1 < 4; ++t1) {
				for (int t2 = t1+1; t2 < 4; ++t2) {
					vec4 intersectionPoint;
					if (intersect(tetraF[t1], tetraF[t2], tetra[t1], tetra[t2], intersectionPoint)) {
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
				
				vec3 n0 = getNormal(points[0].xyz);
				vec3 n1 = getNormal(points[1].xyz);
				vec3 n2 = getNormal(points[2].xyz);
				
				if (foundPoints == 3) {
				

					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];
					vertex_buffer.vertices[vertexBufferStartIndex].normal.xyz = n0;
					
					vertex_buffer.vertices[vertexBufferStartIndex+1].position = points[1];
					vertex_buffer.vertices[vertexBufferStartIndex+1].normal.xyz = n1;
					
					vertex_buffer.vertices[vertexBufferStartIndex+2].position = points[2];
					vertex_buffer.vertices[vertexBufferStartIndex+2].normal.xyz = n2;
					
					vertexBufferStartIndex+=3;
				} else if (foundPoints == 4) {
					
					vec3 n3 = getNormal(points[3].xyz);

					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];
					vertex_buffer.vertices[vertexBufferStartIndex].normal.xyz = n0;
					
					vertex_buffer.vertices[vertexBufferStartIndex+1].position = points[2];
					vertex_buffer.vertices[vertexBufferStartIndex+1].normal.xyz = n2;
					
					vertex_buffer.vertices[vertexBufferStartIndex+2].position = points[3];
					vertex_buffer.vertices[vertexBufferStartIndex+2].normal.xyz = n3;

					vertexBufferStartIndex+=3;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];
					vertex_buffer.vertices[vertexBufferStartIndex].normal.xyz = n0;
					
					vertex_buffer.vertices[vertexBufferStartIndex+1].position = points[3];
					vertex_buffer.vertices[vertexBufferStartIndex+1].normal.xyz = n3;
					
					vertex_buffer.vertices[vertexBufferStartIndex+2].position = points[1];
					vertex_buffer.vertices[vertexBufferStartIndex+2].normal.xyz = n1;

					vertexBufferStartIndex+=3;
					
				}
			}
			
			
		}
   

}