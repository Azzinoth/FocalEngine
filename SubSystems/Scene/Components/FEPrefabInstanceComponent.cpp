#include "FEPrefabInstanceComponent.h"
using namespace FocalEngine;

FEPrefabInstanceComponent::FEPrefabInstanceComponent(FEPrefab* PrefabToSet)
{
	Prefab = PrefabToSet;
}

FEPrefabInstanceComponent::FEPrefabInstanceComponent(const FEPrefabInstanceComponent& Other)
{
	Prefab = Other.Prefab;
}

void FEPrefabInstanceComponent::operator=(const FEPrefabInstanceComponent& Other)
{
	Prefab = Other.Prefab;
}

FEPrefab* FEPrefabInstanceComponent::GetPrefab()
{
	return Prefab;
}