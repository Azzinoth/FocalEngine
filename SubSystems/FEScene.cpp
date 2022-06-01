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

		newLight->cascadeData[0].frameBuffer = RESOURCE_MANAGER.createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		newLight->cascadeData[1].frameBuffer = RESOURCE_MANAGER.createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		newLight->cascadeData[2].frameBuffer = RESOURCE_MANAGER.createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		newLight->cascadeData[3].frameBuffer = RESOURCE_MANAGER.createFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);

		// to clear cascades framebuffer
		newLight->setCastShadows(false);
		newLight->setCastShadows(true);

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
	FEResourceManager& resourceManager = RESOURCE_MANAGER;

	if (Name.size() == 0)
		Name = "unnamedEntity";

	FEEntity* newEntity = resourceManager.createEntity(gameModel, Name, forceObjectID);
	entityMap[newEntity->getObjectID()] = newEntity;
	return entityMap[newEntity->getObjectID()];
}

FEEntity* FEScene::addEntity(FEPrefab* prefab, std::string Name, std::string forceObjectID)
{
	FEResourceManager& resourceManager = RESOURCE_MANAGER;

	if (Name.size() == 0)
		Name = "unnamedEntity";

	FEEntity* newEntity = resourceManager.createEntity(prefab, Name, forceObjectID);
	entityMap[newEntity->getObjectID()] = newEntity;
	return entityMap[newEntity->getObjectID()];
}

bool FEScene::addEntity(FEEntity* newEntity)
{
	if (newEntity == nullptr)
		return false;

	if (newEntity->prefab == nullptr)
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

std::vector<FEEntity*> FEScene::getEntityByName(std::string Name)
{
	std::vector<FEEntity*> result;

	auto it = entityMap.begin();
	while (it != entityMap.end())
	{
		if (it->second->getType() != FE_ENTITY)
		{
			it++;
			continue;
		}

		if (it->second->getName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	return result;
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
	if (OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].find(ID) != OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].end())
		return reinterpret_cast<FEDirectionalLight*>(OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT][ID]);

	if (OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].find(ID) != OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].end())
		return reinterpret_cast<FESpotLight*>(OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT][ID]);

	if (OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].find(ID) != OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].end())
		return reinterpret_cast<FEPointLight*>(OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT][ID]);
	
	return nullptr;
}

std::vector<std::string> FEScene::getLightsList()
{
	std::vector<std::string> result;
	auto iterator = OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	iterator = OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	iterator = OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	return result;
}

void FEScene::clear()
{
	auto entityIterator = entityMap.begin();
	while (entityIterator != entityMap.end())
	{
		delete entityIterator->second;
		entityIterator++;
	}
	entityMap.clear();

	std::vector<FEObject*> allLights;
	auto iterator = OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		allLights.push_back(iterator->second);
		iterator++;
	}

	iterator = OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].end())
	{
		allLights.push_back(iterator->second);
		iterator++;
	}

	iterator = OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].end())
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
	// looking if this gameModel is used in some prefab
	// to-do: should be done through list of pointers to entities that uses this gameModel.
	auto prefabList = RESOURCE_MANAGER.getPrefabList();
	for (size_t i = 0; i < prefabList.size(); i++)
	{
		FEPrefab* currentPrefab = RESOURCE_MANAGER.getPrefab(prefabList[i]);
		for (int j = 0; j < currentPrefab->componentsCount(); j++)
		{
			if (currentPrefab->getComponent(j)->gameModel == gameModel)
			{
				currentPrefab->getComponent(j)->gameModel = RESOURCE_MANAGER.getGameModel(RESOURCE_MANAGER.getStandardGameModelList()[0]);
				currentPrefab->setDirtyFlag(true);
			}
		}
	}
}

void FEScene::prepareForPrefabDeletion(FEPrefab* prefab)
{
	// looking if this prefab is used in some entities
	// to-do: should be done through list of pointers to entities that uses this gameModel.
	auto entitiesIterator = entityMap.begin();
	while (entitiesIterator != entityMap.end())
	{
		if (entitiesIterator->second->prefab == prefab)
		{
			entitiesIterator->second->prefab = RESOURCE_MANAGER.getPrefab(RESOURCE_MANAGER.getStandardPrefabList()[0]);
			entitiesIterator->second->setDirtyFlag(true);
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
				instancedEntity->unSnapFromTerrain();
			}
		}

		entityIt++;
	}
	
	delete terrainToDelete;
	terrainMap.erase(ID);
}

FEEntityInstanced* FEScene::addEntityInstanced(FEPrefab* prefab, std::string Name, std::string forceObjectID)
{
	FEResourceManager& resourceManager = RESOURCE_MANAGER;

	if (Name.size() == 0)
		Name = "unnamedEntityInstanced";

	FEEntityInstanced* newEntityInstanced = new FEEntityInstanced(prefab, Name);
	if (forceObjectID != "")
		newEntityInstanced->setID(forceObjectID);

	entityMap[newEntityInstanced->getObjectID()] = newEntityInstanced;
	return newEntityInstanced;
}

FEEntityInstanced* FEScene::addEntityInstanced(FEGameModel* gameModel, std::string Name, std::string forceObjectID)
{
	FEResourceManager& resourceManager = RESOURCE_MANAGER;
	FEPrefab* tempPrefab = resourceManager.createPrefab(gameModel, gameModel->getName());

	return addEntityInstanced(tempPrefab, Name, forceObjectID);
}

bool FEScene::addEntityInstanced(FEEntityInstanced* newEntityInstanced)
{
	if (newEntityInstanced == nullptr)
		return false;

	if (newEntityInstanced->prefab == nullptr)
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

std::vector<FEEntityInstanced*> FEScene::getEntityInstancedByName(std::string Name)
{
	std::vector<FEEntityInstanced*> result;

	auto it = entityMap.begin();
	while (it != entityMap.end())
	{
		if (it->second->getType() != FE_ENTITY_INSTANCED)
		{
			it++;
			continue;
		}

		if (it->second->getName() == Name)
		{
			result.push_back(reinterpret_cast<FEEntityInstanced*>(it->second));
		}

		it++;
	}

	return result;
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
	if (OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].find(ID) != OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].erase(ID);
		return;
	}
	
	if (OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].find(ID) != OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].end())
	{
		OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].erase(ID);
		return;
	}

	if (OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].find(ID) != OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].end())
	{
		OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].erase(ID);
		return;
	}

	return;
}