#version 430

//in vec2 in_ImagePos;

in vec4 INVEC;

uniform mat4 toClip;

uniform layout(binding = 0, rgba32f) readonly image2D positionImage;

void main() {
	

	int instanceX = gl_InstanceID/120;
	int instanceY = gl_InstanceID - instanceX*120;
	vec4 particlePos = imageLoad(positionImage, ivec2(instanceX, instanceY));
	
	gl_Position = toClip * vec4(particlePos.xyz + INVEC.xyz*0.03, 1);
	
	float depth = gl_Position.z;
	
}