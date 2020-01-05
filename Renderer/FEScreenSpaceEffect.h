#pragma once

#include "FEEntity.h"

static const char* const FEScreenQuadVS = R"(
#version 400 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* const FEScreenQuadFS = R"(
#version 400 core

in vec2 textureCoords;
uniform sampler2D quadTexture;

void main(void)
{
	gl_FragColor = texture(quadTexture, textureCoords);
}
)";

namespace FocalEngine
{
	struct FEScreenSpaceEffectStage
	{
		FEScreenSpaceEffectStage(FETexture* InTexture, FEShader* Shader, FETexture* OutTexture)
			: inTexture(InTexture), shader(Shader), outTexture(OutTexture) {};

		FEScreenSpaceEffectStage(FETexture* InTexture, FEShader* Shader)
			: inTexture(InTexture), shader(Shader), outTexture(nullptr) {};

		FETexture* inTexture = nullptr;
		FEShader* shader = nullptr;
		FETexture* outTexture = nullptr;
	};

	class FEScreenQuadShader : public FEShader 
	{
	public:
		FEScreenQuadShader();
	};

	class FEResourceManager;
	class FERenderer;

	class FEScreenSpaceEffect
	{
		friend FEResourceManager;
		friend FERenderer;
	public:
		FEScreenSpaceEffect(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight);
		~FEScreenSpaceEffect();

		FETexture* getInTexture();
		void setInTexture(FETexture* InTexture);

		void addStage(FEScreenSpaceEffectStage* newStage);
	private:
		int screenWidth, screenHeight;
		FEMesh* screenQuad;
		FEShader* screenQuadShader;
		FETexture* inTexture = nullptr;
		FETexture* finalTexture = nullptr;

		FEFramebuffer* intermediateFramebuffer = nullptr;
		std::vector<FEScreenSpaceEffectStage*> stages;
	};
}