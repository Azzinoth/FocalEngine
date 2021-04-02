#include "FEScene.h"
using namespace FocalEngine;

FEScene* FEScene::_instance = nullptr;

FEScene::FEScene()
{
}

FELight* FEScene::addLight(FEObjectType lightType, std::string Name, std::string forceObjectID)
{
	if (Name.size() == 0)
		Name = "unnamedLight";

	if (lightType == FE_DIRECTIONAL_LIGHT)
	{
		FEDirectionalLight* newLight = new FEDirectionalLight();
		if (forceObjectID != "")
			newLight->setID(forceObjectID);
		newLight->setName(Name);

		newLight->cascadeData[0].frameBuffer = FEResourceManager::getInstance().createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		newLight->cascadeData[1].frameBuffer = FEResourceManager::getInstance().createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		newLight->cascadeData[2].frameBuffer = FEResourceManager::getInstance().createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		newLight->cascadeData[3].frameBuffer = FEResourceManager::getInstance().createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);

		return newLight;
	}
	else if (lightType == FE_SPOT_LIGHT)
	{
		FESpotLight* newLight = new FESpotLight();
		if (forceObjectID != "")
			newLight->setID(forceObjectID);
		newLight->setName(Name);

		return newLight;
	}
	else if (lightType == FE_POINT_LIGHT)
	{
		FEPointLight* newLight = new FEPointLight();
		if (forceObjectID != "")
			newLight->setID(forceObjectID);
		newLight->setName(Name);

		return newLight;
	}

	return nullptr;
}

FEEntity* FEScene::addEntity(FEGameModel* gameModel, std::string Name, std::string forceObjectID)
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	if (Name.size() == 0)
		Name = "unnamedEntity";

	FEEntity* newEntity = resourceManager.createEntity(gameModel, Name, forceObjectID);
	entityMap[newEntity->getObjectID()] = newEntity;
	return entityMap[newEntity->getObjectID()];
}

bool FEScene::addEntity(FEEntity* newEntity)
{
	if (newEntity == nullptr)
		return false;

	if (newEntity->gameModel == nullptr || newEntity->gameModel->mesh == nullptr || newEntity->gameModel->material == nullptr)
		return false;

	entityMap[newEntity->getObjectID()] = newEntity;

	return true;
}

FEEntity* FEScene::getEntity(std::string ID)
{
	if (entityMap.find(ID) == entityMap.end())
		return nullptr;

	return entityMap[ID];
}

void FEScene::deleteEntity(std::string ID)
{
	if (entityMap.find(ID) == entityMap.end())
		return;

	FEEntity* entityToDelete = entityMap[ID];
	delete entityToDelete;
	entityMap.erase(ID);
}

std::vector<std::string> FEScene::getEntityList()
{
	FE_MAP_TO_STR_VECTOR(entityMap)
}

FELight* FEScene::getLight(std::string ID)
{
	if (FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].find(ID) != FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].end())
		return reinterpret_cast<FEDirectionalLight*>(FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT][ID]);

	if (FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].find(ID) != FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].end())
		return reinterpret_cast<FESpotLight*>(FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT][ID]);

	if (FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].find(ID) != FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].end())
		return reinterpret_cast<FEPointLight*>(FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT][ID]);
	
	return nullptr;
}

std::vector<std::string> FEScene::getLightsList()
{
	std::vector<std::string> result;
	auto iterator = FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (iterator != FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	iterator = FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].begin();
	while (iterator != FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	iterator = FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].begin();
	while (iterator != FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	return result;
}

void FEScene::clear()
{
	// memory leak, need to delete entities not only clearing map.
	entityMap.clear();

	std::vector<FEObject*> allLights;
	auto iterator = FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (iterator != FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		allLights.push_back(iterator->second);
		iterator++;
	}

	iterator = FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].begin();
	while (iterator != FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].end())
	{
		allLights.push_back(iterator->second);
		iterator++;
	}

	iterator = FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].begin();
	while (iterator != FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].end())
	{
		allLights.push_back(iterator->second);
		iterator++;
	}

	for (size_t i = 0; i < allLights.size(); i++)
	{
		if (allLights[i]->getType() == FE_DIRECTIONAL_LIGHT)
		{
			delete reinterpret_cast<FEDirectionalLight*>(allLights[i]);
		}
		else if (allLights[i]->getType() == FE_POINT_LIGHT)
		{
			delete reinterpret_cast<FEPointLight*>(allLights[i]);
		}
		else if (allLights[i]->getType() == FE_SPOT_LIGHT)
		{
			delete reinterpret_cast<FESpotLight*>(allLights[i]);
		}
	}

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

bool FEScene::addTerrain(FETerrain* newTerrain)
{
	if (newTerrain == nullptr)
		return false;

	terrainMap[newTerrain->getObjectID()] = newTerrain;

	return true;
}

std::vector<std::string> FEScene::getTerrainList()
{
	FE_MAP_TO_STR_VECTOR(terrainMap)
}

FETerrain* FEScene::getTerrain(std::string ID)
{
	if (terrainMap.find(ID) == terrainMap.end())
		return nullptr;

	return terrainMap[ID];
}

void FEScene::deleteTerrain(std::string ID)
{
	if (terrainMap.find(ID) == terrainMap.end())
		return;

	FETerrain* terrainToDelete = terrainMap[ID];

	auto entityIt = entityMap.begin();
	while (entityIt != entityMap.end())
	{
		if (entityIt->second->getType() == FE_ENTITY_INSTANCED)
		{
			FEEntityInstanced* instancedEntity = reinterpret_cast<FEEntityInstanced*>(entityIt->second);
			if (instancedEntity->getSnappedToTerrain() == terrainToDelete)
			{
				instancedEntity->terrainToSnap = nullptr;
			}
		}

		entityIt++;
	}
	
	delete terrainToDelete;
	terrainMap.erase(ID);
}

FEEntityInstanced* FEScene::addEntityInstanced(FEGameModel* gameModel, std::string Name, std::string forceObjectID)
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	if (Name.size() == 0)
		Name = "unnamedEntityInstanced";

	FEEntityInstanced* newEntityInstanced = new FEEntityInstanced(gameModel, Name);
	if (forceObjectID != "")
		newEntityInstanced->setID(forceObjectID);

	entityMap[newEntityInstanced->getObjectID()] = newEntityInstanced;
	return newEntityInstanced;
}

bool FEScene::addEntityInstanced(FEEntityInstanced* newEntityInstanced)
{
	if (newEntityInstanced == nullptr)
		return false;

	if (newEntityInstanced->gameModel == nullptr || newEntityInstanced->gameModel->mesh == nullptr || newEntityInstanced->gameModel->material == nullptr)
		return false;

	entityMap[newEntityInstanced->getObjectID()] = newEntityInstanced;

	return true;
}

FEEntityInstanced* FEScene::getEntityInstanced(std::string ID)
{
	if (entityMap.find(ID) == entityMap.end())
		return nullptr;

	if (entityMap[ID]->getType() != FE_ENTITY_INSTANCED)
		return nullptr;

	return reinterpret_cast<FEEntityInstanced*>(entityMap[ID]);
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

void FEScene::deleteLight(std::string ID)
{
	if (FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].find(ID) != FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		FEObjectManager::getInstance().objectsByType[FE_DIRECTIONAL_LIGHT].erase(ID);
		return;
	}
	
	if (FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].find(ID) != FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].end())
	{
		FEObjectManager::getInstance().objectsByType[FE_SPOT_LIGHT].erase(ID);
		return;
	}

	if (FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].find(ID) != FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].end())
	{
		FEObjectManager::getInstance().objectsByType[FE_POINT_LIGHT].erase(ID);
		return;
	}

	return;
}