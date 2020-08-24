#version 450 core

in vec2 textureCoords;
@Texture@ inputTexture;
uniform vec2 FEBlurDirection;
uniform float BloomSize;

void main(void)
{
	vec2 tex_offset = 1.0 / textureSize(inputTexture, 0);

	vec2 blurTextureCoords[11];
	vec2 centerTexCoords = textureCoords;

	for (int i = -5; i <= 5; i++)
	{
		//if (FEBloomNoScale > 0.0)
		//{
		//	blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * FEBlurDirection) * i;
		//}
		//else
		//{
			blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * FEBlurDirection) * i * BloomSize;
		//}
	}

	gl_FragColor = vec4(0.0);
	gl_FragColor += texture(inputTexture, blurTextureCoords[0]) * 0.0093;
	gl_FragColor += texture(inputTexture, blurTextureCoords[1]) * 0.028002;
	gl_FragColor += texture(inputTexture, blurTextureCoords[2]) * 0.065984;
	gl_FragColor += texture(inputTexture, blurTextureCoords[3]) * 0.121703;
	gl_FragColor += texture(inputTexture, blurTextureCoords[4]) * 0.175713;
	gl_FragColor += texture(inputTexture, blurTextureCoords[5]) * 0.198596;
	gl_FragColor += texture(inputTexture, blurTextureCoords[6]) * 0.175713;
	gl_FragColor += texture(inputTexture, blurTextureCoords[7]) * 0.121703;
	gl_FragColor += texture(inputTexture, blurTextureCoords[8]) * 0.065984;
	gl_FragColor += texture(inputTexture, blurTextureCoords[9]) * 0.028002;
	gl_FragColor += texture(inputTexture, blurTextureCoords[10]) * 0.0093;
}