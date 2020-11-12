#version 450 core

in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ haloBluredTexture;
@Texture@ haloMaskTexture;

void main(void)
{
	if (texture(haloMaskTexture, textureCoords).a < 1)
	{
		gl_FragColor = texture(sceneTexture, textureCoords) + texture(haloBluredTexture, textureCoords);
	}
	else
	{
		gl_FragColor = texture(sceneTexture, textureCoords);
	}
}