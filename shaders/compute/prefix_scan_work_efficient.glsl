#version 430


#define NUM_BANKS 16
#define LOG_NUM_BANKS 4
#ifdef ZERO_BANK_CONFLICTS
#define CONFLICT_FREE_OFFSET(n) \
 ((n) >> NUM_BANKS + (n) >> (2 * LOG_NUM_BANKS))
#else
#define CONFLICT_FREE_OFFSET(n) ((n) >> LOG_NUM_BANKS)
#endif


layout (local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer InputPrefixSumBuffer_Declaration {
	int inarr[];
};

layout (std430, binding = 1) buffer OutputPrefixSumBuffer_Declaration {
	int outarr[];
};

layout (std430, binding = 2) buffer PrebixBlockBuffer_Declaration {
	int blockArr[];
};


uniform int numValues;
uniform bool storeLastValues;

shared int buf[2048];

void main() {
   
 
	int vIndex = int(gl_GlobalInvocationID.x);
	int tidx = int(gl_LocalInvocationID.x);
	
	int n = 1024;
	
	int ai = tidx;
	int bi = tidx + n/2;
	
	int bankOffsetA = CONFLICT_FREE_OFFSET(ai);
	int bankOffsetB = CONFLICT_FREE_OFFSET(bi); 
	
	buf[ai + bankOffsetA] = inarr[ai];
	buf[bi + bankOffsetB] = inarr[bi];
	
	int lastValue = 0;
	if (2*tidx+1 == n-1) {
		lastValue = buf[bi + bankOffsetB];
	}
	
	// do stuff
	
	int offset = 1;
	
	for (int d = n>>1; d > 0; d >>= 1) {
		
		barrier();
		memoryBarrierShared();
		if (tidx < d) {
			int ai = offset * (2 * tidx +1) - 1;
			ai += CONFLICT_FREE_OFFSET(ai);;
			int bi = offset * (2 * tidx + 2) - 1;
			bi += CONFLICT_FREE_OFFSET(bi);
			buf[bi] += buf[ai];
		}
		
		offset *= 2;
		
	}
	
	if (tidx == 0) buf[n-1+ CONFLICT_FREE_OFFSET(n - 1)] = 0;
	
	for (int d = 1; d < n; d*=2) {
		offset >>= 1;
		
		barrier();
		memoryBarrierShared();
		
		if (tidx < d) {
			int ai = offset * (2 * tidx +1) - 1;
			ai += CONFLICT_FREE_OFFSET(ai);;
			int bi = offset * (2 * tidx + 2) - 1;
			bi += CONFLICT_FREE_OFFSET(bi);
			
			int t = buf[ai];
			buf[ai] = buf[bi];
			buf[bi] += t;
		}
		
	}
	
	barrier();
	memoryBarrierShared();
	
	outarr[2*vIndex + bankOffsetA] = buf[ai+bankOffsetA];
	outarr[2*vIndex+1 + bankOffsetB] = buf[bi+bankOffsetB];
	
	if (storeLastValues && 2*tidx+1 == n-1) {
		// final thread
		blockArr[gl_WorkGroupID.x] = buf[bi+bankOffsetB]+lastValue;
	} else if (!storeLastValues && 2*tidx+1 == n-1) {
		blockArr[n] = buf[2*tidx+1+bankOffsetB]+lastValue;
		// count
	}
	

}