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

		FEEntity* addEntity(FEGameModel* gameModel, std::string Name = "", std::string forceAssetID = "");
		bool addEntity(FEEntity* newEntity);
		FEEntity* getEntity(std::string name);
		std::vector<std::string> getEntityList();
		void deleteEntity(std::string name);
		bool setEntityName(FEEntity* Entity, std::string EntityName);

		FEEntityInstanced* addEntityInstanced(FEGameModel* gameModel, std::string Name = "", std::string forceAssetID = "");
		bool addEntityInstanced(FEEntityInstanced* newEntityInstanced);
		FEEntityInstanced* getEntityInstanced(std::string name);
		/*std::vector<std::string> getEntityInstancedList();
		void deleteEntityInstanced(std::string name);
		bool setEntityInstancedName(FEEntityInstanced* EntityInstanced, std::string EntityInstancedName);*/

		void addLight(FELightType Type, std::string Name);
		FELight* getLight(std::string name);
		std::vector<std::string> getLightsList();

		bool addTerrain(FETerrain* newTerrain);
		std::vector<std::string> getTerrainList();
		FETerrain* getTerrain(std::string name);
		void deleteTerrain(std::string name);
		bool setTerrainName(FETerrain* Terrain, std::string TerrainName);

		void prepareForGameModelDeletion(FEGameModel* gameModel);

		void clear();
	private:
		SINGLETON_PRIVATE_PART(FEScene)

		std::unordered_map<std::string, FEEntity*> entityMap;
		//std::unordered_map<std::string, FEEntityInstanced*> entityInstancedMap;
		std::unordered_map<std::string, FELight*> lightsMap;
		std::unordered_map<std::string, FETerrain*> terrainMap;
	};
}