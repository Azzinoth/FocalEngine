in vec2 textureCoords;
@Texture@ SSAOTexture;
@Texture@ SceneDepthTexture;
@Texture@ SceneNormalsTexture;
uniform vec2 FEBlurDirection;
uniform float BlurRadius;

out vec4 out_Color;

#define KERNEL_COUNT 11
float BlurKernels[KERNEL_COUNT] = float[](0.0093, 0.028002, 0.065984, 0.121703, 0.175713, 0.198596, 0.175713, 0.121703, 0.065984, 0.028002, 0.0093);

void main(void)
{
	vec2 TexelSize = 1.0 / textureSize(SSAOTexture, 0);

	vec2 BlurTextureCoords[KERNEL_COUNT];
	vec2 CenterTexCoords = textureCoords;

	float OriginalDepthValue = texture(SceneDepthTexture, CenterTexCoords).r;
	vec3 OriginalNormal = texture(SceneNormalsTexture, CenterTexCoords).rgb;

	for (int i = -5; i <= 5; i++)
	{
		BlurTextureCoords[i + 5] = CenterTexCoords + (TexelSize * FEBlurDirection) * i * BlurRadius;
	}

	out_Color = vec4(0.0);

	for (int i = 0; i < KERNEL_COUNT; i++)
	{
		float KernelDepthValue = texture(SceneDepthTexture, BlurTextureCoords[i]).r;
		vec3 KernelNormal = texture(SceneNormalsTexture, BlurTextureCoords[i]).rgb;

		if (abs(OriginalDepthValue - KernelDepthValue) < 0.0001f &&
			dot(OriginalNormal, KernelNormal) > 0.9f)
		{
			out_Color += texture(SSAOTexture, BlurTextureCoords[i]) * BlurKernels[i];
		}
		else
		{
			out_Color += texture(SSAOTexture, CenterTexCoords) * BlurKernels[i];
		}
	}
}