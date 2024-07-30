#pragma once

#include "FEScene.h"

namespace FocalEngine
{
	class FESceneManager
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

		FEScene* DuplicateScene(std::string ID, std::string NewSceneName = "");
		FEScene* DuplicateScene(FEScene* SourceScene, std::string NewSceneName = "");
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