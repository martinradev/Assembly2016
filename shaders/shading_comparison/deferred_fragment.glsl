#version 430

uniform vec4 diffuseUniform;
uniform vec3 specularUniform;
uniform vec3 cameraPos;
uniform float glossiness;

in vec3 eval_positionVarying;
in vec3 eval_normalVarying;
in vec4 eval_colorVarying;
in vec2 eval_texCoordVarying;

out vec4 color;
out vec4 normal; // normal -> (x y z), w -> u
out vec4 position; // (x y z) -> position, v -> w

void main( )
{
	
    color = eval_colorVarying;
	normal = vec4(eval_normalVarying, eval_texCoordVarying.s);
	position = vec4(eval_positionVarying, eval_texCoordVarying.t);
	
}