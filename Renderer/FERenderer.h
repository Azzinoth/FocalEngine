#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../ResourceManager/FEResourceManager.h"
#include "../SubSystems/FEBasicCamera.h"

namespace FocalEngine
{
	class FERenderer
	{
	public:
		static FERenderer& getInstance()
		{
			if (!_instance)
				_instance = new FERenderer();

			return *_instance;
		}

		void addToScene(FEEntity* newEntity);
		void render(FEBasicCamera* currentCamera);
	private:
		static FERenderer* _instance;
		FERenderer();
		SINGLETON_COPY_ASSIGN_PART(FERenderer)
		std::vector<FEEntity*> sceneGraph;
	};
}

#endif FERENDERER_H