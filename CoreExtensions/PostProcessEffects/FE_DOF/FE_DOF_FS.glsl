in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ depthTexture;
uniform vec2 FEBlurDirection;
uniform float depthThreshold;
uniform float depthThresholdFar;
uniform float blurSize;
uniform float intMult;
uniform float zNear;
uniform float zFar;

void main(void)
{
	float depthValue = texture(depthTexture, textureCoords).r;
	float depthValueWorld = 2.0 * depthValue - 1.0;
	depthValueWorld = 2.0 * zNear * zFar / (zFar + zNear - depthValueWorld * (zFar - zNear));

	float A = -(zFar + zNear) / (zFar - zNear);
    float B = (-2 * zFar * zNear) / (zFar - zNear);
	float thresholdInZbuffer = 0.5 * (-A * depthThreshold + B) / depthThreshold + 0.5;
	float thresholdInZbufferFar = 0.5 * (-A * depthThresholdFar + B) / depthThresholdFar + 0.5;
	
	vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

	vec4 finalColor = vec4(0.0);
	if (depthValueWorld < depthThreshold)
	{
		float intensity = (thresholdInZbuffer - depthValue) * (intMult / 10.0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords;

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * FEBlurDirection) * i * blurSize * intensity;
		}

		finalColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		finalColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		finalColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else if (depthValueWorld > depthThresholdFar)
	{
		float intensity = (thresholdInZbufferFar - depthValue) * (intMult * 2.0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords;

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + (tex_offset * FEBlurDirection) * i * blurSize * intensity;
		}

		finalColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		finalColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		finalColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		finalColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		finalColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		finalColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		finalColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else
	{
		finalColor = texture(sceneTexture, textureCoords);
	}

	gl_FragColor = finalColor;
}