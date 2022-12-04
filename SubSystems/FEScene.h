#pragma once

#include "../ResourceManager/FEResourceManager.h"
#include "../CoreExtensions/FEFreeCamera.h"
#include "../CoreExtensions/FEModelViewCamera.h"
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

		FEEntity* AddEntity(FEGameModel* GameModel, std::string Name = "", std::string ForceObjectID = "");
		FEEntity* AddEntity(FEPrefab* Prefab, std::string Name = "", std::string ForceObjectID = "");
		bool AddEntity(FEEntity* NewEntity);
		FEEntity* GetEntity(std::string ID);
		std::vector<FEEntity*> GetEntityByName(std::string Name);
		std::vector<std::string> GetEntityList();
		void DeleteEntity(std::string ID);

		FEEntityInstanced* AddEntityInstanced(FEPrefab* Prefab, std::string Name = "", std::string ForceObjectID = "");
		FEEntityInstanced* AddEntityInstanced(FEGameModel* GameModel, std::string Name = "", std::string ForceObjectID = "");
		bool AddEntityInstanced(FEEntityInstanced* NewEntityInstanced);
		FEEntityInstanced* GetEntityInstanced(std::string ID);
		std::vector<FEEntityInstanced*> GetEntityInstancedByName(std::string Name);
		void SetSelectMode(FEEntityInstanced* EntityInstanced, bool NewValue);

		FELight* AddLight(FE_OBJECT_TYPE LightType, std::string Name, std::string ForceObjectID = "");
		FELight* GetLight(std::string ID);
		std::vector<std::string> GetLightsList();
		void DeleteLight(std::string ID);

		bool AddTerrain(FETerrain* NewTerrain);
		std::vector<std::string> GetTerrainList();
		FETerrain* GetTerrain(std::string ID);
		void DeleteTerrain(std::string ID);

		void PrepareForGameModelDeletion(const FEGameModel* GameModel);
		void PrepareForPrefabDeletion(const FEPrefab* Prefab);

		void Clear();
	private:
		SINGLETON_PRIVATE_PART(FEScene)

		std::unordered_map<std::string, FEEntity*> EntityMap;
		std::unordered_map<std::string, FETerrain*> TerrainMap;
	};

	#define SCENE FEScene::getInstance()
}