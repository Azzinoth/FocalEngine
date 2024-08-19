#pragma once

#include "FEScene.h"

namespace FocalEngine
{
	class FOCALENGINE_API FESceneManager
	{
		friend class FERenderer;
		friend class FEngine;
		friend class FEEntity;
		friend class FEInstancedSystem;
	public:
		SINGLETON_PUBLIC_PART(FESceneManager)

		FEScene* GetScene(std::string ID);
		FEScene* CreateScene(std::string Name = "", std::string ForceObjectID = "", bool bActive = true);
		std::vector<std::string> GetSceneIDList();
		std::vector<FEScene*> GetSceneByName(std::string Name);

		void DeleteScene(std::string ID);
		void DeleteScene(FEScene* Scene);

		FEScene* DuplicateScene(std::string ID, std::string NewSceneName = "");
		FEScene* DuplicateScene(FEScene* SourceScene, std::string NewSceneName = "");

		std::vector<FENaiveSceneGraphNode*> ImportSceneAsNode(FEScene* SourceScene, FEScene* TargetScene, FENaiveSceneGraphNode* TargetParent = nullptr, std::function<bool (FEEntity*)> Filter = nullptr);

		template<typename T>
		void RegisterOnComponentConstructCallback(std::function<void(FEEntity*)> Callback);
		template<typename T>
		void RegisterOnComponentDestroyCallback(std::function<void(FEEntity*, bool)> Callback);
		template<typename T>
		void RegisterOnComponentUpdateCallback(std::function<void(FEEntity*)> Callback);

		void Update();

		std::vector<FEScene*> GetAllScenes();
		std::vector<FEScene*> GetActiveScenes();

		void ActivateScene(std::string ID);
		void ActivateScene(FEScene* Scene);

		void DeactivateScene(std::string ID);
		void DeactivateScene(FEScene* Scene);

		void RegisterAllComponentCallbacks(FEScene* NewScene);

		// Checks if two scene graph hierarchies are equivalent.
		// It will not check entities, only the hierarchy structure.
		bool AreSceneGraphHierarchiesEquivalent(FENaiveSceneGraphNode* FirstStaringNode, FENaiveSceneGraphNode* SecondStartingNode, bool bCheckNames = false);
	private:
		SINGLETON_PRIVATE_PART(FESceneManager)

		std::unordered_map<std::string, FEScene*> SceneMap;
		std::unordered_map<std::string, FEScene*> ActiveSceneMap;

		template<typename T>
		static void OnComponentConstructWrapper(entt::registry& Registry, entt::entity EnTTEntity);
		std::unordered_map<std::type_index, std::function<void(FEEntity*)>> OnComponentConstructCallbacks;

		template<typename T>
		static void OnComponentDestroyWrapper(entt::registry& Registry, entt::entity EnTTEntity);
		std::unordered_map<std::type_index, std::function<void(FEEntity*, bool)>> OnComponentDestroyCallbacks;

		template<typename T>
		static void OnComponentUpdateWrapper(entt::registry& Registry, entt::entity EnTTEntity);
		std::unordered_map<std::type_index, std::function<void(FEEntity*)>> OnComponentUpdateCallbacks;

		std::unordered_map<std::type_index, void*> registry;
	};
#include "FESceneManager.inl"
}