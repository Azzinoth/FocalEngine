#include "FESceneManager.h"
using namespace FocalEngine;

FESceneManager* FESceneManager::Instance = nullptr;

FESceneManager::FESceneManager()
{
	//FEDummyScene* DummyScene = new FEDummyScene();
	//entt::registry* FirstRegistry = &DummyScene->Registry;
	//DummySceneMap[DummyScene->ID] = DummyScene;
	//FEDummyScene* SecondDummyScene = new FEDummyScene();
	//entt::registry* SecondRegistry = &SecondDummyScene->Registry;
	//DummySceneMap[SecondDummyScene->ID] = SecondDummyScene;
	//
	//if (&DummyScene->Registry != &SecondDummyScene->Registry)
	//{
	//	int y =0;
	//	y++;
	//	// Do something
	//}

	//if (&DummyScene->Registry == &DummyScene->Registry)
	//{
	//	int y = 0;
	//	y++;
	//	// Do something
	//}

	//if (&SecondDummyScene->Registry == &SecondDummyScene->Registry)
	//{
	//	int y = 0;
	//	y++;
	//	// Do something
	//}

	//auto SceneIterator = DummySceneMap.begin();
	//while (SceneIterator != DummySceneMap.end())
	//{
	//	

	//	SceneIterator++;
	//}
}

FEScene* FESceneManager::GetScene(std::string ID)
{
	if (SceneMap.find(ID) == SceneMap.end())
		return nullptr;

	return SceneMap[ID];
}

FEScene* FESceneManager::AddScene(std::string Name, std::string ForceObjectID)
{
	if (Name.empty())
		Name = "Unnamed Scene";

	/*FEScene* Scene = new FEScene();
	Scene->SetName(Name);

	if (!ForceObjectID.empty())
		Scene->SetID(ForceObjectID);

	SceneMap[Scene->GetObjectID()] = Scene;

	return Scene;*/

	return nullptr;
}

std::vector<std::string> FESceneManager::GetSceneIDList()
{
	FE_MAP_TO_STR_VECTOR(SceneMap)
}

std::vector<FEScene*> FESceneManager::GetSceneByName(const std::string Name)
{
	std::vector<FEScene*> Result;

	/*auto SceneIterator = SceneMap.begin();
	while (SceneIterator != SceneMap.end())
	{
		if (SceneIterator->second->GetName() == Name)
			Result.push_back(SceneIterator->second);

		SceneIterator++;
	}*/

	return Result;
}

void FESceneManager::DeleteScene(std::string ID)
{
	if (SceneMap.find(ID) == SceneMap.end())
		return;

	DeleteScene(SceneMap[ID]);
}

void FESceneManager::DeleteScene(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	//std::string SceneID = Scene->GetObjectID();

	Scene->Clear();
	//delete Scene;

	//SceneMap.erase(SceneID);
}

void FESceneManager::Update()
{
	
}