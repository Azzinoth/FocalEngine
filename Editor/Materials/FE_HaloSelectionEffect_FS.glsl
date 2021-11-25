in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ haloBluredTexture;
@Texture@ haloMaskTexture;

out vec4 out_Color;

void main(void)
{
	if (texture(haloMaskTexture, textureCoords).a < 1)
	{
		out_Color = texture(sceneTexture, textureCoords) + texture(haloBluredTexture, textureCoords);
	}
	else
	{
		out_Color = texture(sceneTexture, textureCoords);
	}
}