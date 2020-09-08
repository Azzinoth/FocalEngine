#version 450 core

uniform vec3 baseColor;

void main(void)
{
	gl_FragColor = vec4(baseColor, 1.0);
}