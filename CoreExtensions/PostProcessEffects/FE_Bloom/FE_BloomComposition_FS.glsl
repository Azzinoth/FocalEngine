in vec2 textureCoords;
@Texture@ inputTexture;
@Texture@ otherTexture;

out vec4 out_Color;

void main(void)
{
	out_Color = vec4(texture(inputTexture, textureCoords).rgb + texture(otherTexture, textureCoords).rgb, 1.0);
}