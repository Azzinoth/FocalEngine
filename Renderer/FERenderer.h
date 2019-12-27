#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../ResourceManager/FEResourceManager.h"
#include "../CoreExtensions/FEFreeCamera.h"

namespace FocalEngine
{
	class FERenderer
	{
	public:
		SINGLETON_PUBLIC_PART(FERenderer)

		void addToScene(FEEntity* newEntity);
		void render(FEBasicCamera* currentCamera);
	private:
		SINGLETON_PRIVATE_PART(FERenderer)
		std::vector<FEEntity*> sceneGraph;
	};
}

#endif FERENDERER_H