#pragma once

#include "../../Renderer/FEScreenSpaceEffect.h"

static const char* const FEBlurEffectVS = R"(
#version 400 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* FEBlurEffectThresholdFS = R"(
#version 400 core

in vec2 textureCoords;
uniform sampler2D inputTexture;

void main(void)
{
	vec3 color = texture(inputTexture, textureCoords).rgb;
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
	{
		gl_FragColor = vec4(color, 1.0);
	}
	else
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
}
)";

static const char* FEBlurEffectHorizontalFS = R"(
#version 400 core

in vec2 textureCoords;
uniform sampler2D inputTexture;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main(void)
{
	vec2 tex_offset = 1.0 / textureSize(inputTexture, 0); // gets size of single texel
    vec3 result = texture(inputTexture, textureCoords).rgb * weight[0]; // current fragment's contribution
    
	float kernelMult = 2.0;
    for(int i = 1; i < 10; ++i)
    {
		kernelMult += 0.3;
        result += texture(inputTexture, textureCoords + vec2(tex_offset.x * i * kernelMult, 0.0)).rgb * weight[i];
        result += texture(inputTexture, textureCoords - vec2(tex_offset.x * i * kernelMult, 0.0)).rgb * weight[i];
    }

	gl_FragColor = vec4(result, 1.0);
}
)";

static const char* FEBlurEffectVerticalFS = R"(
#version 400 core

in vec2 textureCoords;
uniform sampler2D inputTexture;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main(void)
{
	vec2 tex_offset = 1.0 / textureSize(inputTexture, 0); // gets size of single texel
    vec3 result = texture(inputTexture, textureCoords).rgb * weight[0]; // current fragment's contribution
    
	float kernelMult = 2.0;
    for(int i = 1; i < 10; ++i)
    {
		kernelMult += 0.3;
        result += texture(inputTexture, textureCoords + vec2(0.0, tex_offset.y * i * kernelMult)).rgb * weight[i];
        result += texture(inputTexture, textureCoords - vec2(0.0, tex_offset.y * i * kernelMult)).rgb * weight[i];
    }

	gl_FragColor = vec4(result, 1.0);
}
)";

static const char* FEBlurEffectFinalFS = R"(
#version 420 core

in vec2 textureCoords;
layout (binding = 0) uniform sampler2D inputTexture;
layout (binding = 1) uniform sampler2D otherTexture;
//uniform sampler2D inputTexture;
//uniform sampler2D otherTexture;

void main(void)
{
	gl_FragColor = vec4(texture(inputTexture, textureCoords).rgb + texture(otherTexture, textureCoords).rgb, 1.0);
}
)";

namespace FocalEngine
{
	class FEBlurEffect : public FEScreenSpaceEffect
	{
	public:
		FEBlurEffect(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight, FETexture* sceneTexture);
		~FEBlurEffect();

	private:

	};
}