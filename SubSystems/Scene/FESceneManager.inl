#pragma once

template<typename T>
void FESceneManager::RegisterOnComponentConstructCallback(std::function<void(FEEntity*)> Callback)
{
	// Only one callback per component type is allowed.
	if (OnComponentConstructCallbacks.find(std::type_index(typeid(T))) == OnComponentConstructCallbacks.end())
		OnComponentConstructCallbacks[std::type_index(typeid(T))] = std::move(Callback);

	auto SceneIterator = ActiveSceneMap.begin();
	while (SceneIterator != ActiveSceneMap.end())
	{
		SceneIterator->second->Registry.on_construct<T>().connect<&FESceneManager::OnComponentConstructWrapper<T>>();

		SceneIterator++;
	}
	//Registry.on_construct<T>().connect<&FESceneManager::OnComponentConstructWrapper<T>>();
}

//template<typename T>
//void FEScene::RegisterOnComponentDestroyCallback(std::function<void(FEEntity*, bool)> Callback)
//{
//	OnComponentDestroyCallbacks[std::type_index(typeid(T))] = std::move(Callback);
//	Registry.on_destroy<T>().connect<&FEScene::OnComponentDestroyWrapper<T>>();
//}
//
//template<typename T>
//void FEScene::RegisterOnComponentUpdateCallback(std::function<void(FEEntity*)> Callback)
//{
//	OnComponentUpdateCallbacks[std::type_index(typeid(T))] = std::move(Callback);
//	Registry.on_update<T>().connect<&FEScene::OnComponentUpdateWrapper<T>>();
//}
//
//template<typename T>
//std::vector<std::string> FEScene::GetEntityIDListWith()
//{
//	std::vector<std::string> Result;
//	entt::basic_view ComponentView = Registry.view<T>();
//	for (entt::entity CurrentEntity : ComponentView)
//		Result.push_back(EnttToEntity[CurrentEntity]->GetObjectID());
//
//	return Result;
//}

template<typename T>
static void FESceneManager::OnComponentConstructWrapper(entt::registry& Registry, entt::entity EnTTEntity)
{
	entt::registry* RegistryPointer = &Registry;

	auto SceneIterator = FESceneManager::getInstance().ActiveSceneMap.begin();
	while (SceneIterator != FESceneManager::getInstance().ActiveSceneMap.end())
	{
		if (&SceneIterator->second->Registry == RegistryPointer)
		{
			FEEntity* Entity = SceneIterator->second->GetEntityByEnTT(EnTTEntity);
			if (Entity != nullptr)
			{
				//auto MapIterator = FEScene::getInstance().OnComponentConstructCallbacks.find(std::type_index(typeid(T)));
				//if (MapIterator != FEScene::getInstance().OnComponentConstructCallbacks.end())
				//	MapIterator->second(Entity);

				break;
			}
		}

		

		SceneIterator++;
	}

	/*FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
	if (Entity != nullptr)
	{
		auto MapIterator = FEScene::getInstance().OnComponentConstructCallbacks.find(std::type_index(typeid(T)));
		if (MapIterator != FEScene::getInstance().OnComponentConstructCallbacks.end())
			MapIterator->second(Entity);
	}*/
}

//template<typename T>
//static void FEScene::OnComponentDestroyWrapper(entt::registry& Registry, entt::entity EnTTEntity)
//{
//	FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
//	if (Entity != nullptr)
//	{
//		auto MapIterator = FEScene::getInstance().OnComponentDestroyCallbacks.find(std::type_index(typeid(T)));
//		if (MapIterator != FEScene::getInstance().OnComponentDestroyCallbacks.end())
//			MapIterator->second(Entity, FEScene::getInstance().bIsSceneClearing);
//	}
//}
//
//template<typename T>
//static void FEScene::OnComponentUpdateWrapper(entt::registry& Registry, entt::entity EnTTEntity)
//{
//	FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
//	if (Entity != nullptr)
//	{
//		auto MapIterator = FEScene::getInstance().OnComponentUpdateCallbacks.find(std::type_index(typeid(T)));
//		if (MapIterator != FEScene::getInstance().OnComponentUpdateCallbacks.end())
//			MapIterator->second(Entity);
//	}
//}