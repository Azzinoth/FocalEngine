#include "FEScene.h"
using namespace FocalEngine;

FEScene* FEScene::Instance = nullptr;

FEScene::FEScene()
{
}

FELight* FEScene::AddLight(const FE_OBJECT_TYPE LightType, std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedLight";

	if (LightType == FE_DIRECTIONAL_LIGHT)
	{
		FEDirectionalLight* NewLight = new FEDirectionalLight();
		if (!ForceObjectID.empty())
			NewLight->SetID(ForceObjectID);
		NewLight->SetName(Name);

		NewLight->CascadeData[0].FrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		NewLight->CascadeData[1].FrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		NewLight->CascadeData[2].FrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		NewLight->CascadeData[3].FrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);

		// to clear cascades framebuffer
		NewLight->SetCastShadows(false);
		NewLight->SetCastShadows(true);

		return NewLight;
	}
	else if (LightType == FE_SPOT_LIGHT)
	{
		FESpotLight* NewLight = new FESpotLight();
		if (!ForceObjectID.empty())
			NewLight->SetID(ForceObjectID);
		NewLight->SetName(Name);

		return NewLight;
	}
	else if (LightType == FE_POINT_LIGHT)
	{
		FEPointLight* NewLight = new FEPointLight();
		if (!ForceObjectID.empty())
			NewLight->SetID(ForceObjectID);
		NewLight->SetName(Name);

		return NewLight;
	}

	return nullptr;
}

FEEntity* FEScene::AddEntity(FEGameModel* GameModel, std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedEntity";

	FEEntity* NewEntity = RESOURCE_MANAGER.CreateEntity(GameModel, Name, ForceObjectID);
	EntityMap[NewEntity->GetObjectID()] = NewEntity;
	return EntityMap[NewEntity->GetObjectID()];
}

FEEntity* FEScene::AddEntity(FEPrefab* Prefab, std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedEntity";

	FEEntity* NewEntity = RESOURCE_MANAGER.CreateEntity(Prefab, Name, ForceObjectID);
	EntityMap[NewEntity->GetObjectID()] = NewEntity;
	return EntityMap[NewEntity->GetObjectID()];
}

bool FEScene::AddEntity(FEEntity* NewEntity)
{
	if (NewEntity == nullptr)
		return false;

	if (NewEntity->Prefab == nullptr)
		return false;

	EntityMap[NewEntity->GetObjectID()] = NewEntity;

	return true;
}

FEEntity* FEScene::GetEntity(const std::string ID)
{
	if (EntityMap.find(ID) == EntityMap.end())
		return nullptr;

	return EntityMap[ID];
}

std::vector<FEEntity*> FEScene::GetEntityByName(const std::string Name)
{
	std::vector<FEEntity*> result;

	auto it = EntityMap.begin();
	while (it != EntityMap.end())
	{
		if (it->second->GetType() != FE_ENTITY)
		{
			it++;
			continue;
		}

		if (it->second->GetName() == Name)
		{
			result.push_back(it->second);
		}

		it++;
	}

	return result;
}

void FEScene::DeleteEntity(const std::string ID)
{
	if (EntityMap.find(ID) == EntityMap.end())
		return;

	const FEEntity* EntityToDelete = EntityMap[ID];
	delete EntityToDelete;
	EntityMap.erase(ID);
}

std::vector<std::string> FEScene::GetEntityList()
{
	FE_MAP_TO_STR_VECTOR(EntityMap)
}

FELight* FEScene::GetLight(const std::string ID)
{
	if (OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].find(ID) != OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].end())
		return reinterpret_cast<FEDirectionalLight*>(OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT][ID]);

	if (OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].find(ID) != OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].end())
		return reinterpret_cast<FESpotLight*>(OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT][ID]);

	if (OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].find(ID) != OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].end())
		return reinterpret_cast<FEPointLight*>(OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT][ID]);
	
	return nullptr;
}

std::vector<std::string> FEScene::GetLightsList()
{
	std::vector<std::string> result;
	auto iterator = OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	iterator = OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	iterator = OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].end())
	{
		result.push_back(iterator->first);
		iterator++;
	}

	return result;
}

void FEScene::Clear()
{
	auto EntityIterator = EntityMap.begin();
	while (EntityIterator != EntityMap.end())
	{
		delete EntityIterator->second;
		EntityIterator++;
	}
	EntityMap.clear();

	std::vector<FEObject*> AllLights;
	auto iterator = OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		AllLights.push_back(iterator->second);
		iterator++;
	}

	iterator = OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].end())
	{
		AllLights.push_back(iterator->second);
		iterator++;
	}

	iterator = OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].begin();
	while (iterator != OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].end())
	{
		AllLights.push_back(iterator->second);
		iterator++;
	}

	for (size_t i = 0; i < AllLights.size(); i++)
	{
		if (AllLights[i]->GetType() == FE_DIRECTIONAL_LIGHT)
		{
			delete reinterpret_cast<FEDirectionalLight*>(AllLights[i]);
		}
		else if (AllLights[i]->GetType() == FE_POINT_LIGHT)
		{
			delete reinterpret_cast<FEPointLight*>(AllLights[i]);
		}
		else if (AllLights[i]->GetType() == FE_SPOT_LIGHT)
		{
			delete reinterpret_cast<FESpotLight*>(AllLights[i]);
		}
	}

	auto TerrainIterator = TerrainMap.begin();
	while (TerrainIterator != TerrainMap.end())
	{
		delete TerrainIterator->second;
		TerrainIterator++;
	}
	TerrainMap.clear();
}

void FEScene::PrepareForGameModelDeletion(const FEGameModel* GameModel)
{
	// looking if this gameModel is used in some prefab
	// to-do: should be done through list of pointers to entities that uses this gameModel.
	const auto PrefabList = RESOURCE_MANAGER.GetPrefabList();
	for (size_t i = 0; i < PrefabList.size(); i++)
	{
		FEPrefab* CurrentPrefab = RESOURCE_MANAGER.GetPrefab(PrefabList[i]);
		for (int j = 0; j < CurrentPrefab->ComponentsCount(); j++)
		{
			if (CurrentPrefab->GetComponent(j)->GameModel == GameModel)
			{
				CurrentPrefab->GetComponent(j)->GameModel = RESOURCE_MANAGER.GetGameModel(RESOURCE_MANAGER.GetStandardGameModelList()[0]);
				CurrentPrefab->SetDirtyFlag(true);
			}
		}
	}
}

void FEScene::PrepareForPrefabDeletion(const FEPrefab* Prefab)
{
	// looking if this prefab is used in some entities
	// to-do: should be done through list of pointers to entities that uses this gameModel.
	auto EntitiesIterator = EntityMap.begin();
	while (EntitiesIterator != EntityMap.end())
	{
		if (EntitiesIterator->second->Prefab == Prefab)
		{
			EntitiesIterator->second->Prefab = RESOURCE_MANAGER.GetPrefab(RESOURCE_MANAGER.GetStandardPrefabList()[0]);
			EntitiesIterator->second->SetDirtyFlag(true);
		}

		EntitiesIterator++;
	}
}

bool FEScene::AddTerrain(FETerrain* NewTerrain)
{
	if (NewTerrain == nullptr)
		return false;

	TerrainMap[NewTerrain->GetObjectID()] = NewTerrain;

	return true;
}

std::vector<std::string> FEScene::GetTerrainList()
{
	FE_MAP_TO_STR_VECTOR(TerrainMap)
}

FETerrain* FEScene::GetTerrain(const std::string ID)
{
	if (TerrainMap.find(ID) == TerrainMap.end())
		return nullptr;

	return TerrainMap[ID];
}

void FEScene::DeleteTerrain(const std::string ID)
{
	if (TerrainMap.find(ID) == TerrainMap.end())
		return;

	const FETerrain* TerrainToDelete = TerrainMap[ID];

	auto EntityIt = EntityMap.begin();
	while (EntityIt != EntityMap.end())
	{
		if (EntityIt->second->GetType() == FE_ENTITY_INSTANCED)
		{
			FEEntityInstanced* InstancedEntity = reinterpret_cast<FEEntityInstanced*>(EntityIt->second);
			if (InstancedEntity->GetSnappedToTerrain() == TerrainToDelete)
			{
				InstancedEntity->UnSnapFromTerrain();
			}
		}

		EntityIt++;
	}
	
	delete TerrainToDelete;
	TerrainMap.erase(ID);
}

FEEntityInstanced* FEScene::AddEntityInstanced(FEPrefab* Prefab, std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedEntityInstanced";

	FEEntityInstanced* NewEntityInstanced = new FEEntityInstanced(Prefab, Name);
	if (!ForceObjectID.empty())
		NewEntityInstanced->SetID(ForceObjectID);

	EntityMap[NewEntityInstanced->GetObjectID()] = NewEntityInstanced;
	return NewEntityInstanced;
}

FEEntityInstanced* FEScene::AddEntityInstanced(FEGameModel* GameModel, const std::string Name, const std::string ForceObjectID)
{
	FEPrefab* TempPrefab = RESOURCE_MANAGER.CreatePrefab(GameModel, GameModel->GetName());
	return AddEntityInstanced(TempPrefab, Name, ForceObjectID);
}

bool FEScene::AddEntityInstanced(FEEntityInstanced* NewEntityInstanced)
{
	if (NewEntityInstanced == nullptr)
		return false;

	if (NewEntityInstanced->Prefab == nullptr)
		return false;

	EntityMap[NewEntityInstanced->GetObjectID()] = NewEntityInstanced;

	return true;
}

FEEntityInstanced* FEScene::GetEntityInstanced(const std::string ID)
{
	if (EntityMap.find(ID) == EntityMap.end())
		return nullptr;

	if (EntityMap[ID]->GetType() != FE_ENTITY_INSTANCED)
		return nullptr;

	return reinterpret_cast<FEEntityInstanced*>(EntityMap[ID]);
}

std::vector<FEEntityInstanced*> FEScene::GetEntityInstancedByName(const std::string Name)
{
	std::vector<FEEntityInstanced*> result;

	auto it = EntityMap.begin();
	while (it != EntityMap.end())
	{
		if (it->second->GetType() != FE_ENTITY_INSTANCED)
		{
			it++;
			continue;
		}

		if (it->second->GetName() == Name)
		{
			result.push_back(reinterpret_cast<FEEntityInstanced*>(it->second));
		}

		it++;
	}

	return result;
}

void FEScene::SetSelectMode(FEEntityInstanced* EntityInstanced, const bool NewValue)
{
	auto it = EntityMap.begin();
	while (it != EntityMap.end())
	{
		if (it->second->GetType() != FE_ENTITY_INSTANCED)
		{
			it++;
			continue;
		}

		reinterpret_cast<FEEntityInstanced*>(it->second)->SetSelectMode(false);
		it++;
	}

	EntityInstanced->SetSelectMode(NewValue);
}

void FEScene::DeleteLight(const std::string ID)
{
	if (OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].find(ID) != OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		OBJECT_MANAGER.ObjectsByType[FE_DIRECTIONAL_LIGHT].erase(ID);
		return;
	}
	
	if (OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].find(ID) != OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].end())
	{
		OBJECT_MANAGER.ObjectsByType[FE_SPOT_LIGHT].erase(ID);
		return;
	}

	if (OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].find(ID) != OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].end())
	{
		OBJECT_MANAGER.ObjectsByType[FE_POINT_LIGHT].erase(ID);
		return;
	}

	return;
}

FEVirtualUIContext* FEScene::AddVirtualUIContext(int Width, int Height, FEMesh* SampleMesh, std::string Name)
{
	FEVirtualUIContext* NewVirtualUIContext = new FEVirtualUIContext(Width, Height, SampleMesh, Name);
	NewVirtualUIContext->CanvasEntity = AddEntity(NewVirtualUIContext->CanvasPrefab, Name + "_Virtual_UI_Canvas");
	NewVirtualUIContext->CanvasEntity->SetUniformLighting(true);
	VirtualUIContextMap[NewVirtualUIContext->GetObjectID()] = NewVirtualUIContext;
	return NewVirtualUIContext;
}

FEVirtualUIContext* FEScene::GetVirtualUIContext(const std::string ID)
{
	if (VirtualUIContextMap.find(ID) == VirtualUIContextMap.end())
		return nullptr;

	return VirtualUIContextMap[ID];
}

std::vector<std::string> FEScene::GetVirtualUIContextList()
{
	FE_MAP_TO_STR_VECTOR(VirtualUIContextMap)
}

void FEScene::DeleteVirtualUIContext(const std::string ID)
{
	if (VirtualUIContextMap.find(ID) == VirtualUIContextMap.end())
		return;

	const FEVirtualUIContext* VirtualUIContextToDelete = VirtualUIContextMap[ID];
	delete VirtualUIContextToDelete;
	VirtualUIContextMap.erase(ID);
}