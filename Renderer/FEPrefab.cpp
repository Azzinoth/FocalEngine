#include "FEPrefab.h"
using namespace FocalEngine;

FEPrefab::FEPrefab() : FEObject(FE_PREFAB, "")
{
	SetDirtyFlag(true);
}

FEPrefab::FEPrefab(FEGameModel* GameModel, const std::string Name) : FEObject(FE_PREFAB, Name)
{
	Components.push_back(new FEPrefabComponent());
	Components.back()->GameModel = GameModel;

	SetDirtyFlag(true);
}

FEPrefab::~FEPrefab()
{
	for (int i = 0; i < Components.size(); i++)
	{
		delete Components[i];
	}
}

void FEPrefab::UpdateAABB()
{
	AABB = FEAABB();

	for (int i = 0; i < Components.size(); i++)
	{
		if (Components[i]->GameModel == nullptr || Components[i]->GameModel->Mesh == nullptr)
			continue;
		AABB = AABB.Merge(Components[i]->GameModel->Mesh->GetAABB().Transform(Components[i]->Transform.GetTransformMatrix()));
	}
}

FEAABB FEPrefab::GetAABB()
{
	if (IsDirty())
		UpdateAABB();

	return AABB;
}

bool FEPrefab::UsesMaterial(const std::string MaterialID) const
{
	for (int i = 0; i < Components.size(); i++)
	{
		if (Components[i]->GameModel->Material->GetObjectID() == MaterialID)
			return true;

		if (Components[i]->GameModel->BillboardMaterial != nullptr && Components[i]->GameModel->BillboardMaterial->GetObjectID() == MaterialID)
			return true;
	}

	return false;
}

bool FEPrefab::UsesGameModel(const std::string GameModelID) const
{
	for (int i = 0; i < Components.size(); i++)
	{
		if (Components[i]->GameModel->GetObjectID() == GameModelID)
			return true;
	}

	return false;
}

int FEPrefab::ComponentsCount() const
{
	return static_cast<int>(Components.size());
}

void FEPrefab::AddComponent(FEGameModel* GameModel, const FETransformComponent Transform)
{
	if (GameModel == nullptr)
		return;

	Components.push_back(new FEPrefabComponent());
	Components.back()->GameModel = GameModel;
	Components.back()->Transform = Transform;

	SetDirtyFlag(true);
}

FEPrefabComponent* FEPrefab::GetComponent(const int Index) const
{
	if (Index >= Components.size())
		return nullptr;

	return Components[Index];
}

void FEPrefab::RemoveComponent(const FEGameModel* GameModel)
{
	for (int i = 0; i < Components.size(); i++)
	{
		if (Components[i]->GameModel == GameModel)
		{
			RemoveComponent(i);
			i--;
		}
	}
}

void FEPrefab::RemoveComponent(const int Index)
{
	if (Index >= Components.size())
		return;

	delete Components[Index];
	Components.erase(Components.begin() + Index);

	SetDirtyFlag(true);
}