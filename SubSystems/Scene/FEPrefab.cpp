#include "FEPrefab.h"
#include "../SubSystems/Scene/FESceneManager.h"
#include "../ResourceManager/FEResourceManager.h"
using namespace FocalEngine;

FEPrefab::FEPrefab(const std::string Name, bool bSceneAllocation) : FEObject(FE_PREFAB, Name)
{
	SetName(Name);
	if (bSceneAllocation)
	{
		Scene = SCENE_MANAGER.CreateScene(GetName() + "_Scene", "", FESceneFlag::PrefabDescription);
		RESOURCE_MANAGER.SetTag(Scene, PREFAB_SCENE_DESCRIPTION_TAG);
	}
	
	SetDirtyFlag(true);
}

FEPrefab::~FEPrefab()
{
	SCENE_MANAGER.DeleteScene(Scene);
}

FEAABB FEPrefab::GetAABB()
{
	return Scene->GetSceneAABB();
}

bool FEPrefab::IsUsingMaterial(const std::string MaterialID) const
{
	if (Scene == nullptr)
		return false;
	
	std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FEGameModelComponent>();
	for (int i = 0; i < Entities.size(); i++)
	{
		FEGameModelComponent& GameModelComponent = Entities[i]->GetComponent<FEGameModelComponent>();
		if (GameModelComponent.GetGameModel()->Material->GetObjectID() == MaterialID)
			return true;

		if (GameModelComponent.GetGameModel()->BillboardMaterial != nullptr && GameModelComponent.GetGameModel()->BillboardMaterial->GetObjectID() == MaterialID)
			return true;
	}

	return false;
}

bool FEPrefab::IsUsingGameModel(const std::string GameModelID) const
{
	if (Scene == nullptr)
		return false;

	std::vector<FEEntity*> Entities = Scene->GetEntityListWithComponent<FEGameModelComponent>();
	for (int i = 0; i < Entities.size(); i++)
	{
		FEGameModelComponent& GameModelComponent = Entities[i]->GetComponent<FEGameModelComponent>();
		if (GameModelComponent.GetGameModel()->GetObjectID() == GameModelID)
			return true;
	}

	return false;
}

FEScene* FEPrefab::GetScene() const
{
	return Scene;
}

void FEPrefab::SetScene(FEScene* Scene, bool DeleteOldScene)
{
	if (DeleteOldScene)
		SCENE_MANAGER.DeleteScene(this->Scene);

	this->Scene = Scene;
}