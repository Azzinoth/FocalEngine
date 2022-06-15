in vec2 textureCoords;
uniform vec2 brushCenter;
uniform float brushSize;
uniform float brushIntensity;
uniform float brushMode;
@Texture@ textureToEdit;
uniform float layerIndex;

out vec4 out_Color;

void main(void)
{
	float distanceToBrush = distance(brushCenter, textureCoords);
	if (distanceToBrush < brushSize)
	{
		float finalIntensity = brushIntensity;
		if (brushMode == 2.0)
			finalIntensity = -finalIntensity;

		float percentage = (distanceToBrush / brushSize);
		float displacement = finalIntensity * (1 - percentage);
		if (brushMode == 5.0)
		{
			for (int i = 0; i < 4; i++)
			{
				if (int(layerIndex) == i)
				{
					out_Color[i] = texture(textureToEdit, textureCoords)[i] + displacement;
				}
				else
				{
					out_Color[i] = texture(textureToEdit, textureCoords)[i] - displacement;
					if (out_Color[i] < 0)
						out_Color[i] = 0;
				}
			}
		}
		else
		{	
			out_Color.r = texture(textureToEdit, textureCoords).r + displacement;
		}

		if (brushMode == 3.0)
			out_Color.r = brushIntensity;

		if (brushMode == 4.0)
		{
			vec2 tex_offset = 1.0 / textureSize(textureToEdit, 0);

			vec2 blurTextureCoords[11];
			vec2 centerTexCoords = textureCoords;

			for (int i = -5; i <= 5; i++)
			{
				blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * vec2(1.0, 0.0)) * i * 2.0;
			}

			out_Color.r = 0.0;
			out_Color.r += texture(textureToEdit, blurTextureCoords[0]).r * 0.0093;
			out_Color.r += texture(textureToEdit, blurTextureCoords[1]).r * 0.028002;
			out_Color.r += texture(textureToEdit, blurTextureCoords[2]).r * 0.065984;
			out_Color.r += texture(textureToEdit, blurTextureCoords[3]).r * 0.121703;
			out_Color.r += texture(textureToEdit, blurTextureCoords[4]).r * 0.175713;
			out_Color.r += texture(textureToEdit, blurTextureCoords[5]).r * 0.198596;
			out_Color.r += texture(textureToEdit, blurTextureCoords[6]).r * 0.175713;
			out_Color.r += texture(textureToEdit, blurTextureCoords[7]).r * 0.121703;
			out_Color.r += texture(textureToEdit, blurTextureCoords[8]).r * 0.065984;
			out_Color.r += texture(textureToEdit, blurTextureCoords[9]).r * 0.028002;
			out_Color.r += texture(textureToEdit, blurTextureCoords[10]).r * 0.0093;

			for (int i = -5; i <= 5; i++)
			{
				blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * vec2(0.0, 1.0)) * i * 2.0;
			}

			out_Color.r += texture(textureToEdit, blurTextureCoords[0]).r * 0.0093;
			out_Color.r += texture(textureToEdit, blurTextureCoords[1]).r * 0.028002;
			out_Color.r += texture(textureToEdit, blurTextureCoords[2]).r * 0.065984;
			out_Color.r += texture(textureToEdit, blurTextureCoords[3]).r * 0.121703;
			out_Color.r += texture(textureToEdit, blurTextureCoords[4]).r * 0.175713;
			out_Color.r += texture(textureToEdit, blurTextureCoords[5]).r * 0.198596;
			out_Color.r += texture(textureToEdit, blurTextureCoords[6]).r * 0.175713;
			out_Color.r += texture(textureToEdit, blurTextureCoords[7]).r * 0.121703;
			out_Color.r += texture(textureToEdit, blurTextureCoords[8]).r * 0.065984;
			out_Color.r += texture(textureToEdit, blurTextureCoords[9]).r * 0.028002;
			out_Color.r += texture(textureToEdit, blurTextureCoords[10]).r * 0.0093;

			out_Color.r = out_Color.r / 2.0;
		}
	}
	else
	{
		out_Color = texture(textureToEdit, textureCoords);
	}
}
