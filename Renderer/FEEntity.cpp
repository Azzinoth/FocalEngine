#include "FEEntity.h"
using namespace FocalEngine;

FEEntity::FEEntity() : FEObject(FE_ENTITY, "")
{
}

FEEntity::FEEntity(FEPrefab* Prefab, std::string Name) : FEObject(FE_ENTITY, Name)
{
	this->Prefab = Prefab;
	SetName(Name);
	if (Prefab != nullptr)
		EntityAABB = Prefab->GetAABB();
}

FEEntity::~FEEntity()
{
}

void FEEntity::Render()
{
}

bool FEEntity::IsVisible() const
{
	return bVisible;
}

void FEEntity::SetVisibility(const bool NewValue)
{
	bVisible = NewValue;
}

FEAABB FEEntity::GetAABB()
{
	/*if (Transform.bDirtyFlag)
	{
		EntityAABB = Prefab->GetAABB().Transform(Transform.GetTransformMatrix());
		Transform.bDirtyFlag = false;
	}*/

	//// Temporary solution
	//if (Prefab == nullptr)
	//	return EntityAABB;

	//// Temporary solution
	//EntityAABB = Prefab->GetAABB().Transform(Transform.GetTransformMatrix());

	//return EntityAABB;

	return Prefab->GetAABB();
}

bool FEEntity::IsCastShadows() const
{
	return bCastShadows;
}

void FEEntity::SetCastShadows(const bool NewValue)
{
	bCastShadows = NewValue;
}

bool FEEntity::IsReceivingShadows() const
{
	return bReceiveShadows;
}

void FEEntity::SetReceivingShadows(const bool NewValue)
{
	bReceiveShadows = NewValue;
}

bool FEEntity::IsPostprocessApplied() const
{
	return bApplyPostprocess;
}

void FEEntity::SetIsPostprocessApplied(const bool NewValue)
{
	bApplyPostprocess = NewValue;
}

void FEEntity::SetWireframeMode(const bool NewValue)
{
	bWireframeMode = NewValue;
}

bool FEEntity::IsWireframeMode() const
{
	return bWireframeMode;
}

bool FEEntity::IsUniformLighting() const
{
	return bUniformLighting;
}

void FEEntity::SetUniformLighting(bool NewValue)
{
	bUniformLighting = NewValue;
}