#pragma once

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
std::vector<FEEntity*> FEScene::GetEntityListWith()
{
	std::vector<FEEntity*> Result;
	entt::basic_view ComponentView = Registry.view<T>();
	for (entt::entity CurrentEntity : ComponentView)
		Result.push_back(EnttToEntity[CurrentEntity]);

	return Result;
}