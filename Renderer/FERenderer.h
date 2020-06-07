#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../SubSystems/FEScene.h"

namespace FocalEngine
{
	class FEngine;

	class FERenderer
	{
		friend FEngine;
	public:
		SINGLETON_PUBLIC_PART(FERenderer)

		void render(FEBasicCamera* currentCamera);
		void renderEntity(FEEntity* entity, FEBasicCamera* currentCamera);
		void addPostProcess(FEPostProcess* newPostProcess, bool noProcessing = false);

		std::vector<std::string> getPostProcessList();
		FEPostProcess* getPostProcessEffect(std::string name);

		//#fix postProcess
		FEFramebuffer* sceneToTextureFB = nullptr;
		std::vector<FEPostProcess*> postProcessEffects;
	private:
		SINGLETON_PRIVATE_PART(FERenderer)
		void loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEEntity* entity);
		void loadUniformBlocks();

		void standardFBInit(int WindowWidth, int WindowHeight);
		
		// in current version only shadows from one directional light is supported.
		FETexture* shadowMap = nullptr;

		void takeScreenshot(const char* fileName, int width, int height);
	};
}

#endif FERENDERER_H