#pragma once

#include "FEEntity.h"

namespace FocalEngine
{
	enum FEPostProcessSources
	{
		FEPP_PREVIOUS_STAGE_RESULT0 = 0,
		FEPP_SCENE_HDR_COLOR = 1,
		FEPP_SCENE_DEPTH = 2,
		FEPP_OWN_TEXTURE = 3,
	};

	struct FEPostProcessStage
	{
		FEPostProcessStage(int InTextureSource, FEShader* Shader);
		FEPostProcessStage(std::vector<int>&& InTextureSource, FEShader* Shader);

		std::vector<FEShaderParam> stageSpecificUniforms;
		std::vector<int> inTextureSource;
		std::vector<FETexture*> inTexture;
		FEShader* shader = nullptr;
		FETexture* outTexture = nullptr;
	};

	class FEResourceManager;
	class FERenderer;

	class FEPostProcess
	{
		friend FEResourceManager;
		friend FERenderer;
	public:
		~FEPostProcess();

		void renderResult();

		FETexture* getInTexture();
		void addStage(FEPostProcessStage* newStage);

		std::string getName();
		void setName(std::string newName);

		bool active = true;
		bool replaceOutTexture(size_t stageIndex, FETexture* newTexture, bool deleteOldTexture = true);
	private:
		FEPostProcess(std::string Name);

		std::string name;

		int screenWidth, screenHeight;
		FEMesh* screenQuad;
		FEShader* screenQuadShader;
		FETexture* inTexture = nullptr;
		FETexture* finalTexture = nullptr;

		FEFramebuffer* intermediateFramebuffer = nullptr;
		std::vector<FETexture*> texturesToDelete;
	public:
		std::vector<FEPostProcessStage*> stages;
	};
}