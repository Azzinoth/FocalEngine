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

void FEScene::AddEntityToEntityMap(FEEntity* Entity)
{
	EntityMap[Entity->GetObjectID()] = Entity;
	SceneGraph.AddEntity(Entity);
}

FEEntity* FEScene::AddEntity(FEGameModel* GameModel, std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedEntity";

	FEEntity* NewEntity = RESOURCE_MANAGER.CreateEntity(GameModel, Name, ForceObjectID);
	AddEntityToEntityMap(NewEntity);
	return NewEntity;
}

FEEntity* FEScene::AddEntity(FEPrefab* Prefab, std::string Name, const std::string ForceObjectID)
{
	if (Name.empty())
		Name = "unnamedEntity";

	FEEntity* NewEntity = RESOURCE_MANAGER.CreateEntity(Prefab, Name, ForceObjectID);
	AddEntityToEntityMap(NewEntity);
	return NewEntity;
}

bool FEScene::AddEntity(FEEntity* NewEntity)
{
	if (NewEntity == nullptr)
		return false;

	if (NewEntity->Prefab == nullptr)
		return false;

	AddEntityToEntityMap(NewEntity);

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

	AddEntityToEntityMap(NewEntityInstanced);
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

	AddEntityToEntityMap(NewEntityInstanced);

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

std::vector<FEObject*> FEScene::ImportAsset(std::string FileName)
{
	std::vector<FEObject*> Result;

	if (FileName.empty())
	{
		LOG.Add("call of FEScene::ImportAsset with empty FileName", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}

	if (!FILE_SYSTEM.CheckFile(FileName))
	{
		LOG.Add("Can't locate file: " + std::string(FileName) + " in FEScene::ImportAsset", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}

	std::string FileExtention = FILE_SYSTEM.GetFileExtension(FileName);
	std::transform(FileExtention.begin(), FileExtention.end(), FileExtention.begin(), [](const unsigned char C) { return std::tolower(C); });

	if (FileExtention == ".png" || FileExtention == ".jpg" || FileExtention == ".bmp")
	{
		FETexture* LoadedTexture = RESOURCE_MANAGER.ImportTexture(FileName.c_str());
		if (LoadedTexture != nullptr)
			Result.push_back(LoadedTexture);
	}
	else if (FileExtention == ".obj")
	{
		std::vector<FEObject*> LoadedObjects = RESOURCE_MANAGER.ImportOBJ(FileName.c_str(), true);
		Result.insert(Result.end(), LoadedObjects.begin(), LoadedObjects.end());
	}
	// .gltf could contain scene, so it should be loaded in FEScene
	else if (FileExtention == ".gltf")
	{
		std::vector<FEObject*> LoadedObjects = LoadGLTF(FileName);
		Result.insert(Result.end(), LoadedObjects.begin(), LoadedObjects.end());
	}

	return Result;
}

std::vector<FEObject*> FEScene::LoadGLTF(std::string FileName)
{
	std::vector<FEObject*> Result;
	if (!FILE_SYSTEM.CheckFile(FileName))
	{
		LOG.Add("call of FEScene::LoadGLTF can't locate file: " + std::string(FileName), "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}

	FEGLTFLoader& GLTF = FEGLTFLoader::getInstance();
	GLTF.Load(FileName.c_str());
	std::string Directory = FILE_SYSTEM.GetDirectoryPath(FileName);

	std::unordered_map<std::string, FETexture*> AlreadyLoadedTextures;
	std::unordered_map<int, FETexture*> TextureMap;
	for (size_t i = 0; i < GLTF.Textures.size(); i++)
	{
		int ImageIndex = GLTF.Textures[i].Source;
		if (ImageIndex < 0 || ImageIndex >= GLTF.Images.size())
		{
			LOG.Add("FEScene::LoadGLTF image index out of bounds", "FE_LOG_LOADING", FE_LOG_ERROR);
			continue;
		}

		std::string FullPath = Directory + "\\" + GLTF.Images[ImageIndex].Uri;
		if (AlreadyLoadedTextures.find(FullPath) != AlreadyLoadedTextures.end())
		{
			TextureMap[static_cast<int>(TextureMap.size())] = AlreadyLoadedTextures[FullPath];
			continue;
		}

		if (!FILE_SYSTEM.CheckFile(FullPath.c_str()))
		{
			TextureMap[static_cast<int>(TextureMap.size())] = nullptr;
			continue;
		}

		FETexture* LoadedTexture = RESOURCE_MANAGER.ImportTexture(FullPath.c_str());
		if (LoadedTexture != nullptr)
		{
			if (!GLTF.Textures[i].Name.empty())
			{
				LoadedTexture->SetName(GLTF.Textures[i].Name);
			}
			else if (!GLTF.Images[ImageIndex].Name.empty())
			{
				LoadedTexture->SetName(GLTF.Images[ImageIndex].Name);
			}

			TextureMap[static_cast<int>(TextureMap.size())] = LoadedTexture;
			AlreadyLoadedTextures[FullPath] = LoadedTexture;
			Result.push_back(LoadedTexture);
		}
	}

	std::unordered_map<int, FEMaterial*> MaterialsMap;
	for (size_t i = 0; i < GLTF.Materials.size(); i++)
	{
		FEMaterial* NewMaterial = RESOURCE_MANAGER.CreateMaterial(GLTF.Materials[i].Name);
		MaterialsMap[static_cast<int>(i)] = NewMaterial;
		NewMaterial->Shader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

		if (TextureMap.find(GLTF.Materials[i].PBRMetallicRoughness.BaseColorTexture.Index) != TextureMap.end() && TextureMap[GLTF.Materials[i].PBRMetallicRoughness.BaseColorTexture.Index] != nullptr)
		{
			NewMaterial->AddTexture(TextureMap[GLTF.Materials[i].PBRMetallicRoughness.BaseColorTexture.Index]);
			NewMaterial->SetAlbedoMap(TextureMap[GLTF.Materials[i].PBRMetallicRoughness.BaseColorTexture.Index]);
		}
		else if (GLTF.Materials[i].PBRMetallicRoughness.BaseColorFactor[0] != -1)
		{
			NewMaterial->SetBaseColor(GLTF.Materials[i].PBRMetallicRoughness.BaseColorFactor);
		}

		if (TextureMap.find(GLTF.Materials[i].PBRMetallicRoughness.MetallicRoughnessTexture.Index) != TextureMap.end() && TextureMap[GLTF.Materials[i].PBRMetallicRoughness.MetallicRoughnessTexture.Index] != nullptr)
		{
			/*
				https://github.com/KhronosGroup/glTF/blob/main/specification/2.0/Specification.adoc#reference-material
				The textures for metalness and roughness properties are packed together in a single texture called metallicRoughnessTexture.
				Its green channel contains roughness values and its blue channel contains metalness values.
			*/
			NewMaterial->AddTexture(TextureMap[GLTF.Materials[i].PBRMetallicRoughness.MetallicRoughnessTexture.Index]);
			NewMaterial->SetRoughnessMap(TextureMap[GLTF.Materials[i].PBRMetallicRoughness.MetallicRoughnessTexture.Index], 1, 0);
			NewMaterial->SetMetalnessMap(TextureMap[GLTF.Materials[i].PBRMetallicRoughness.MetallicRoughnessTexture.Index], 2, 0);
		}

		if (TextureMap.find(GLTF.Materials[i].NormalTexture.Index) != TextureMap.end() && TextureMap[GLTF.Materials[i].NormalTexture.Index] != nullptr)
		{
			NewMaterial->AddTexture(TextureMap[GLTF.Materials[i].NormalTexture.Index]);
			NewMaterial->SetNormalMap(TextureMap[GLTF.Materials[i].NormalTexture.Index]);
		}

		if (TextureMap.find(GLTF.Materials[i].OcclusionTexture.Index) != TextureMap.end() && TextureMap[GLTF.Materials[i].OcclusionTexture.Index] != nullptr)
		{
			NewMaterial->AddTexture(TextureMap[GLTF.Materials[i].OcclusionTexture.Index]);
			NewMaterial->SetAOMap(TextureMap[GLTF.Materials[i].OcclusionTexture.Index]);
		}

		Result.push_back(NewMaterial);
	}

	std::unordered_map<int, FEPrefab*> PrefabMap;
	for (size_t i = 0; i < GLTF.Meshes.size(); i++)
	{
		PrefabMap[static_cast<int>(i)] = nullptr;

		if (!GLTF.Meshes[i].Primitives[0].RawData.Indices.empty())
		{
			if (GLTF.Meshes[i].Primitives[0].Material != -1)
			{
				int UVIndex = 0;
				UVIndex = GLTF.Materials[GLTF.Meshes[i].Primitives[0].Material].PBRMetallicRoughness.BaseColorTexture.TexCoord;
				if (GLTF.Meshes[i].Primitives[0].RawData.UVs.size() <= UVIndex)
					UVIndex = 0;
			}

			Result.push_back(RESOURCE_MANAGER.RawDataToMesh(GLTF.Meshes[i].Primitives[0].RawData.Positions,
															GLTF.Meshes[i].Primitives[0].RawData.Normals,
															GLTF.Meshes[i].Primitives[0].RawData.Tangents,
															GLTF.Meshes[i].Primitives[0].RawData.UVs[0/*UVIndex*/],
															GLTF.Meshes[i].Primitives[0].RawData.Indices,
															GLTF.Meshes[i].Name));

			if (GLTF.Meshes[i].Primitives[0].Material != -1)
			{
				FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(reinterpret_cast<FEMesh*>(Result.back()), MaterialsMap[GLTF.Meshes[i].Primitives[0].Material]);
				NewGameModel->SetName(GLTF.Meshes[i].Name + "_GameModel");
				Result.push_back(NewGameModel);

				FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab(NewGameModel);
				NewPrefab->SetName(GLTF.Meshes[i].Name + "_Prefab");
				PrefabMap[static_cast<int>(i)] = NewPrefab;
				Result.push_back(NewPrefab);
			}
		}
	}

	for (size_t i = 0; i < GLTF.Nodes.size(); i++)
	{
		int PrefabIndex = -1;
		PrefabIndex = GLTF.Nodes[i].Mesh;

		if (PrefabIndex != -1)
		{
			if (PrefabMap.find(PrefabIndex) == PrefabMap.end())
			{
				LOG.Add("PrefabMap does not contain PrefabIndex in FEScene::LoadGLTF", "FE_LOG_LOADING", FE_LOG_ERROR);
				continue;
			}

			if (PrefabMap[PrefabIndex] == nullptr)
			{
				LOG.Add("PrefabMap[PrefabIndex] is nullptr in FEScene::LoadGLTF", "FE_LOG_LOADING", FE_LOG_ERROR);
				continue;
			}

			FEEntity* NewEntity = AddEntity(PrefabMap[PrefabIndex], GLTF.Nodes[i].Name);
			NewEntity->Transform.SetPosition(GLTF.Nodes[i].Translation);
			NewEntity->Transform.RotateByQuaternion(GLTF.Nodes[i].Rotation);
			NewEntity->Transform.SetScale(GLTF.Nodes[i].Scale);

			Result.push_back(NewEntity);
		}
	}

	GLTF.Clear();

	return Result;
}