in vec2 TextureCoordinates;
@Texture@ InterfaceTexture;

layout (location = 0) out vec4 OutColor;

void main(void)
{
	OutColor = texture(InterfaceTexture, TextureCoordinates);
}