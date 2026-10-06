#pragma once

template<typename T>
void FEResourceManager::ClearResource(std::unordered_map<FEUUID, T*>& ResourceMap)
{
	auto Iterator = ResourceMap.begin();
	while (Iterator != ResourceMap.end())
	{
		if (std::find(TagsThatWillPreventDeletion.begin(), TagsThatWillPreventDeletion.end(),
			Iterator->second->GetTag()) == TagsThatWillPreventDeletion.end())
		{
			delete Iterator->second;
			Iterator = ResourceMap.erase(Iterator);
		}
		else
		{
			Iterator++;
		}
	}
}

template<typename T>
std::vector<FEUUID> FEResourceManager::GetResourceIDList(const std::unordered_map<FEUUID, T*>& Resources)
{
	std::vector<FEUUID> Result;

	auto Iterator = Resources.begin();
	while (Iterator != Resources.end())
	{
		Result.push_back(Iterator->first);
		Iterator++;
	}

	return Result;
}

template<typename T>
std::vector<FEUUID> FEResourceManager::GetResourceIDListByTag(const std::unordered_map<FEUUID, T*>& Resources, const std::string& Tag)
{
	std::vector<FEUUID> Result;

	auto Iterator = Resources.begin();
	while (Iterator != Resources.end())
	{
		if (Iterator->second->GetTag() == Tag)
			Result.push_back(Iterator->first);

		Iterator++;
	}

	return Result;
}