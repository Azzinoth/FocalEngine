#include "FEScene.h"
using namespace FocalEngine;

FEScene* FEScene::_instance = nullptr;

FEScene::FEScene()
{
}

void FEScene::add(FELight* newLight)
{
	sceneLights.push_back(newLight);
}

void FEScene::addEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name)
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	if (!Material)
		Material = resourceManager.materials["SolidColorMaterial"];

	short nextID = short(entityMap.size());
	if (Name.size())
	{
		// if there is entity with that name already
		if (entityMap.find(Name) != entityMap.end())
			Name = "entity_" + std::to_string(nextID);
	}
	else
	{
		Name = "entity_" + std::to_string(nextID);
	}

	entityMap[Name] = resourceManager.createEntity(Mesh, Material, Name);
}

FEEntity* FEScene::getEntity(std::string name)
{
	if (entityMap.find(name) == entityMap.end())
		return nullptr;

	return entityMap[name];
}

std::vector<std::string> FEScene::getEntityList()
{
	FE_MAP_TO_STR_VECTOR(entityMap)
}