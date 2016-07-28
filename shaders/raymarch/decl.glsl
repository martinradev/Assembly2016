#version 430

#define EPS 0.0000000001
#define TMAX 100.0
#define TMIN 0.0000000001
#define DEBUG false

//#define SIMPLE 1

uniform float time;

float fScene(in vec3 p);
float fHeight(in float x, in float z);

uniform vec3 center; // camera center