#pragma once

#include "../ResourceManager/FEResourceManager.h"
#include "../CoreExtensions/FEFreeCamera.h"
#include "../Renderer/FELight.h"
#include "../Renderer/FELine.h"

namespace FocalEngine
{
	class FERenderer;
	class FEScene
	{
		friend FERenderer;
	public:
		SINGLETON_PUBLIC_PART(FEScene)

		FEEntity* addEntity(FEGameModel* gameModel, std::string Name = "", std::string forceObjectID = "");
		bool addEntity(FEEntity* newEntity);
		FEEntity* getEntity(std::string ID);
		std::vector<std::string> getEntityList();
		void deleteEntity(std::string ID);

		FEEntityInstanced* addEntityInstanced(FEGameModel* gameModel, std::string Name = "", std::string forceObjectID = "");
		bool addEntityInstanced(FEEntityInstanced* newEntityInstanced);
		FEEntityInstanced* getEntityInstanced(std::string ID);
		void setSelectMode(FEEntityInstanced* entityInstanced, bool newValue);

		FELight* addLight(FEObjectType lightType, std::string Name, std::string forceObjectID = "");
		FELight* getLight(std::string ID);
		std::vector<std::string> getLightsList();
		void deleteLight(std::string ID);

		bool addTerrain(FETerrain* newTerrain);
		std::vector<std::string> getTerrainList();
		FETerrain* getTerrain(std::string ID);
		void deleteTerrain(std::string ID);

		void prepareForGameModelDeletion(FEGameModel* gameModel);

		void clear();

		//FEOctree* testTree;
	private:
		SINGLETON_PRIVATE_PART(FEScene)

		std::unordered_map<std::string, FEEntity*> entityMap;
		std::unordered_map<std::string, FETerrain*> terrainMap;
	};
}