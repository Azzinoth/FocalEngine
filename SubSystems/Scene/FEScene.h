#pragma once

#include "../ResourceManager/FEResourceManager.h"
#include "../CoreExtensions/FEFreeCamera.h"
#include "../CoreExtensions/FEModelViewCamera.h"
#include "../Renderer/FELight.h"
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
		void RegisterOnComponentConstructCallback(std::function<void(FEEntity*)> Callback)
		{
			OnComponentConstructCallbacks[std::type_index(typeid(T))] = std::move(Callback);
			Registry.on_construct<T>().connect<&FEScene::OnComponentConstructWrapper<T>>();
		}

		template<typename T>
		void RegisterOnComponentDestroyCallback(std::function<void(FEEntity*)> Callback)
		{
			OnComponentDestroyCallbacks[std::type_index(typeid(T))] = std::move(Callback);
			Registry.on_destroy<T>().connect<&FEScene::OnComponentDestroyWrapper<T>>();
		}

		template<typename T>
		void RegisterOnComponentUpdateCallback(std::function<void(FEEntity*)> Callback)
		{
			OnComponentUpdateCallbacks[std::type_index(typeid(T))] = std::move(Callback);
			Registry.on_update<T>().connect<&FEScene::OnComponentUpdateWrapper<T>>();
		}

		std::vector<FEEntity*> GetEntityByName(std::string Name);
		void DeleteNewEntity(std::string ID);
		void DeleteNewEntity(FEEntity* Entity);

		FEEntity* GetNewStyleEntity(std::string ID);
		FEEntity* AddNewStyleEntity(std::string Name = "", std::string ForceObjectID = "");
		std::vector<std::string> GetNewEntityList();

		FELight* AddLight(FE_OBJECT_TYPE LightType, std::string Name, std::string ForceObjectID = "");
		FELight* GetLight(std::string ID);
		std::vector<std::string> GetLightsList();
		void DeleteLight(std::string ID);

		std::vector<std::string> GetTerrainList();

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

		entt::registry Registry;

		template<typename T>
		static void OnComponentConstructWrapper(entt::registry& Registry, entt::entity EnTTEntity)
		{
			FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
			if (Entity != nullptr)
			{
				auto MapIterator = FEScene::getInstance().OnComponentConstructCallbacks.find(std::type_index(typeid(T)));
				if (MapIterator != FEScene::getInstance().OnComponentConstructCallbacks.end())
					MapIterator->second(Entity);
			}
		}
		std::unordered_map<std::type_index, std::function<void(FEEntity*)>> OnComponentConstructCallbacks;

		template<typename T>
		static void OnComponentDestroyWrapper(entt::registry& Registry, entt::entity EnTTEntity)
		{
			FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
			if (Entity != nullptr)
			{
				auto MapIterator = FEScene::getInstance().OnComponentDestroyCallbacks.find(std::type_index(typeid(T)));
				if (MapIterator != FEScene::getInstance().OnComponentDestroyCallbacks.end())
					MapIterator->second(Entity);
			}
		}
		std::unordered_map<std::type_index, std::function<void(FEEntity*)>> OnComponentDestroyCallbacks;

		template<typename T>
		static void OnComponentUpdateWrapper(entt::registry& Registry, entt::entity EnTTEntity)
		{
			FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
			if (Entity != nullptr)
			{
				auto MapIterator = FEScene::getInstance().OnComponentUpdateCallbacks.find(std::type_index(typeid(T)));
				if (MapIterator != FEScene::getInstance().OnComponentUpdateCallbacks.end())
					MapIterator->second(Entity);
			}
		}
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
		void UpdateSceneGraph();
		bool bSceneGraphInitialization = true;
	};

	#define SCENE FEScene::getInstance()
}