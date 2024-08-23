#pragma once

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetSceneManager();
	#define SCENE_MANAGER (*static_cast<FESceneManager*>(GetSceneManager()))
#else
	#define SCENE_MANAGER FESceneManager::GetInstance()
#endif

template<typename T>
void FESceneManager::RegisterOnComponentConstructCallback(std::function<void(FEEntity*)> Callback)
{
	// Only one callback per component type is allowed.
	if (OnComponentConstructCallbacks.find(std::type_index(typeid(T))) == OnComponentConstructCallbacks.end())
		OnComponentConstructCallbacks[std::type_index(typeid(T))] = std::move(Callback);
}

template<typename T>
void FESceneManager::RegisterOnComponentDestroyCallback(std::function<void(FEEntity*, bool)> Callback)
{
	// Only one callback per component type is allowed.
	if (OnComponentDestroyCallbacks.find(std::type_index(typeid(T))) == OnComponentDestroyCallbacks.end())
		OnComponentDestroyCallbacks[std::type_index(typeid(T))] = std::move(Callback);
}

template<typename T>
void FESceneManager::RegisterOnComponentUpdateCallback(std::function<void(FEEntity*)> Callback)
{
	// Only one callback per component type is allowed.
	if (OnComponentUpdateCallbacks.find(std::type_index(typeid(T))) == OnComponentUpdateCallbacks.end())
		OnComponentUpdateCallbacks[std::type_index(typeid(T))] = std::move(Callback);
}

template<typename T>
static void FESceneManager::OnComponentConstructWrapper(entt::registry& Registry, entt::entity EnTTEntity)
{
	entt::registry* RegistryPointer = &Registry;
	auto SceneIterator = SCENE_MANAGER.ActiveSceneMap.begin();
	while (SceneIterator != SCENE_MANAGER.ActiveSceneMap.end())
	{
		if (&SceneIterator->second->Registry == RegistryPointer)
		{
			FEEntity* Entity = SceneIterator->second->GetEntityByEnTT(EnTTEntity);
			if (Entity != nullptr)
			{
				auto CallBackMapIterator = SCENE_MANAGER.OnComponentConstructCallbacks.find(std::type_index(typeid(T)));
				if (CallBackMapIterator != SCENE_MANAGER.OnComponentConstructCallbacks.end())
					CallBackMapIterator->second(Entity);

				break;
			}
		}

		SceneIterator++;
	}
}

template<typename T>
static void FESceneManager::OnComponentDestroyWrapper(entt::registry& Registry, entt::entity EnTTEntity)
{
	entt::registry* RegistryPointer = &Registry;
	auto SceneIterator = SCENE_MANAGER.ActiveSceneMap.begin();
	while (SceneIterator != SCENE_MANAGER.ActiveSceneMap.end())
	{
		if (&SceneIterator->second->Registry == RegistryPointer)
		{
			FEEntity* Entity = SceneIterator->second->GetEntityByEnTT(EnTTEntity);
			if (Entity != nullptr)
			{
				auto CallBackMapIterator = SCENE_MANAGER.OnComponentDestroyCallbacks.find(std::type_index(typeid(T)));
				if (CallBackMapIterator != SCENE_MANAGER.OnComponentDestroyCallbacks.end())
					CallBackMapIterator->second(Entity, SceneIterator->second->bIsSceneClearing);

				break;
			}
		}

		SceneIterator++;
	}
}

template<typename T>
static void FESceneManager::OnComponentUpdateWrapper(entt::registry& Registry, entt::entity EnTTEntity)
{
	entt::registry* RegistryPointer = &Registry;
	auto SceneIterator = SCENE_MANAGER.ActiveSceneMap.begin();
	while (SceneIterator != SCENE_MANAGER.ActiveSceneMap.end())
	{
		if (&SceneIterator->second->Registry == RegistryPointer)
		{
			FEEntity* Entity = SceneIterator->second->GetEntityByEnTT(EnTTEntity);
			if (Entity != nullptr)
			{
				auto CallBackMapIterator = SCENE_MANAGER.OnComponentUpdateCallbacks.find(std::type_index(typeid(T)));
				if (CallBackMapIterator != SCENE_MANAGER.OnComponentUpdateCallbacks.end())
					CallBackMapIterator->second(Entity);

				break;
			}
		}

		SceneIterator++;
	}
}