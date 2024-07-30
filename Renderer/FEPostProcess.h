#pragma once

#include "FEFramebuffer.h"
#include "FEPrefab.h"

namespace FocalEngine
{
	enum FE_POST_PROCESS_SOURCES
	{
		FE_POST_PROCESS_PREVIOUS_STAGE_RESULT0 = 0,
		FE_POST_PROCESS_SCENE_HDR_COLOR = 1,
		FE_POST_PROCESS_SCENE_DEPTH = 2,
		FE_POST_PROCESS_OWN_TEXTURE = 3,
	};

	struct FEPostProcessStage
	{
		FEPostProcessStage(int InTextureSource, FEShader* Shader);
		FEPostProcessStage(std::vector<int>&& InTextureSource, FEShader* Shader);

		std::vector<FEShaderParam> StageSpecificUniforms;
		std::vector<int> InTextureSource;
		std::vector<FETexture*> InTexture;
		FEShader* Shader = nullptr;
		FETexture* OutTexture = nullptr;
	};

	class FEResourceManager;
	class FERenderer;

	class FEPostProcess : public FEObject
	{
		friend FEResourceManager;
		friend FERenderer;
	public:
		~FEPostProcess();

		void RenderResult();

		FETexture* GetInTexture();
		void AddStage(FEPostProcessStage* NewStage);

		bool bActive = true;
		bool ReplaceOutTexture(size_t StageIndex, FETexture* NewTexture, bool bDeleteOldTexture = true);
	private:
		FEPostProcess(std::string Name);

		int ScreenWidth, ScreenHeight;
		static FEMesh* ScreenQuad;
		static FEShader* ScreenQuadShader;
		FETexture* InTexture = nullptr;
		FETexture* FinalTexture = nullptr;

		FEFramebuffer* IntermediateFramebuffer = nullptr;
		std::vector<FETexture*> TexturesToDelete;
	public:
		std::vector<FEPostProcessStage*> Stages;
	};
}