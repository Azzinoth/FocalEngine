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

		FEEntity* addEntity(FEMesh* Mesh, FEMaterial* Material = nullptr, std::string Name = "");
		FEEntity* getEntity(std::string name);
		std::vector<std::string> getEntityList();
		void deleteEntity(std::string name);

		void addLight(FELightType Type, std::string Name);
		FELight* getLight(std::string name);
		std::vector<std::string> getLightsList();

		void clear();
	private:
		SINGLETON_PRIVATE_PART(FEScene)

		std::unordered_map<std::string, FEEntity*> entityMap;
		std::unordered_map<std::string, FELight*> lightsMap;
	};
}