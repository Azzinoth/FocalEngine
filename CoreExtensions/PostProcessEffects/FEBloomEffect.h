#pragma once

#include "../../Renderer/FEPostProcess.h"

static const char* const FEBloomEffectVS = R"(
#version 400 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* FEBloomEffectThresholdFS = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ inputTexture;
uniform float thresholdBrightness;

void main(void)
{
	vec3 color = texture(inputTexture, textureCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > thresholdBrightness)
	{
		gl_FragColor = vec4(color, 1.0);
	}
	else
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
)";

static const char* FEBloomEffectHorizontalFS = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ inputTexture;
uniform float BloomSize;

void main(void)
{
	vec2 tex_offset = 1.0 / textureSize(inputTexture, 0);

	vec2 blurTextureCoords[11];
	vec2 centerTexCoords = textureCoords; // * 0.5 + 0.5

	for (int i = -5; i <= 5; i++)
	{
		blurTextureCoords[i + 5] = centerTexCoords + vec2(0.0, tex_offset.y * i * BloomSize);
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
)";

static const char* FEBloomEffectVerticalFS = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ inputTexture;
uniform float BloomSize;

void main(void)
{
	vec2 tex_offset = 1.0 / textureSize(inputTexture, 0);

	vec2 blurTextureCoords[11];
	vec2 centerTexCoords = textureCoords;// * 0.5 + 0.5

	for (int i = -5; i <= 5; i++)
	{
		blurTextureCoords[i + 5] = centerTexCoords + vec2(tex_offset.x * i * BloomSize, 0.0);
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
)";

static const char* FEWeakBloomEffectHorizontalFS = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ inputTexture;
					
void main(void)
{
	vec2 tex_offset = 1.0 / textureSize(inputTexture, 0);

	vec2 blurTextureCoords[11];
	vec2 centerTexCoords = textureCoords;

	for (int i = -5; i <= 5; i++)
	{
		blurTextureCoords[i + 5] = centerTexCoords + vec2(0.0, tex_offset.y * i);
	}

	gl_FragColor = vec4(0.0);
	gl_FragColor += texture(inputTexture, blurTextureCoords[0]) * 0.0073;
	gl_FragColor += texture(inputTexture, blurTextureCoords[1]) * 0.020002;
	gl_FragColor += texture(inputTexture, blurTextureCoords[2]) * 0.055984;
	gl_FragColor += texture(inputTexture, blurTextureCoords[3]) * 0.111703;
	gl_FragColor += texture(inputTexture, blurTextureCoords[4]) * 0.165713;
	gl_FragColor += texture(inputTexture, blurTextureCoords[5]) * 0.178596;
	gl_FragColor += texture(inputTexture, blurTextureCoords[6]) * 0.165713;
	gl_FragColor += texture(inputTexture, blurTextureCoords[7]) * 0.111703;
	gl_FragColor += texture(inputTexture, blurTextureCoords[8]) * 0.055984;
	gl_FragColor += texture(inputTexture, blurTextureCoords[9]) * 0.020002;
	gl_FragColor += texture(inputTexture, blurTextureCoords[10]) * 0.0073;
}
)";

static const char* FEWeakBloomEffectVerticalFS = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ inputTexture;

void main(void)
{
	vec2 tex_offset = 1.0 / textureSize(inputTexture, 0);

	vec2 blurTextureCoords[11];
	vec2 centerTexCoords = textureCoords;

	for (int i = -5; i <= 5; i++)
	{
		blurTextureCoords[i + 5] = centerTexCoords + vec2(tex_offset.x * i, 0.0);
	}

	gl_FragColor = vec4(0.0);
	gl_FragColor += texture(inputTexture, blurTextureCoords[0]) * 0.0073;
	gl_FragColor += texture(inputTexture, blurTextureCoords[1]) * 0.020002;
	gl_FragColor += texture(inputTexture, blurTextureCoords[2]) * 0.055984;
	gl_FragColor += texture(inputTexture, blurTextureCoords[3]) * 0.111703;
	gl_FragColor += texture(inputTexture, blurTextureCoords[4]) * 0.165713;
	gl_FragColor += texture(inputTexture, blurTextureCoords[5]) * 0.178596;
	gl_FragColor += texture(inputTexture, blurTextureCoords[6]) * 0.165713;
	gl_FragColor += texture(inputTexture, blurTextureCoords[7]) * 0.111703;
	gl_FragColor += texture(inputTexture, blurTextureCoords[8]) * 0.055984;
	gl_FragColor += texture(inputTexture, blurTextureCoords[9]) * 0.020002;
	gl_FragColor += texture(inputTexture, blurTextureCoords[10]) * 0.0073;
}
)";

static const char* FEBloomEffectFinalFS = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ inputTexture;
@Texture@ otherTexture;

void main(void)
{
	gl_FragColor = vec4(texture(inputTexture, textureCoords).rgb + texture(otherTexture, textureCoords).rgb, 1.0);
}
)";

namespace FocalEngine
{
	class FEBloomEffect : public FEPostProcess
	{
	public:
		FEBloomEffect(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight);
		~FEBloomEffect();

	private:

	};
}