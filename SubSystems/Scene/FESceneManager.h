#pragma once

#include "FEScene.h"

namespace FocalEngine
{
	class FEDummyScene
	{
		friend class FESceneManager;
		entt::registry Registry;

		std::string ID;
	public:
		FEDummyScene()
		{
			ID = APPLICATION.GetUniqueHexID();
			//Scene = new FEScene();
		}
	};

	class FESceneManager
	{
		friend class FERenderer;
		friend class FEngine;
		friend class FEEntity;
		friend class FEInstancedSystem;
	public:
		SINGLETON_PUBLIC_PART(FESceneManager)

		FEScene* GetScene(std::string ID);
		FEScene* AddScene(std::string Name = "", std::string ForceObjectID = "");
		std::vector<std::string> GetSceneIDList();
		std::vector<FEScene*> GetSceneByName(std::string Name);

		void DeleteScene(std::string ID);
		void DeleteScene(FEScene* Scene);

		//void SetMainScene(FEScene* Scene);

		template<typename T>
		void RegisterOnComponentConstructCallback(std::function<void(FEEntity*)> Callback);

		void Update();
	private:
		SINGLETON_PRIVATE_PART(FESceneManager)

		std::unordered_map<std::string, FEScene*> SceneMap;
		std::unordered_map<std::string, FEDummyScene*> DummySceneMap;
		std::unordered_map<std::string, FEScene*> ActiveSceneMap;

		template<typename T>
		static void OnComponentConstructWrapper(entt::registry& Registry, entt::entity EnTTEntity);
		std::unordered_map<std::type_index, std::function<void(FEEntity*)>> OnComponentConstructCallbacks;
	};

#include "FESceneManager.inl"

	#define SCENE_MANAGER FESceneManager::getInstance()
}