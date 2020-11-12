#pragma once

#include "FEDearImguiWrapper/FEDearImguiWrapper.h"

class FEEditorInternalResources
{
public:
	SINGLETON_PUBLIC_PART(FEEditorInternalResources)
	SINGLETON_PRIVATE_PART(FEEditorInternalResources)

	std::unordered_map<int, FEMesh*> internalEditorMesh;
	std::unordered_map<int, FEGameModel*> internalEditorGameModels;
	std::unordered_map<int, FEEntity*> internalEditorEntities;

	template <class RESOURCE>
	void addResourceToInternalEditorList(RESOURCE* resource);

	bool isInInternalEditorList(FEMesh* mesh);
	bool isInInternalEditorList(FEGameModel* gameModel);
	bool isInInternalEditorList(FEEntity* entity);
};

template <class RESOURCE>
void FEEditorInternalResources::addResourceToInternalEditorList(RESOURCE* resource)
{
	if (resource->getType() == FE_ENTITY)
	{
		internalEditorEntities[resource->getNameHash()] = reinterpret_cast<FEEntity*>(resource);
	}
	else if (resource->getType() == FE_GAMEMODEL)
	{
		internalEditorGameModels[resource->getNameHash()] = reinterpret_cast<FEGameModel*>(resource);;
	}
	else if (resource->getType() == FE_MESH)
	{
		internalEditorMesh[resource->getNameHash()] = reinterpret_cast<FEMesh*>(resource);;
	}
}

#define EDITOR_INTERNAL_RESOURCES FEEditorInternalResources::getInstance()