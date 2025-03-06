#pragma once

template<typename T>
void FEResourceManager::ClearResource(std::unordered_map<std::string, T*>& ResourceMap)
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
std::vector<std::string> FEResourceManager::GetResourceIDListByTag(const std::unordered_map<std::string, T*>& Resources, const std::string& Tag)
{
	std::vector<std::string> Result;

	auto Iterator = Resources.begin();
	while (Iterator != Resources.end())
	{
		if (Iterator->second->GetTag() == Tag)
			Result.push_back(Iterator->second->GetObjectID());

		Iterator++;
	}

	return Result;
}