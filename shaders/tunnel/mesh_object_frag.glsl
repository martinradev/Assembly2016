#version 450

in vec3 positionFrag;
in vec3 normalFrag;
in vec2 oldUVFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec2 oldUVOut;

uniform vec3 cameraPos;
uniform vec3 lightColor;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 specColorIN) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.18, 1.0) * difColor;
	vec3 specular = specColorIN * pow(clamp(dot(h, n), 0.1, 1.0), 8.0);
	vec3 Li = intensity * (1.0*diffuse+0.2*specular);
	
	return Li;
}

void main() {

	vec3 difColorIN = vec3(70,40,0)/255.0;
	
	

	vec3 V = normalize(cameraPos-positionFrag);
	vec3 lightDirection = normalize(cameraPos-positionFrag);
	vec3 R = reflect(-V, normalFrag);
	//vec3 specColorIN = texture(envMapTex,R).xyz;
	
	vec3 color = calcLight(normalize(V+lightDirection),normalFrag,lightDirection,difColorIN,lightColor, vec3(0.07, 0.34, 0.68));

	diffuseColorOUT = vec4(0.6*color, 1.0);
	normalOUT = vec4(normalFrag, 0);
	oldUVOut = oldUVFrag;
}