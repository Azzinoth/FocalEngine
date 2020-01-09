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
		void addPostProcess(FEPostProcess* newPostProcess);

		std::vector<std::string> getPostProcessList();
		FEPostProcess* getPostProcessEffect(std::string name);
	private:
		SINGLETON_PRIVATE_PART(FERenderer)
		void loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEEntity* entity);

		void standardFBInit(int WindowWidth, int WindowHeight);
		FEFramebuffer* sceneToTextureFB = nullptr;

		std::vector<FEPostProcess*> postProcessEffects;
	};
}

#endif FERENDERER_H