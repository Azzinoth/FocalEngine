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

		void addEntity(FEMesh* Mesh, FEMaterial* Material = nullptr, std::string Name = "");
		FEEntity* getEntity(std::string name);
		std::vector<std::string> getEntityList();

		void addLight(FELightType Type, std::string Name);
		FELight* getLight(std::string name);
		std::vector<std::string> getLightsList();
	private:
		SINGLETON_PRIVATE_PART(FEScene)

		std::unordered_map<std::string, FEEntity*> entityMap;
		std::unordered_map<std::string, FELight*> lightsMap;
	};
}