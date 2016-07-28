#version 430

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 1) buffer PrefixSumBuffer_Declaration {
	int arr[];
};

layout (std430, binding = 0) buffer PrebixBlockBuffer_Declaration {
	int blockArr[];
};

uniform int numBlocks;

shared int blockData[1024];

void main() {
	
	uint tidx = gl_LocalInvocationID.x;
	
	// copy blockArr to shared memory
	blockData[tidx] = blockArr[tidx];
	
	barrier(); // sync all threads
	
	if (tidx >= numBlocks) return;
	
	for (int i = 0; i < 1024; ++i) {
		arr[tidx + i*1024] += blockArr[i];
	}

}