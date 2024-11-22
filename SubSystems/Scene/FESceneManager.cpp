#include "FESceneManager.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetSceneManager()
{
	return FESceneManager::GetInstancePointer();
}
#endif

FESceneManager::FESceneManager()
{
}

FEScene* FESceneManager::GetScene(std::string ID)
{
	if (Scenes.find(ID) == Scenes.end())
		return nullptr;

	return Scenes[ID];
}

FEScene* FESceneManager::CreateScene(std::string Name, std::string ForceObjectID, FESceneFlag Flags)
{
	FEScene* Scene = new FEScene();
	Scene->Flags = Flags;

	if (!Name.empty())
		Scene->SetName(Name);

	if (!ForceObjectID.empty())
		Scene->SetID(ForceObjectID);

	Scenes[Scene->GetObjectID()] = Scene;
	if (Scene->HasFlag(FESceneFlag::Active))
		RegisterAllComponentCallbacks(Scene);
	
	return Scene;
}

Json::Value FESceneManager::SaveSceneToJSON(FEScene* Scene, std::function<bool(FEEntity*)> Filter)
{
	if (Scene == nullptr)
	{
		LOG.Add("FESceneManager::SaveSceneToJSON: Scene is nullptr.", "FE_LOG_ECS", FE_LOG_ERROR);
		return Json::Value();
	}

	Json::Value Root;

	Root["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(Scene);
	Json::Value SceneHierarchy = Scene->SceneGraph.ToJson(Filter);
	Root["Scene hierarchy"] = SceneHierarchy;

	return Root;
}

FEScene* FESceneManager::LoadSceneFromJSON(Json::Value& Root, FESceneFlag Flags)
{
	FEObjectLoadedData LoadedObjectData = RESOURCE_MANAGER.LoadFEObjectPart(Root["FEObjectData"]);

	FEScene* NewScene = SCENE_MANAGER.CreateScene(LoadedObjectData.Name, LoadedObjectData.ID, Flags);
	RESOURCE_MANAGER.SetTag(NewScene, LoadedObjectData.Tag);

	NewScene->SceneGraph.FromJson(Root["Scene hierarchy"]);
	return NewScene;
}

void FESceneManager::RegisterAllComponentCallbacks(FEScene* Scene)
{
	Scene->Registry.on_construct<FETagComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FETagComponent>>();
	Scene->Registry.on_construct<FETransformComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FETransformComponent>>();
	Scene->Registry.on_construct<FECameraComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FECameraComponent>>();
	Scene->Registry.on_construct<FELightComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FELightComponent>>();
	Scene->Registry.on_construct<FEGameModelComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FEGameModelComponent>>();
	Scene->Registry.on_construct<FEInstancedComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FEInstancedComponent>>();
	Scene->Registry.on_construct<FETerrainComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FETerrainComponent>>();
	Scene->Registry.on_construct<FESkyDomeComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FESkyDomeComponent>>();
	Scene->Registry.on_construct<FEPrefabInstanceComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FEPrefabInstanceComponent>>();
	Scene->Registry.on_construct<FEVirtualUIComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FEVirtualUIComponent>>();
	Scene->Registry.on_construct<FENativeScriptComponent>().connect<&FESceneManager::OnComponentConstructWrapper<FENativeScriptComponent>>();

	Scene->Registry.on_destroy<FETagComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FETagComponent>>();
	Scene->Registry.on_destroy<FETransformComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FETransformComponent>>();
	Scene->Registry.on_destroy<FECameraComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FECameraComponent>>();
	Scene->Registry.on_destroy<FELightComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FELightComponent>>();
	Scene->Registry.on_destroy<FEGameModelComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FEGameModelComponent>>();
	Scene->Registry.on_destroy<FEInstancedComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FEInstancedComponent>>();
	Scene->Registry.on_destroy<FETerrainComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FETerrainComponent>>();
	Scene->Registry.on_destroy<FESkyDomeComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FESkyDomeComponent>>();
	Scene->Registry.on_destroy<FEPrefabInstanceComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FEPrefabInstanceComponent>>();
	Scene->Registry.on_destroy<FEVirtualUIComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FEVirtualUIComponent>>();
	Scene->Registry.on_destroy<FENativeScriptComponent>().connect<&FESceneManager::OnComponentDestroyWrapper<FENativeScriptComponent>>();

	Scene->Registry.on_update<FETagComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FETagComponent>>();
	Scene->Registry.on_update<FETransformComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FETransformComponent>>();
	Scene->Registry.on_update<FECameraComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FECameraComponent>>();
	Scene->Registry.on_update<FELightComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FELightComponent>>();
	Scene->Registry.on_update<FEGameModelComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FEGameModelComponent>>();
	Scene->Registry.on_update<FEInstancedComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FEInstancedComponent>>();
	Scene->Registry.on_update<FETerrainComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FETerrainComponent>>();
	Scene->Registry.on_update<FESkyDomeComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FESkyDomeComponent>>();
	Scene->Registry.on_update<FEPrefabInstanceComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FEPrefabInstanceComponent>>();
	Scene->Registry.on_update<FEVirtualUIComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FEVirtualUIComponent>>();
	Scene->Registry.on_update<FENativeScriptComponent>().connect<&FESceneManager::OnComponentUpdateWrapper<FENativeScriptComponent>>();
}

void FESceneManager::UnRegisterAllComponentCallbacks(FEScene* Scene)
{
	Scene->Registry.on_construct<FETagComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FETagComponent>>();
	Scene->Registry.on_construct<FETransformComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FETransformComponent>>();
	Scene->Registry.on_construct<FECameraComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FECameraComponent>>();
	Scene->Registry.on_construct<FELightComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FELightComponent>>();
	Scene->Registry.on_construct<FEGameModelComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FEGameModelComponent>>();
	Scene->Registry.on_construct<FEInstancedComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FEInstancedComponent>>();
	Scene->Registry.on_construct<FETerrainComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FETerrainComponent>>();
	Scene->Registry.on_construct<FESkyDomeComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FESkyDomeComponent>>();
	Scene->Registry.on_construct<FEPrefabInstanceComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FEPrefabInstanceComponent>>();
	Scene->Registry.on_construct<FEVirtualUIComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FEVirtualUIComponent>>();
	Scene->Registry.on_construct<FENativeScriptComponent>().disconnect<&FESceneManager::OnComponentConstructWrapper<FENativeScriptComponent>>();

	Scene->Registry.on_destroy<FETagComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FETagComponent>>();
	Scene->Registry.on_destroy<FETransformComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FETransformComponent>>();
	Scene->Registry.on_destroy<FECameraComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FECameraComponent>>();
	Scene->Registry.on_destroy<FELightComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FELightComponent>>();
	Scene->Registry.on_destroy<FEGameModelComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FEGameModelComponent>>();
	Scene->Registry.on_destroy<FEInstancedComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FEInstancedComponent>>();
	Scene->Registry.on_destroy<FETerrainComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FETerrainComponent>>();
	Scene->Registry.on_destroy<FESkyDomeComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FESkyDomeComponent>>();
	Scene->Registry.on_destroy<FEPrefabInstanceComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FEPrefabInstanceComponent>>();
	Scene->Registry.on_destroy<FEVirtualUIComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FEVirtualUIComponent>>();
	Scene->Registry.on_destroy<FENativeScriptComponent>().disconnect<&FESceneManager::OnComponentDestroyWrapper<FENativeScriptComponent>>();

	Scene->Registry.on_update<FETagComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FETagComponent>>();
	Scene->Registry.on_update<FETransformComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FETransformComponent>>();
	Scene->Registry.on_update<FECameraComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FECameraComponent>>();
	Scene->Registry.on_update<FELightComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FELightComponent>>();
	Scene->Registry.on_update<FEGameModelComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FEGameModelComponent>>();
	Scene->Registry.on_update<FEInstancedComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FEInstancedComponent>>();
	Scene->Registry.on_update<FETerrainComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FETerrainComponent>>();
	Scene->Registry.on_update<FESkyDomeComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FESkyDomeComponent>>();
	Scene->Registry.on_update<FEPrefabInstanceComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FEPrefabInstanceComponent>>();
	Scene->Registry.on_update<FEVirtualUIComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FEVirtualUIComponent>>();
	Scene->Registry.on_update<FENativeScriptComponent>().disconnect<&FESceneManager::OnComponentUpdateWrapper<FENativeScriptComponent>>();
}

std::vector<std::string> FESceneManager::GetSceneIDList()
{
	FE_MAP_TO_STR_VECTOR(Scenes)
}

std::vector<FEScene*> FESceneManager::GetSceneByName(const std::string Name)
{
	std::vector<FEScene*> Result;

	auto SceneIterator = Scenes.begin();
	while (SceneIterator != Scenes.end())
	{
		if (SceneIterator->second->GetName() == Name)
			Result.push_back(SceneIterator->second);

		SceneIterator++;
	}

	return Result;
}

void FESceneManager::DeleteScene(std::string ID)
{
	if (Scenes.find(ID) == Scenes.end())
		return;

	DeleteScene(Scenes[ID]);
}

void FESceneManager::DeleteScene(FEScene* Scene)
{
	if (Scene == nullptr)
		return;

	std::string SceneID = Scene->GetObjectID();
	delete Scene;

	Scenes.erase(SceneID);
}

void FESceneManager::Update()
{
	std::vector<FEScene*> ActiveScenes = GetScenesByFlagMask(FESceneFlag::Active);
	for (size_t i = 0; i < ActiveScenes.size(); i++)
	{
		ActiveScenes[i]->Update();
	}
}

std::vector<FEScene*> FESceneManager::GetAllScenes()
{
	std::vector<FEScene*> Result;

	auto SceneIterator = Scenes.begin();
	while (SceneIterator != Scenes.end())
	{
		Result.push_back(SceneIterator->second);
		SceneIterator++;
	}

	return Result;
}

std::vector<FEScene*> FESceneManager::GetScenesByFlagMask(FESceneFlag FlagMask)
{
	std::vector<FEScene*> Result;

	auto SceneIterator = Scenes.begin();
	while (SceneIterator != Scenes.end())
	{
		if ((SceneIterator->second->Flags & FlagMask) == FlagMask)
			Result.push_back(SceneIterator->second);

		SceneIterator++;
	}

	return Result;
}

FEScene* FESceneManager::DuplicateScene(std::string ID, std::string NewSceneName, std::function<bool(FEEntity*)> Filter, FESceneFlag Flags)
{
	FEScene* SceneToDuplicate = GetScene(ID);
	if (SceneToDuplicate == nullptr)
		return nullptr;

	return DuplicateScene(SceneToDuplicate, NewSceneName, Filter, Flags);
}

FEScene* FESceneManager::DuplicateScene(FEScene* SourceScene, std::string NewSceneName, std::function<bool(FEEntity*)> Filter, FESceneFlag Flags)
{
	FEScene* Result = CreateScene(NewSceneName, "", Flags);

	// Get children of the root entity and import them.
	std::vector<FENaiveSceneGraphNode*> RootChildrens = SourceScene->SceneGraph.GetRoot()->GetChildren();
	for (auto RootChildren : RootChildrens)
	{
		FEEntity* EntityToDuplicate = RootChildren->GetEntity();
		Result->ImportEntity(EntityToDuplicate, nullptr, Filter);
	}

	return Result;
}

// TO-DO: Look into makeing FETransformSystem more robust, with less speciale cases.
#include "Components/Systems/FETransformSystem.h"
std::vector<FENaiveSceneGraphNode*> FESceneManager::ImportSceneAsNode(FEScene* SourceScene, FEScene* TargetScene, FENaiveSceneGraphNode* TargetParent, std::function<bool(FEEntity*)> Filter)
{
	std::vector<FENaiveSceneGraphNode*> Result;
	if (SourceScene == nullptr || TargetScene == nullptr)
	{
		LOG.Add("FESceneManager::ImportSceneAsNode: SourceScene or TargetScene is nullptr.", "FE_LOG_ECS", FE_LOG_ERROR);
		return Result;
	}

	// We are doing this to make sure that all entities have updated world matrices.
	// It is especially important for FEPrefab scenes, because they are not updated by default.
	TRANSFORM_SYSTEM.UpdateInternal(SourceScene->SceneGraph.GetRoot());

	// Get children of the root entity and import them.
	std::vector<FENaiveSceneGraphNode*> RootChildrens = SourceScene->SceneGraph.GetRoot()->GetChildren();
	for (auto RootChildren : RootChildrens)
	{
		FEEntity* EntityToDuplicate = RootChildren->GetEntity();
		FEEntity* NewChildEntity = TargetScene->ImportEntity(EntityToDuplicate, TargetParent, Filter);
		if (NewChildEntity != nullptr)
		{
			FENaiveSceneGraphNode* NewChildNode = TargetScene->SceneGraph.GetNodeByEntityID(NewChildEntity->GetObjectID());
			if (NewChildNode != nullptr)
			{
				Result.push_back(NewChildNode);
			}	
			else
			{
				LOG.Add("FESceneManager::ImportSceneAsNode: Failed to get new node.", "FE_LOG_ECS", FE_LOG_ERROR);
			}
		}
	}

	return Result;
}

bool FESceneManager::AreSceneGraphHierarchiesEquivalent(FENaiveSceneGraphNode* FirstStaringNode, FENaiveSceneGraphNode* SecondStartingNode, bool bCheckNames)
{
	if (FirstStaringNode == nullptr || SecondStartingNode == nullptr)
		return FirstStaringNode == SecondStartingNode; // Both null is considered equivalent

	if (FirstStaringNode->GetChildren().size() != SecondStartingNode->GetChildren().size())
		return false;

	if (bCheckNames && FirstStaringNode->GetName() != SecondStartingNode->GetName())
		return false;

	for (size_t i = 0; i < FirstStaringNode->GetChildren().size(); i++)
	{
		if (!AreSceneGraphHierarchiesEquivalent(FirstStaringNode->GetChildren()[i], SecondStartingNode->GetChildren()[i], bCheckNames))
			return false;
	}

	return true;
}

std::vector<FEEntity*> FESceneManager::InstantiatePrefab(FEPrefab* Prefab, FEScene* Scene, bool bAddToSceneRoot)
{
	std::vector<FEEntity*> Result;

	if (Prefab == nullptr || Scene == nullptr)
	{
		LOG.Add("FESceneManager::InstantiatePrefab: Prefab or Scene is nullptr.", "FE_LOG_ECS", FE_LOG_ERROR);
		return Result;
	}

	FEScene* PrefabScene = Prefab->GetScene();
	if (PrefabScene == nullptr)
	{
		LOG.Add("FESceneManager::InstantiatePrefab: Prefab scene is nullptr.", "FE_LOG_ECS", FE_LOG_ERROR);
		return Result;
	}

	FENaiveSceneGraphNode* RootNode = PrefabScene->SceneGraph.GetRoot();
	if (RootNode == nullptr)
	{
		LOG.Add("FESceneManager::InstantiatePrefab: Prefab scene root node is nullptr.", "FE_LOG_ECS", FE_LOG_ERROR);
		return Result;
	}

	if (bAddToSceneRoot)
	{
		// Import prefab scene graph nodes to the scene root.
		std::vector<FENaiveSceneGraphNode*> PrefabNodes = ImportSceneAsNode(PrefabScene, Scene, Scene->SceneGraph.GetRoot());
		if (!PrefabNodes.empty())
		{
			FEEntity* SceneRootEntity = Scene->SceneGraph.GetRoot()->GetEntity();
			for (size_t i = 0; i < PrefabNodes.size(); i++)
			{
				Result.push_back(PrefabNodes[i]->GetEntity());
			}

			return Result;
		}
		else
		{
			LOG.Add("FESceneManager::InstantiatePrefab: Failed to import prefab scene graph nodes.", "FE_LOG_ECS", FE_LOG_ERROR);
		}
	}
	else
	{
		// Create a new entity to hold the prefab instance and its scene graph nodes
		FEEntity* NewEntity = Scene->CreateEntity(Prefab->GetName());
		FENaiveSceneGraphNode* NewNode = Scene->SceneGraph.GetNodeByEntityID(NewEntity->GetObjectID());

		std::vector<FENaiveSceneGraphNode*> PrefabNodes = ImportSceneAsNode(PrefabScene, Scene, NewNode);
		if (!PrefabNodes.empty())
		{
			NewEntity->AddComponent<FEPrefabInstanceComponent>(Prefab);
			Result.push_back(NewEntity);
			for (size_t i = 0; i < PrefabNodes.size(); i++)
			{
				Result.push_back(PrefabNodes[i]->GetEntity());
			}
			
			return Result;
		}
		else
		{
			LOG.Add("FESceneManager::InstantiatePrefab: Failed to import prefab scene graph nodes.", "FE_LOG_ECS", FE_LOG_ERROR);
			Scene->DeleteEntity(NewEntity);
		}
	}

	return Result;
}

void FESceneManager::Clear()
{
	std::vector<std::string> Tags = RESOURCE_MANAGER.GetTagsThatWillPreventDeletion();
	std::vector<FEScene*> ScenesToDelete;

	auto SceneIterator = Scenes.begin();
	while (SceneIterator != Scenes.end())
	{
		bool bCanDelete = true;
		for (size_t i = 0; i < Tags.size(); i++)
		{
			if (Tags[i] == SceneIterator->second->GetTag())
			{
				bCanDelete = false;
				break;
			}
		}

		if (SceneIterator->second->GetTag() == PREFAB_SCENE_DESCRIPTION_TAG)
		{
			std::vector<std::string> PrefabIDList = RESOURCE_MANAGER.GetPrefabIDList();
			for (size_t i = 0; i < PrefabIDList.size(); i++)
			{
				FEPrefab* Prefab = RESOURCE_MANAGER.GetPrefab(PrefabIDList[i]);
				if (Prefab == nullptr)
					continue;

				if (Prefab->GetScene() != SceneIterator->second)
					continue;

				for (size_t j = 0; j < Tags.size(); j++)
				{
					if (Tags[j] == Prefab->GetTag())
					{
						bCanDelete = false;
						break;
					}
				}

				if (!bCanDelete)
					break;
			}
		}

		if (bCanDelete)
			ScenesToDelete.push_back(SceneIterator->second);

		SceneIterator++;
	}

	for (size_t i = 0; i < ScenesToDelete.size(); i++)
	{
		DeleteScene(ScenesToDelete[i]);
	}
}

FEScene* FESceneManager::GetStartingScene()
{
	if (StartingSceneID.empty() || GetScene(StartingSceneID) == nullptr)
	{
		std::vector<std::string> TagsToAvoid = RESOURCE_MANAGER.GetTagsThatWillPreventDeletion();
		TagsToAvoid.push_back(PREFAB_SCENE_DESCRIPTION_TAG);

		std::vector<FEScene*> AllScenes = GetAllScenes();
		for (size_t i = 0; i < AllScenes.size(); i++)
		{
			bool bCanUse = true;
			for (size_t j = 0; j < TagsToAvoid.size(); j++)
			{
				if (AllScenes[i]->GetTag() == TagsToAvoid[j])
				{
					bCanUse = false;
					break;
				}
			}

			if (bCanUse)
				return AllScenes[i];
		}

		return nullptr;
	}

	return GetScene(StartingSceneID);
}

bool FESceneManager::SetStartingScene(std::string SceneID)
{
	if (GetScene(StartingSceneID) == nullptr)
	{
		LOG.Add("FESceneManager::SetStartingScene: Scene with ID " + SceneID + " does not exist.", "FE_LOG_ECS", FE_LOG_ERROR);
		return false;
	}

	StartingSceneID = SceneID;
	return true;
}