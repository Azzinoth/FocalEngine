#pragma once

#include "../ResourceManager/FEResourceManager.h"
#include "../CoreExtensions/FEFreeCamera.h"
#include "../CoreExtensions/FEModelViewCamera.h"
#include "../Renderer/FELine.h"
#include "../FEVirtualUIContext.h"
#include "FENaiveSceneGraph.h"

#include <typeindex>
#include "entt.hpp"

namespace FocalEngine
{
	class FEScene
	{
		friend class FERenderer;
		friend class FEngine;
		friend class FEEntity;
		friend class FEInstancedSystem;
	public:
		SINGLETON_PUBLIC_PART(FEScene)

		void AddOnSceneClearCallback(std::function<void()> Callback);

		template<typename T>
		void RegisterOnComponentConstructCallback(std::function<void(FEEntity*)> Callback);
		template<typename T>
		void RegisterOnComponentDestroyCallback(std::function<void(FEEntity*)> Callback);
		template<typename T>
		void RegisterOnComponentUpdateCallback(std::function<void(FEEntity*)> Callback);

		FEEntity* GetEntity(std::string ID);
		FEEntity* AddEntity(std::string Name = "", std::string ForceObjectID = "");
		// Entity will not be added to the scene graph, use with caution.
		FEEntity* AddEntityOrphan(std::string Name = "", std::string ForceObjectID = "");
		std::vector<std::string> GetEntityIDList();
		template<typename T>
		std::vector<std::string> GetEntityIDListWith();
		std::vector<FEEntity*> GetEntityByName(std::string Name);

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
		SINGLETON_PRIVATE_PART(FEScene)

		entt::registry Registry;

		template<typename T>
		static void OnComponentConstructWrapper(entt::registry& Registry, entt::entity EnTTEntity);
		std::unordered_map<std::type_index, std::function<void(FEEntity*)>> OnComponentConstructCallbacks;

		template<typename T>
		static void OnComponentDestroyWrapper(entt::registry& Registry, entt::entity EnTTEntity);
		std::unordered_map<std::type_index, std::function<void(FEEntity*)>> OnComponentDestroyCallbacks;

		template<typename T>
		static void OnComponentUpdateWrapper(entt::registry& Registry, entt::entity EnTTEntity);
		std::unordered_map<std::type_index, std::function<void(FEEntity*)>> OnComponentUpdateCallbacks;

		std::vector<std::function<void()>> OnSceneClearCallbacks;
		void ReRegisterOnComponentCallbacks();

		std::unordered_map<entt::entity, FEEntity*> EnttToEntity;
		std::unordered_map<std::string, FEEntity*> EntityMap;
		// FIX ME! should be a component
		std::unordered_map<std::string, FEVirtualUIContext*> VirtualUIContextMap;

		FEEntity* GetEntityByEnTT(entt::entity ID);

		std::vector<FEObject*> LoadGLTF(std::string FileName);
		std::vector<FEObject*> AddGLTFNodeToSceneGraph(const FEGLTFLoader& GLTF, const GLTFNodes& Node, const std::unordered_map<int, std::vector<FEPrefab*>>& GLTFMeshesToPrefabMap, const std::string ParentID);

		void TransformUpdate(FENaiveSceneGraphNode* SubTreeRoot);
		FEEntity* AddEntityInternal(std::string Name = "", std::string ForceObjectID = "");
	};

#include "FEScene.inl"

	#define SCENE FEScene::getInstance()
}