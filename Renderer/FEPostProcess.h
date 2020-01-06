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
@Texture@ quadTexture;

void main(void)
{
	gl_FragColor = texture(quadTexture, textureCoords);
}
)";

namespace FocalEngine
{
	enum FEPostProcessSources
	{
		FEPP_PREVIOUS_STAGE_RESULT0 = 0,
		FEPP_SCENE_HDR_COLOR = 1,
		FEPP_SCENE_DEPTH = 2,
	};

	struct FEPostProcessStage
	{
		FEPostProcessStage(int InTextureSource, FEShader* Shader);
		FEPostProcessStage(std::vector<int>&& InTextureSource, FEShader* Shader);

		std::vector<int> inTextureSource;
		std::vector<FETexture*> inTexture;
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

	class FEPostProcess
	{
		friend FEResourceManager;
		friend FERenderer;
	public:
		FEPostProcess(FEMesh* ScreenQuad, int ScreenWidth, int ScreenHeight);
		~FEPostProcess();

		FETexture* getInTexture();
		void setInTexture(FETexture* InTexture);

		void addStage(FEPostProcessStage* newStage);
	private:
		int screenWidth, screenHeight;
		FEMesh* screenQuad;
		FEShader* screenQuadShader;
		FETexture* inTexture = nullptr;
		FETexture* finalTexture = nullptr;

		FEFramebuffer* intermediateFramebuffer = nullptr;
		std::vector<FEPostProcessStage*> stages;
	};
}