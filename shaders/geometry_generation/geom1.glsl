#version 430

float fSphere(in vec3 p, float r);

float fScene(in vec3 p) {
	return fSphere(p, 5.0);
}
