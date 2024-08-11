#include "FEPrefab.h"
#include "../SubSystems/Scene/FESceneManager.h"
using namespace FocalEngine;

FEPrefab::FEPrefab(const std::string Name) : FEObject(FE_PREFAB, Name)
{
	SetName(Name);
	Scene = SCENE_MANAGER.CreateScene(GetName(), "", false);
	SetDirtyFlag(true);
}

FEPrefab::~FEPrefab()
{
	// FIX ME! Should we delete scene here?
}

FEAABB FEPrefab::GetAABB()
{
	return Scene->GetSceneAABB();
}

bool FEPrefab::IsUsingMaterial(const std::string MaterialID) const
{
	if (Scene == nullptr)
		return false;
	
	std::vector<FEEntity*> Entities = Scene->GetEntityListWith<FEGameModelComponent>();
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

	std::vector<FEEntity*> Entities = Scene->GetEntityListWith<FEGameModelComponent>();
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
