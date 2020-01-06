#pragma once

#include "FEBlurEffect.h"

static const char* const FEGammaAndHDRVS = R"(
#version 400 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* FEGammaAndHDRFS = R"(
#version 400 core

in vec2 textureCoords;
uniform float FEExposure;
uniform float FEGamma;
@Texture@ inputTexture;

void main(void)
{
    vec3 hdrColor = texture(inputTexture, textureCoords).rgb;
  
    // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * FEExposure);
    // Gamma correction 
    mapped = pow(mapped, vec3(1.0 / FEGamma));
  
    gl_FragColor = vec4(mapped, 1.0);
}
)";

namespace FocalEngine
{
	class FEGammaAndHDRCorrection : public FEPostProcess
	{
	public:
		FEGammaAndHDRCorrection(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight);
		~FEGammaAndHDRCorrection();

	private:

	};
}