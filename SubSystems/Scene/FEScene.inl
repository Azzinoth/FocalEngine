#pragma once

template<typename T>
void FEScene::RegisterOnComponentConstructCallback(std::function<void(FEEntity*)> Callback)
{
	OnComponentConstructCallbacks[std::type_index(typeid(T))] = std::move(Callback);
	Registry.on_construct<T>().connect<&FEScene::OnComponentConstructWrapper<T>>();
	//Registry.on_construct<T>().connect<std::bind(&FEScene::OnComponentConstructWrapper<T>, this)>();


	/*Registry.on_construct<T>().connect([this](entt::registry& registry, entt::entity entity) {
		this->OnComponentConstructWrapper<T>(registry, entity);
	});*/

	//Registry.on_construct<T>().connect<&FEScene::OnComponentConstructWrapper<T>, FEScene>(*this);
}

template<typename T>
void FEScene::RegisterOnComponentDestroyCallback(std::function<void(FEEntity*, bool)> Callback)
{
	OnComponentDestroyCallbacks[std::type_index(typeid(T))] = std::move(Callback);
	Registry.on_destroy<T>().connect<&FEScene::OnComponentDestroyWrapper<T>>();
}

template<typename T>
void FEScene::RegisterOnComponentUpdateCallback(std::function<void(FEEntity*)> Callback)
{
	OnComponentUpdateCallbacks[std::type_index(typeid(T))] = std::move(Callback);
	Registry.on_update<T>().connect<&FEScene::OnComponentUpdateWrapper<T>>();
}

template<typename T>
std::vector<std::string> FEScene::GetEntityIDListWith()
{
	std::vector<std::string> Result;
	entt::basic_view ComponentView = Registry.view<T>();
	for (entt::entity CurrentEntity : ComponentView)
		Result.push_back(EnttToEntity[CurrentEntity]->GetObjectID());

	return Result;
}

template<typename T>
static void FEScene::OnComponentConstructWrapper(entt::registry& Registry, entt::entity EnTTEntity)
{
	FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
	if (Entity != nullptr)
	{
		auto MapIterator = FEScene::getInstance().OnComponentConstructCallbacks.find(std::type_index(typeid(T)));
		if (MapIterator != FEScene::getInstance().OnComponentConstructCallbacks.end())
			MapIterator->second(Entity);
	}
}

template<typename T>
static void FEScene::OnComponentDestroyWrapper(entt::registry& Registry, entt::entity EnTTEntity)
{
	FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
	if (Entity != nullptr)
	{
		auto MapIterator = FEScene::getInstance().OnComponentDestroyCallbacks.find(std::type_index(typeid(T)));
		if (MapIterator != FEScene::getInstance().OnComponentDestroyCallbacks.end())
			MapIterator->second(Entity, FEScene::getInstance().bIsSceneClearing);
	}
}

template<typename T>
static void FEScene::OnComponentUpdateWrapper(entt::registry& Registry, entt::entity EnTTEntity)
{
	FEEntity* Entity = FEScene::getInstance().GetEntityByEnTT(EnTTEntity);
	if (Entity != nullptr)
	{
		auto MapIterator = FEScene::getInstance().OnComponentUpdateCallbacks.find(std::type_index(typeid(T)));
		if (MapIterator != FEScene::getInstance().OnComponentUpdateCallbacks.end())
			MapIterator->second(Entity);
	}
}