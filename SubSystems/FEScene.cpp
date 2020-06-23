#include "FEScene.h"
using namespace FocalEngine;

FEScene* FEScene::_instance = nullptr;

FEScene::FEScene()
{
}

void FEScene::addLight(FELightType Type, std::string Name)
{
	short nextID = short(lightsMap.size());
	
	if (Name.size())
	{
		// if there is light with that name already
		if (lightsMap.find(Name) != lightsMap.end())
			Name = "light_" + std::to_string(nextID);
	}
	else
	{
		Name = "light_" + std::to_string(nextID);
	}

	if (Type == FE_DIRECTIONAL_LIGHT)
	{
		lightsMap[Name] = new FEDirectionalLight();
		lightsMap[Name]->setName(Name);

		reinterpret_cast<FEDirectionalLight*>(lightsMap[Name])->cascadeData[0].frameBuffer = new FEFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		reinterpret_cast<FEDirectionalLight*>(lightsMap[Name])->cascadeData[1].frameBuffer = new FEFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		reinterpret_cast<FEDirectionalLight*>(lightsMap[Name])->cascadeData[2].frameBuffer = new FEFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		reinterpret_cast<FEDirectionalLight*>(lightsMap[Name])->cascadeData[3].frameBuffer = new FEFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
	}
	else if (Type == FE_SPOT_LIGHT)
	{
		lightsMap[Name] = new FESpotLight();
		lightsMap[Name]->setName(Name);
	}
	else if (Type == FE_POINT_LIGHT)
	{
		lightsMap[Name] = new FEPointLight();
		lightsMap[Name]->setName(Name);
	}
}

FEEntity* FEScene::addEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name)
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	if (!Material)
		Material = resourceManager.getMaterial("SolidColorMaterial");
	
	if (Name.size() == 0 || entityMap.find(Name) != entityMap.end())
	{
		size_t nextID = entityMap.size();
		size_t index = 0;
		while (entityMap.find(Name) != entityMap.end() || Name.size() == 0)
		{
			index++;
			Name = "entity_" + std::to_string(nextID + index);
		}
	}

	entityMap[Name] = resourceManager.createEntity(Mesh, Material, Name);
	return entityMap[Name];
}

FEEntity* FEScene::getEntity(std::string name)
{
	if (entityMap.find(name) == entityMap.end())
		return nullptr;

	return entityMap[name];
}

void FEScene::deleteEntity(std::string name)
{
	if (entityMap.find(name) == entityMap.end())
		return;

	FEEntity* entityToDelete = entityMap[name];
	delete entityToDelete;
	entityMap.erase(name);
}

std::vector<std::string> FEScene::getEntityList()
{
	FE_MAP_TO_STR_VECTOR(entityMap)
}

FELight* FEScene::getLight(std::string name)
{
	if (lightsMap.find(name) == lightsMap.end())
		return nullptr;

	return lightsMap[name];
}

std::vector<std::string> FEScene::getLightsList()
{
	FE_MAP_TO_STR_VECTOR(lightsMap)
}

void FEScene::clear()
{
	entityMap.clear();
	lightsMap.clear();
}

void FEScene::prepareForFEMeshDeletion(FEMesh* mesh)
{
	// looking if this mesh is used in some entities
	// to-do: should be done through list of pointers to entities that uses this mesh.
	auto entitiesIterator = entityMap.begin();
	while (entitiesIterator != entityMap.end())
	{
		if (entitiesIterator->second->mesh == mesh)
		{
			entitiesIterator->second->mesh = FEResourceManager::getInstance().getMesh(FEResourceManager::getInstance().getStandardMeshList()[0]);
		}

		entitiesIterator++;
	}
}