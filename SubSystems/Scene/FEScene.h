#pragma once

#include "../ResourceManager/FEResourceManager.h"
#include "../CoreExtensions/FEFreeCamera.h"
#include "../CoreExtensions/FEModelViewCamera.h"
#include "../Renderer/FELight.h"
#include "../Renderer/FELine.h"
#include "../FEVirtualUIContext.h"
#include "FENaiveSceneGraph.h"

namespace FocalEngine
{
	class FEScene
	{
		friend class FERenderer;
		friend class FEngine;
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

		FEVirtualUIContext* AddVirtualUIContext(int Width = 1280, int Height = 720, FEMesh* SampleMesh = nullptr, std::string Name = "UnNamed");
		FEVirtualUIContext* GetVirtualUIContext(std::string ID);
		std::vector<std::string> GetVirtualUIContextList();
		void DeleteVirtualUIContext(std::string ID);

		void PrepareForGameModelDeletion(const FEGameModel* GameModel);
		void PrepareForPrefabDeletion(const FEPrefab* Prefab);

		void Clear();

		std::vector<FEObject*> ImportAsset(std::string FileName);
		

		FENaiveSceneGraph SceneGraph;
	private:
		SINGLETON_PRIVATE_PART(FEScene)

		std::unordered_map<std::string, FEEntity*> EntityMap;
		std::unordered_map<std::string, FETerrain*> TerrainMap;
		std::unordered_map<std::string, FEVirtualUIContext*> VirtualUIContextMap;

		void AddEntityToEntityMap(FEEntity* Entity);
		//void AddEntityToSceneGraph(FEEntity* Entity);

		std::vector<FEObject*> LoadGLTF(std::string FileName);
		std::vector<FEObject*> AddGLTFNodeToSceneGraph(const FEGLTFLoader& GLTF, const GLTFNodes& Node, const std::unordered_map<int, FEPrefab*>& PrefabMap, const std::string ParentID);
	};

	#define SCENE FEScene::getInstance()
}