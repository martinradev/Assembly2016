#version 450

in vec3 positionFrag;
in vec3 normalFrag;

layout(location = 0) out vec4 diffuseColorOUT;
layout(location = 1) out vec4 normalOUT;
layout(location = 2) out vec4 positionOUT;


uniform vec3 lightPos[3];
uniform vec3 lightColor[3];
uniform vec3 lightDirection[3];
uniform vec3 cameraPos;
uniform samplerCube envMapTex;

vec3 calcLight(vec3 h, vec3 n, vec3 dir, vec3 difColor, vec3 intensity, vec3 specColorIN, vec3 V) {
	vec3 diffuse;
	diffuse = clamp(dot(n, dir), 0.008, 1.0) * difColor;
	diffuse *= 0.0;
	//vec3 specular = specColorIN * pow(clamp(dot(h, n), 0.06, 1.0), 20.0);
	//vec3 specular = vec3(3.0, 4.0, 0.2) * specColorIN * pow(clamp(dot(h, n), 0.06, 1.0), 20.0);
	
	vec3 specular = vec3(0.0);
	
	specColorIN *= vec3(4.0, 2.0, 0.3) * 0.9;
	
	if (1==1) {

	float shiny = 2.;
	float att = 1.0;
    float w = pow(1.0 - max(0.0, dot(h, V)), 5.0);
    specular = att * specColorIN
	  * mix(vec3(specColorIN), vec3(1.0), w)
	  * pow(max(0.0, dot(reflect(dir, n), V)), shiny);
	}
	
	vec3 Li = intensity * (30.0*diffuse+20.0*specular);
	
	return Li;
}

void main() {

	vec3 difColorIN = vec3(70,40,0)/255.0;

	vec3 V = normalize(cameraPos-positionFrag);
	vec3 R = reflect(-V, normalFrag);
	vec3 specColorIN = texture(envMapTex,R).xyz;
	
	vec3 color = calcLight(normalize(-lightDirection[0] + V),normalFrag,-lightDirection[0], difColorIN, lightColor[0], specColorIN, V);
	
	color += calcLight(normalize(-lightDirection[1] + V),normalFrag,-lightDirection[1],difColorIN,lightColor[1], specColorIN, V);
	
	diffuseColorOUT = vec4(color, 1.0);
	
	normalOUT = vec4(normalFrag, 0);
	positionOUT = vec4(positionFrag, 0);
}