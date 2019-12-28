#pragma once

#include "../ResourceManager/FEResourceManager.h"
#include "../CoreExtensions/FEFreeCamera.h"
#include "../Renderer/FELight.h"

namespace FocalEngine
{
	class FERenderer;
	class FEScene
	{
		friend FERenderer;
	public:
		SINGLETON_PUBLIC_PART(FEScene)

		void add(FEEntity* newEntity);
		void add(FELight* newLight);
	private:
		SINGLETON_PRIVATE_PART(FEScene)

		std::unordered_map<std::string, FEEntity*> entityMap;
		std::vector<FELight*> sceneLights;
	};
}