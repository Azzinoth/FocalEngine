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

		reinterpret_cast<FEDirectionalLight*>(lightsMap[Name])->cascadeData[0].frameBuffer = FEResourceManager::getInstance().createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		reinterpret_cast<FEDirectionalLight*>(lightsMap[Name])->cascadeData[1].frameBuffer = FEResourceManager::getInstance().createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		reinterpret_cast<FEDirectionalLight*>(lightsMap[Name])->cascadeData[2].frameBuffer = FEResourceManager::getInstance().createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		reinterpret_cast<FEDirectionalLight*>(lightsMap[Name])->cascadeData[3].frameBuffer = FEResourceManager::getInstance().createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
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

FEEntity* FEScene::addEntity(FEGameModel* gameModel, std::string Name, std::string forceAssetID)
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

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

	entityMap[Name] = resourceManager.createEntity(gameModel, Name, forceAssetID);
	return entityMap[Name];
}

bool FEScene::addEntity(FEEntity* newEntity)
{
	if (newEntity == nullptr || newEntity->getName().size() == 0 || entityMap.find(newEntity->getName()) != entityMap.end())
		return false;

	if (newEntity->gameModel == nullptr || newEntity->gameModel->mesh == nullptr || newEntity->gameModel->material == nullptr)
		return false;

	entityMap[newEntity->getName()] = newEntity;

	return true;
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
	// memory leak, need to delete entities not only clearing map.
	entityMap.clear();

	auto lightIterator = lightsMap.begin();
	while (lightIterator != lightsMap.end())
	{
		if (lightIterator->second->getType() == FE_DIRECTIONAL_LIGHT)
		{
			delete reinterpret_cast<FEDirectionalLight*>(lightIterator->second);
		}
		else if (lightIterator->second->getType() == FE_POINT_LIGHT)
		{
			delete reinterpret_cast<FEPointLight*>(lightIterator->second);
		}
		else if (lightIterator->second->getType() == FE_SPOT_LIGHT)
		{
			delete reinterpret_cast<FESpotLight*>(lightIterator->second);
		}

		lightIterator++;
	}
	lightsMap.clear();

	auto terrainIterator = terrainMap.begin();
	while (terrainIterator != terrainMap.end())
	{
		delete terrainIterator->second;
		terrainIterator++;
	}
	terrainMap.clear();
}

void FEScene::prepareForGameModelDeletion(FEGameModel* gameModel)
{
	// looking if this gameModel is used in some entities
	// to-do: should be done through list of pointers to entities that uses this gameModel.
	auto entitiesIterator = entityMap.begin();
	while (entitiesIterator != entityMap.end())
	{
		if (entitiesIterator->second->gameModel == gameModel)
		{
			entitiesIterator->second->gameModel = FEResourceManager::getInstance().getGameModel(FEResourceManager::getInstance().getStandardGameModelList()[0]);
		}

		entitiesIterator++;
	}
}

bool FEScene::setEntityName(FEEntity* Entity, std::string EntityName)
{
	if (EntityName.size() == 0 || entityMap.find(EntityName) != entityMap.end())
		return false;

	entityMap.erase(Entity->getName());
	entityMap[EntityName] = Entity;

	Entity->setName(EntityName);
	return true;
}

bool FEScene::addTerrain(FETerrain* newTerrain)
{
	if (newTerrain == nullptr || newTerrain->getName().size() == 0 || terrainMap.find(newTerrain->getName()) != terrainMap.end())
		return false;

	terrainMap[newTerrain->getName()] = newTerrain;

	return true;
}

std::vector<std::string> FEScene::getTerrainList()
{
	FE_MAP_TO_STR_VECTOR(terrainMap)
}

FETerrain* FEScene::getTerrain(std::string name)
{
	if (terrainMap.find(name) == terrainMap.end())
		return nullptr;

	return terrainMap[name];
}

void FEScene::deleteTerrain(std::string name)
{
	if (terrainMap.find(name) == terrainMap.end())
		return;

	FETerrain* terrainToDelete = terrainMap[name];
	delete terrainToDelete;
	terrainMap.erase(name);
}

bool FEScene::setTerrainName(FETerrain* Terrain, std::string TerrainName)
{
	if (TerrainName.size() == 0 || terrainMap.find(TerrainName) != terrainMap.end())
		return false;

	terrainMap.erase(Terrain->getName());
	terrainMap[TerrainName] = Terrain;

	Terrain->setName(TerrainName);
	return true;
}

FEEntityInstanced* FEScene::addEntityInstanced(FEGameModel* gameModel, std::string Name, std::string forceAssetID)
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	if (Name.size() == 0 || entityMap.find(Name) != entityMap.end())
	{
		size_t nextID = entityMap.size();
		size_t index = 0;
		while (entityMap.find(Name) != entityMap.end() || Name.size() == 0)
		{
			index++;
			Name = "entityInstanced_" + std::to_string(nextID + index);
		}
	}

	entityMap[Name] = resourceManager.createEntityInstanced(gameModel, Name, forceAssetID);
	return reinterpret_cast<FEEntityInstanced*>(entityMap[Name]);
}

bool FEScene::addEntityInstanced(FEEntityInstanced* newEntityInstanced)
{
	if (newEntityInstanced == nullptr || newEntityInstanced->getName().size() == 0 || entityMap.find(newEntityInstanced->getName()) != entityMap.end())
		return false;

	if (newEntityInstanced->gameModel == nullptr || newEntityInstanced->gameModel->mesh == nullptr || newEntityInstanced->gameModel->material == nullptr)
		return false;

	entityMap[newEntityInstanced->getName()] = newEntityInstanced;

	return true;
}

FEEntityInstanced* FEScene::getEntityInstanced(std::string name)
{
	if (entityMap.find(name) == entityMap.end())
		return nullptr;

	if (entityMap[name]->getType() != FE_ENTITY_INSTANCED)
		return nullptr;

	return reinterpret_cast<FEEntityInstanced*>(entityMap[name]);
}

void FEScene::setSelectMode(FEEntityInstanced* entityInstanced, bool newValue)
{
	auto it = entityMap.begin();
	while (it != entityMap.end())
	{
		if (it->second->getType() != FE_ENTITY_INSTANCED)
		{
			it++;
			continue;
		}

		reinterpret_cast<FEEntityInstanced*>(it->second)->setSelectMode(false);
		it++;
	}

	entityInstanced->setSelectMode(newValue);
}