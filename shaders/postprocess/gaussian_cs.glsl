
#define PI 3.14159265

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout (rgba32f, binding = 0) uniform image2D originalImage;
layout (rgba32f, binding = 1) uniform image2D inputImage;
layout (rgba32f, binding = 2) uniform image2D blurredImage;
layout (rgba32f, binding = 3) uniform image2D outImage;

uniform ivec2 screenSize;

void main() {
	
	if (gl_GlobalInvocationID.y >= screenSize.y || gl_GlobalInvocationID.x >= screenSize.x) return;

	
	float sigma = knob1.x;
	int kernelSize = int(sigma * 3.0);
	int X = int(gl_GlobalInvocationID.x);
	int Y = int(gl_GlobalInvocationID.y);

	int xFrom = X - kernelSize;
	int xTo = X + kernelSize;
	int yFrom = Y - kernelSize;
	int yTo = Y + kernelSize;

	vec3 color = vec3(0);

	for (int y = yFrom; y <= yTo; ++y) {
	
		for (int x = xFrom; x <= xTo; ++x) {
			
			int yPos = clamp(y, 0, screenSize.y-1);
			int xPos = clamp(x, 0, screenSize.x-1);
			
			vec4 texelColor = imageLoad(inputImage, ivec2(xPos,yPos));
			texelColor = clamp(texelColor, vec4(0), vec4(1));
			float dx = (X-x);
			float dy = (Y-y);
			float l = dx*dx+dy*dy;
			float weight = exp(- l  / (2.0 * sigma * sigma)) / (2.0 * PI * sigma *sigma);
			color += weight * texelColor.rgb;
		}
		
	}
	imageStore(blurredImage, ivec2(gl_GlobalInvocationID.xy), vec4(color,1.0));
	
	vec3 origTexel = imageLoad(originalImage, ivec2(gl_GlobalInvocationID.xy)).rgb;
	
	vec3 finalColor = mix(origTexel, color, 0.4);
	
	imageStore(outImage, ivec2(gl_GlobalInvocationID.xy), vec4(finalColor,1.0));
	
	
}