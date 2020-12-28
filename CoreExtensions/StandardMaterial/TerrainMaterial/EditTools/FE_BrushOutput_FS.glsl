in vec2 textureCoords;
uniform vec2 brushCenter;
uniform float brushSize;
uniform float brushIntensity;
uniform float brushMode;
@Texture@ brushCanvasTexture;
	
void main(void)
{
	float distanceToBrush = distance(brushCenter, textureCoords);
	if (distanceToBrush < brushSize)
	{
		float finalIntensity = brushIntensity;
		if (brushMode == 1.0)
			finalIntensity = -finalIntensity;

		float persentage = (distanceToBrush / brushSize);
		float displacement = finalIntensity * (1 - persentage);
		gl_FragColor.r = texture(brushCanvasTexture, textureCoords).r + displacement;

		if (brushMode == 2.0)
			gl_FragColor.r = brushIntensity;

		if (brushMode == 3.0)
		{
			vec2 tex_offset = 1.0 / textureSize(brushCanvasTexture, 0);

			vec2 blurTextureCoords[11];
			vec2 centerTexCoords = textureCoords;

			for (int i = -5; i <= 5; i++)
			{
				blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * vec2(1.0, 0.0)) * i * 2.0;
			}

			gl_FragColor.r = 0.0;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[0]).r * 0.0093;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[1]).r * 0.028002;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[2]).r * 0.065984;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[3]).r * 0.121703;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[4]).r * 0.175713;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[5]).r * 0.198596;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[6]).r * 0.175713;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[7]).r * 0.121703;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[8]).r * 0.065984;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[9]).r * 0.028002;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[10]).r * 0.0093;

			for (int i = -5; i <= 5; i++)
			{
				blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * vec2(0.0, 1.0)) * i * 2.0;
			}

			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[0]).r * 0.0093;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[1]).r * 0.028002;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[2]).r * 0.065984;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[3]).r * 0.121703;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[4]).r * 0.175713;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[5]).r * 0.198596;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[6]).r * 0.175713;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[7]).r * 0.121703;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[8]).r * 0.065984;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[9]).r * 0.028002;
			gl_FragColor.r += texture(brushCanvasTexture, blurTextureCoords[10]).r * 0.0093;

			gl_FragColor.r = gl_FragColor.r / 2.0;
		}
	}
	else
	{
		gl_FragColor.r = texture(brushCanvasTexture, textureCoords).r;
	}
}