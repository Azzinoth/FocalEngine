in vec2 textureCoords;
@Texture@ quadTexture;

out vec4 out_Color;

void main(void)
{
	out_Color = texture(quadTexture, textureCoords);
}