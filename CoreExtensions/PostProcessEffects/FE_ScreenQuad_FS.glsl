#version 450 core

in vec2 textureCoords;
@Texture@ quadTexture;

void main(void)
{
	gl_FragColor = texture(quadTexture, textureCoords);
}