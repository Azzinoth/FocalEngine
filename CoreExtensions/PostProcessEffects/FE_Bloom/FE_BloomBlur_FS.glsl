in vec2 textureCoords;
@Texture@ inputTexture;
uniform vec2 FEBlurDirection;
uniform float BloomSize;

out vec4 out_Color;

void main(void)
{
	vec2 tex_offset = 1.0 / textureSize(inputTexture, 0);

	vec2 blurTextureCoords[11];
	vec2 centerTexCoords = textureCoords;

	for (int i = -5; i <= 5; i++)
	{
		blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * FEBlurDirection) * i * BloomSize;
	}

	out_Color = vec4(0.0);
	out_Color += texture(inputTexture, blurTextureCoords[0]) * 0.0093;
	out_Color += texture(inputTexture, blurTextureCoords[1]) * 0.028002;
	out_Color += texture(inputTexture, blurTextureCoords[2]) * 0.065984;
	out_Color += texture(inputTexture, blurTextureCoords[3]) * 0.121703;
	out_Color += texture(inputTexture, blurTextureCoords[4]) * 0.175713;
	out_Color += texture(inputTexture, blurTextureCoords[5]) * 0.198596;
	out_Color += texture(inputTexture, blurTextureCoords[6]) * 0.175713;
	out_Color += texture(inputTexture, blurTextureCoords[7]) * 0.121703;
	out_Color += texture(inputTexture, blurTextureCoords[8]) * 0.065984;
	out_Color += texture(inputTexture, blurTextureCoords[9]) * 0.028002;
	out_Color += texture(inputTexture, blurTextureCoords[10]) * 0.0093;
}