#version 430

layout (local_size_x = 20, local_size_y = 20, local_size_z = 1) in;

uniform layout(binding = 0, rgba32f) readonly image2D positionImage;

uniform layout(binding = 1, rgba32f) image2D positionImageResult;

uniform ivec2 imageSize;

vec3 verlet(in vec3 a, in vec3 x, in vec3 xOld, in float dt) {
	
	return 2.0 * x - xOld + a * (dt*dt);
	
}

shared vec4 sharedPositions[400];

void main() {
   
	const float G = 9.8;
	
	ivec2 globalID = ivec2(gl_GlobalInvocationID.xy);
	if (globalID.x >= imageSize.x || globalID.y >= imageSize.y) return;
	
	ivec2 localID = ivec2(gl_LocalInvocationID.xy);
   
	vec4 particleInfo = imageLoad(positionImage, globalID);
	vec3 pos = particleInfo.xyz;
	float mass = particleInfo.w;
	
	vec3 oldPos = imageLoad(positionImageResult, globalID).xyz;
	
	vec3 acc = vec3(0);
	
	for (int i = 0; i < imageSize.x; i+=20) {
		
		for (int j = 0; j < imageSize.y; j+=20) {
			
			
			sharedPositions[localID.y * 20 + localID.x] = imageLoad(positionImage, localID+ivec2(i,j));
			
			barrier();
			memoryBarrierShared();
			
			for (int ii = 0; ii < 20; ++ii) {
				for (int jj = 0; jj < 20; ++jj) {
					vec4 otherParticleInfo = sharedPositions[jj + ii*20];
					vec3 otherPos = otherParticleInfo.xyz;
					float otherMass = otherParticleInfo.w;
			
					vec3 dirToParticle = (otherPos-pos);
			
					float rSq = dot(dirToParticle,dirToParticle);
					//float r = length(dirToParticle);
			
					acc += dirToParticle * otherMass / pow(rSq + 0.1, 1.5);
				}
			}
			
			barrier();
	
		}
		
	}
	
	acc *= G;
	
	vec3 newPos = verlet(acc, pos, oldPos, 0.0001);
	
	imageStore(positionImageResult, globalID, vec4(newPos,mass));
}