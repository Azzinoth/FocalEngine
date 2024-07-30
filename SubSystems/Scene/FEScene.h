#pragma once

#include"FEEntity.h"
#include "../ResourceManager/FEResourceManager.h"
#include "../Renderer/FELine.h"
#include "../FEVirtualUIContext.h"
#include "FENaiveSceneGraph.h"
#include <typeindex>
#include "entt.hpp"

namespace FocalEngine
{
	class FEScene : public FEObject
	{
		friend class FEEntity;
		friend class FENaiveSceneGraph;
		friend class FESceneManager;
		friend class FEInstancedSystem;
		friend class FERenderer;
		friend class FEngine;
		friend class FETransformSystem;
	public:
		FEScene();

		FEEntity* GetEntity(std::string ID);
		FEEntity* CreateEntity(std::string Name = "", std::string ForceObjectID = "");
		// Entity will not be added to the scene graph, use with caution.
		FEEntity* CreateEntityOrphan(std::string Name = "", std::string ForceObjectID = "");
		std::vector<std::string> GetEntityIDList();
		template<typename T>
		std::vector<std::string> GetEntityIDListWith();
		template<typename T>
		std::vector<FEEntity*> GetEntityListWith();
		std::vector<FEEntity*> GetEntityByName(std::string Name);

		FEEntity* DuplicateEntity(std::string ID, std::string NewEntityName = "");
		FEEntity* DuplicateEntity(FEEntity* SourceEntity, std::string NewEntityName = "");

		FEEntity* ImportEntity(FEEntity* EntityFromDifferentScene, FENaiveSceneGraphNode* TargetParent = nullptr);

		void DeleteEntity(std::string ID);
		void DeleteEntity(FEEntity* Entity);

		FEVirtualUIContext* AddVirtualUIContext(int Width = 1280, int Height = 720, FEMesh* SampleMesh = nullptr, std::string Name = "UnNamed");
		FEVirtualUIContext* GetVirtualUIContext(std::string ID);
		std::vector<std::string> GetVirtualUIContextList();
		void DeleteVirtualUIContext(std::string ID);

		void PrepareForGameModelDeletion(const FEGameModel* GameModel);
		void PrepareForPrefabDeletion(const FEPrefab* Prefab);

		void Clear();

		std::vector<FEObject*> ImportAsset(std::string FileName);
		
		void Update();
		FENaiveSceneGraph SceneGraph;
	private:
		~FEScene();

		entt::registry Registry;
		bool bIsSceneClearing = false;
		bool bActive = true;

		std::unordered_map<entt::entity, FEEntity*> EnttToEntity;
		std::unordered_map<std::string, FEEntity*> EntityMap;
		// FIX ME! should be a component
		std::unordered_map<std::string, FEVirtualUIContext*> VirtualUIContextMap;

		FEEntity* GetEntityByEnTT(entt::entity ID);

		std::vector<FEObject*> LoadGLTF(std::string FileName);
		std::vector<FEObject*> AddGLTFNodeToSceneGraph(const FEGLTFLoader& GLTF, const GLTFNodes& Node, const std::unordered_map<int, std::vector<FEPrefab*>>& GLTFMeshesToPrefabMap, const std::string ParentID);

		FEEntity* CreateEntityInternal(std::string Name = "", std::string ForceObjectID = "");
		FEEntity* DuplicateEntityInternal(FEEntity* SourceEntity, std::string NewEntityName = "");
	};
#include "FEScene.inl"
}