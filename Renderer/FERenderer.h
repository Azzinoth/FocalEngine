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
	private:
		SINGLETON_PRIVATE_PART(FERenderer)
		void loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEEntity* entity);

		void standardFBInit(int WindowWidth, int WindowHeight);
		FEFramebuffer* sceneToTextureFB = nullptr;
		FEFramebuffer* sceneToTextureFB_DELETE = nullptr;
	};
}

#endif FERENDERER_H