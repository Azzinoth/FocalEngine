#pragma once

template<typename T>
std::vector<FEUUID> FEScene::GetEntityIDListWithComponent()
{
	std::vector<FEUUID> Result;
	entt::basic_view ComponentView = Registry.view<T>();
	for (entt::entity CurrentEntity : ComponentView)
		Result.push_back(EnttToEntity[CurrentEntity]->GetID());

	return Result;
}

template<typename T>
std::vector<FEEntity*> FEScene::GetEntityListWithComponent()
{
	std::vector<FEEntity*> Result;
	entt::basic_view ComponentView = Registry.view<T>();
	for (entt::entity CurrentEntity : ComponentView)
		Result.push_back(EnttToEntity[CurrentEntity]);

	return Result;
}