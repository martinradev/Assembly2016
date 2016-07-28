#version 330


// uniforms
uniform vec2 windowSize;
uniform sampler2D img;

out vec4 outColor;


void main( )
{
    vec2 p = gl_FragCoord.xy/windowSize;
    
    outColor = texture(img, p);
}