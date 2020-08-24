#version 450 core

in vec2 textureCoords;
@Texture@ inputTexture;
@Texture@ otherTexture;

void main(void)
{
	gl_FragColor = vec4(texture(inputTexture, textureCoords).rgb + texture(otherTexture, textureCoords).rgb, 1.0);
}