#include "FEScene.h"
#include "Components/Systems/FEComponentSystems.h"

using namespace FocalEngine;

FEScene::FEScene() : FEObject(FE_OBJECT_TYPE::FE_SCENE, "Unnamed Scene")
{
	SceneGraph.Initialize(this);
}

FEScene::~FEScene()
{
	Clear();
}

void FEScene::SetFlag(FESceneFlag Flag, bool Value)
{
	if (Value)
		Flags |= Flag;
	else
		Flags = static_cast<FESceneFlag>(
			static_cast<std::underlying_type_t<FESceneFlag>>(Flags) &
			~static_cast<std::underlying_type_t<FESceneFlag>>(Flag)
		);
}

bool FEScene::HasFlag(FESceneFlag Flag) const
{
	return (static_cast<std::underlying_type_t<FESceneFlag>>(Flags) &
			static_cast<std::underlying_type_t<FESceneFlag>>(Flag)) != 0;
}

FEEntity* FEScene::GetEntity(std::string ID)
{
	if (EntityMap.find(ID) == EntityMap.end())
		return nullptr;

	return EntityMap[ID];
}

std::vector<FEEntity*> FEScene::GetEntityByName(const std::string Name)
{
	std::vector<FEEntity*> Result;

	auto EntityIterator = EntityMap.begin();
	while (EntityIterator != EntityMap.end())
	{
		if (EntityIterator->second->GetName() == Name)
			Result.push_back(EntityIterator->second);

		EntityIterator++;
	}

	return Result;
}

void FEScene::DeleteEntity(std::string ID)
{
	if (EntityMap.find(ID) == EntityMap.end())
		return;

	DeleteEntity(EntityMap[ID]);
}

void FEScene::DeleteEntity(FEEntity* Entity)
{
	if (Entity == nullptr)
	{
		LOG.Add("Call of FEScene::DeleteEntity with nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	std::string EntityID = Entity->ID;
	FENaiveSceneGraphNode* GraphNode = SceneGraph.GetNodeByEntityID(EntityID);
	if (GraphNode == nullptr)
	{
		LOG.Add("Entity does not have a graph node in FEScene::DeleteEntity", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	SceneGraph.DeleteNode(GraphNode);
}

void FEScene::ClearEntityRecords(std::string EntityID, entt::entity EnttEntity)
{
	EntityMap.erase(EntityID);
	EnttToEntity.erase(EnttEntity);
}

std::vector<std::string> FEScene::GetEntityIDList()
{
	FE_MAP_TO_STR_VECTOR(EntityMap)
}

void FEScene::Clear()
{
	bIsSceneClearing = true;

	SceneGraph.Clear();

	// Some entities could be not in the scene graph, so we need to delete them manually.
	auto EntityIterator = EntityMap.begin();
	while (EntityIterator != EntityMap.end())
	{
		delete EntityIterator->second;
		EntityIterator++;
	}
	EntityMap.clear();
	EnttToEntity.clear();

	// Force clear registry.
	Registry = entt::registry{};

	bIsSceneClearing = false;
}

void FEScene::PrepareForGameModelDeletion(const FEGameModel* GameModel)
{
	// Looking if this gameModel is used in some prefab.
	// to-do: should be done through list of pointers to entities that uses this gameModel.
}

// FIX ME!
void FEScene::PrepareForPrefabDeletion(const FEPrefab* Prefab)
{
	// Looking if this prefab is used in some entities.
	// to-do: should be done through list of pointers to entities that uses this gameModel.
	/*auto EntitiesIterator = EntityMap.begin();
	while (EntitiesIterator != EntityMap.end())
	{
		if (EntitiesIterator->second->Prefab == Prefab)
		{
			EntitiesIterator->second->Prefab = RESOURCE_MANAGER.GetPrefab(RESOURCE_MANAGER.GetStandardPrefabList()[0]);
			EntitiesIterator->second->SetDirtyFlag(true);
		}

		EntitiesIterator++;
	}*/
}

std::vector<FEObject*> FEScene::ImportAsset(std::string FileName)
{
	std::vector<FEObject*> Result;

	if (FileName.empty())
	{
		LOG.Add("call of FEScene::ImportAsset with empty FileName", "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}

	if (!FILE_SYSTEM.DoesFileExist(FileName))
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
	// .gltf could contain scene, so EntityIterator should be loaded in FEScene
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
	if (!FILE_SYSTEM.DoesFileExist(FileName))
	{
		LOG.Add("call of FEScene::LoadGLTF can't locate file: " + std::string(FileName), "FE_LOG_LOADING", FE_LOG_ERROR);
		return Result;
	}

	FEGLTFLoader& GLTF = FEGLTFLoader::GetInstance();
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

		if (!FILE_SYSTEM.DoesFileExist(FullPath.c_str()))
		{
			TextureMap[static_cast<int>(TextureMap.size())] = nullptr;
			continue;
		}

		FETexture* LoadedTexture = RESOURCE_MANAGER.ImportTexture(FullPath.c_str());
		//FETexture* LoadedTexture = RESOURCE_MANAGER.NoTexture;
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
			NewMaterial->SetNormalMapIntensity(GLTF.Materials[i].NormalTexture.Scale);
		}

		if (TextureMap.find(GLTF.Materials[i].OcclusionTexture.Index) != TextureMap.end() && TextureMap[GLTF.Materials[i].OcclusionTexture.Index] != nullptr)
		{
			NewMaterial->AddTexture(TextureMap[GLTF.Materials[i].OcclusionTexture.Index]);
			NewMaterial->SetAOMap(TextureMap[GLTF.Materials[i].OcclusionTexture.Index]);
			NewMaterial->SetAmbientOcclusionMapIntensity(GLTF.Materials[i].OcclusionTexture.Strength);
		}

		Result.push_back(NewMaterial);
	}

	// Each Primitive need to have GameModel, there could be multiple Primitives in one glTFMesh.
	std::unordered_map<int, std::vector<FEGameModel*>> GLTFMeshesToGameModelMap;
	for (size_t i = 0; i < GLTF.Meshes.size(); i++)
	{
		GLTFMeshesToGameModelMap[static_cast<int>(i)] = std::vector<FEGameModel*>();
		GLTFMeshesToGameModelMap[static_cast<int>(i)].resize(GLTF.Meshes[i].Primitives.size());

		for (size_t j = 0; j < GLTF.Meshes[i].Primitives.size(); j++)
		{
			GLTFMeshesToGameModelMap[static_cast<int>(i)][j] = nullptr;

			if (GLTF.Meshes[i].Primitives[j].RawData.Indices.empty())
			{
				LOG.Add("Primitive[" + std::to_string(j) + "].RawData.Indices is empty in function FEScene::LoadGLTF.", "FE_LOG_LOADING", FE_LOG_ERROR);
				continue;
			}

			if (GLTF.Meshes[i].Primitives[j].RawData.Positions.empty())
			{
				LOG.Add("Primitive[" + std::to_string(j) + "].RawData.Positions is empty in function FEScene::LoadGLTF.", "FE_LOG_LOADING", FE_LOG_ERROR);
				continue;
			}

			if (GLTF.Meshes[i].Primitives[j].RawData.Normals.empty())
			{
				LOG.Add("Primitive[" + std::to_string(j) + "].RawData.Normals is empty in function FEScene::LoadGLTF. Trying to calculate normals.", "FE_LOG_LOADING", FE_LOG_WARNING);

				GLTF.Meshes[i].Primitives[j].RawData.Normals.resize(GLTF.Meshes[i].Primitives[j].RawData.Positions.size());
				GEOMETRY.CalculateNormals(GLTF.Meshes[i].Primitives[j].RawData.Indices, GLTF.Meshes[i].Primitives[0].RawData.Positions, GLTF.Meshes[i].Primitives[j].RawData.Normals);
			}

			if (GLTF.Meshes[i].Primitives[j].RawData.Tangents.empty())
			{
				LOG.Add("Primitive[" + std::to_string(j) + "].RawData.Tangents is empty in function FEScene::LoadGLTF. Trying to calculate tangents.", "FE_LOG_LOADING", FE_LOG_WARNING);

				if (GLTF.Meshes[i].Primitives[j].RawData.UVs.empty())
				{
					LOG.Add("Primitive[" + std::to_string(j) + "].RawData.UVs is empty in function FEScene::LoadGLTF. Can't calculate tangents.", "FE_LOG_LOADING", FE_LOG_ERROR);
					continue;
				}

				GLTF.Meshes[i].Primitives[j].RawData.Tangents.resize(GLTF.Meshes[i].Primitives[j].RawData.Positions.size());
				GEOMETRY.CalculateTangents(GLTF.Meshes[i].Primitives[j].RawData.Indices, GLTF.Meshes[i].Primitives[j].RawData.Positions, GLTF.Meshes[i].Primitives[j].RawData.UVs[0], GLTF.Meshes[i].Primitives[j].RawData.Normals, GLTF.Meshes[i].Primitives[j].RawData.Tangents);
			}

			if (GLTF.Meshes[i].Primitives[j].Material != -1)
			{
				int UVIndex = 0;
				UVIndex = GLTF.Materials[GLTF.Meshes[i].Primitives[j].Material].PBRMetallicRoughness.BaseColorTexture.TexCoord;
				if (GLTF.Meshes[i].Primitives[j].RawData.UVs.size() <= UVIndex)
					UVIndex = 0;
			}

			Result.push_back(RESOURCE_MANAGER.RawDataToMesh(GLTF.Meshes[i].Primitives[j].RawData.Positions,
															GLTF.Meshes[i].Primitives[j].RawData.Normals,
															GLTF.Meshes[i].Primitives[j].RawData.Tangents,
															GLTF.Meshes[i].Primitives[j].RawData.UVs[0/*UVIndex*/],
															GLTF.Meshes[i].Primitives[j].RawData.Indices,
															GLTF.Meshes[i].Name));

			if (GLTF.Meshes[i].Primitives[j].Material != -1)
			{
				FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel(reinterpret_cast<FEMesh*>(Result.back()), MaterialsMap[GLTF.Meshes[i].Primitives[j].Material]);

				std::string GameModelName = "Unnamed Gamemodel";
				std::string PrefabName = "Unnamed Prefab";
				if (!GLTF.Meshes[i].Name.empty())
				{
					if (GLTF.Meshes[i].Primitives.size() == 1)
					{
						GameModelName = GLTF.Meshes[i].Name + "_GameModel";
						PrefabName = GLTF.Meshes[i].Name + "_Prefab";
					}
					else
					{
						GameModelName = GLTF.Meshes[i].Name + "_GameModel_Primitive_" + std::to_string(j);
						PrefabName = GLTF.Meshes[i].Name + "_Prefab_Primitive_" + std::to_string(j);
					}
				}

				NewGameModel->SetName(GameModelName);
				Result.push_back(NewGameModel);

				//FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab(NewGameModel);
				//NewPrefab->SetName(PrefabName);
				//GLTFMeshesToGameModelMap[static_cast<int>(i)][j] = NewPrefab;
				//Result.push_back(NewPrefab);

				GLTFMeshesToGameModelMap[static_cast<int>(i)][j] = NewGameModel;
			}
		}
	}

	if (GLTF.Scene != -1)
	{
		GLTFScene& SceneToLoad = GLTF.Scenes[GLTF.Scene];

		for (size_t i = 0; i < SceneToLoad.RootChildren.size(); i++)
		{
			AddGLTFNodeToSceneGraph(GLTF, GLTF.Nodes[SceneToLoad.RootChildren[i]], GLTFMeshesToGameModelMap, SceneGraph.GetRoot()->GetObjectID());
		}
	}

	GLTF.Clear();
	return Result;
}

std::vector<FEObject*> FEScene::AddGLTFNodeToSceneGraph(const FEGLTFLoader& GLTF, const GLTFNodes& Node, const std::unordered_map<int, std::vector<FEGameModel*>>& GLTFMeshesToGameModelMap, const std::string ParentID)
{
	std::vector<FEObject*> Result;

	int GLTFMeshToPrefabIndex = -1;
	GLTFMeshToPrefabIndex = Node.Mesh;

	std::string NodeName = "Unnamed Entity";
	if (!NodeName.empty())
		NodeName = Node.Name;
	FEEntity* Entity = CreateEntity(NodeName);

	FETransformComponent& Transform = Entity->GetComponent<FETransformComponent>();
	Transform.SetPosition(Node.Translation);
	Transform.SetQuaternion(Node.Rotation);
	Transform.SetScale(Node.Scale);

	FENaiveSceneGraphNode* AddedNode = SceneGraph.GetNodeByEntityID(Entity->GetObjectID());
	SceneGraph.MoveNode(AddedNode->GetObjectID(), ParentID, false);

	if (GLTFMeshToPrefabIndex != -1)
	{
		if (GLTFMeshesToGameModelMap.find(GLTFMeshToPrefabIndex) == GLTFMeshesToGameModelMap.end())
		{
			LOG.Add("PrefabMap does not contain GLTFMesheToPrefabIndex in FEScene::LoadGLTF", "FE_LOG_LOADING", FE_LOG_ERROR);
		}

		if (GLTFMeshesToGameModelMap.find(GLTFMeshToPrefabIndex)->second.empty())
		{
			LOG.Add("GLTFMeshesToPrefabMap[GLTFMesheToPrefabIndex] is empty in FEScene::LoadGLTF", "FE_LOG_LOADING", FE_LOG_ERROR);
		}

		std::vector<FEGameModel*> GameModels = GLTFMeshesToGameModelMap.find(GLTFMeshToPrefabIndex)->second;
		if (GameModels.size() == 1)
		{
			Entity->AddComponent<FEGameModelComponent>(GameModels[0]);
		}
		else
		{
			for (size_t i = 0; i < GameModels.size(); i++)
			{
				std::string CurrentNodeName = NodeName;
				CurrentNodeName = NodeName + "_Primitive_" + std::to_string(i);

				FEEntity* ChildEntity = CreateEntity(CurrentNodeName);
				ChildEntity->AddComponent<FEGameModelComponent>(GameModels[i]);
		
				FENaiveSceneGraphNode* ChildNode = SceneGraph.GetNodeByEntityID(ChildEntity->GetObjectID());
				SceneGraph.MoveNode(ChildNode->GetObjectID(), AddedNode->GetObjectID(), false);
			}
		}
	}

	Result.push_back(Entity);

	for (size_t i = 0; i < Node.Children.size(); i++)
	{
		if (Node.Children[i] < 0 || Node.Children[i] >= GLTF.Nodes.size())
		{
			LOG.Add("Node.Children[i] out of bounds in FEScene::AddGLTFNodeToSceneGraph", "FE_LOG_LOADING", FE_LOG_ERROR);
			continue;
		}

		GLTFNodes ChildNode = GLTF.Nodes[Node.Children[i]];
		std::vector<FEObject*> TempResult = AddGLTFNodeToSceneGraph(GLTF, ChildNode, GLTFMeshesToGameModelMap, AddedNode->GetObjectID());
		Result.insert(Result.end(), TempResult.begin(), TempResult.end());
	}

	return Result;
}

FEEntity* FEScene::CreateEntity(std::string Name, std::string ForceObjectID)
{
	FEEntity* Result = CreateEntityOrphan(Name, ForceObjectID);
	SceneGraph.AddNode(Result, false);

	return Result;
}

FEEntity* FEScene::CreateEntityOrphan(std::string Name, std::string ForceObjectID)
{
	return CreateEntityInternal(Name, ForceObjectID);
}

FEEntity* FEScene::CreateEntityInternal(std::string Name, std::string ForceObjectID)
{
	if (Name.empty())
		Name = "Unnamed Entity";

	FEEntity* Entity = new FEEntity(Registry.create(), this);
	if (!ForceObjectID.empty())
		Entity->SetID(ForceObjectID);
	Entity->SetName(Name);

	EntityMap[Entity->GetObjectID()] = Entity;
	EnttToEntity[Entity->EnTTEntity] = Entity;

	Entity->AddComponent<FETagComponent>();
	Entity->AddComponent<FETransformComponent>();

	return Entity;
}

FEEntity* FEScene::GetEntityByEnTT(entt::entity ID)
{
	if (EnttToEntity.find(ID) == EnttToEntity.end())
		return nullptr;

	return EnttToEntity[ID];
}

void FEScene::Update()
{

}

FEEntity* FEScene::DuplicateEntity(std::string ID, std::string NewEntityName)
{
	if (EntityMap.find(ID) == EntityMap.end())
		return nullptr;

	return DuplicateEntity(EntityMap[ID], NewEntityName);
}

FEEntity* FEScene::DuplicateEntity(FEEntity* SourceEntity, std::string NewEntityName)
{
	if (SourceEntity == nullptr)
		return nullptr;

	if (NewEntityName.empty())
		NewEntityName = SourceEntity->GetName() + "_Copy";

	return DuplicateEntityInternal(SourceEntity, NewEntityName);
}

FEEntity* FEScene::DuplicateEntityInternal(FEEntity* SourceEntity, std::string NewEntityName)
{
	FEEntity* NewEntity = CreateEntityOrphan(NewEntityName);

	std::vector<FEComponentTypeInfo> List = SourceEntity->GetComponentsInfoList();
	// Sort to make sure that components are loaded in proper order.
	COMPONENTS_TOOL.SortComponentsByLoadingPriority(List);

	for (size_t i = 0; i < List.size(); i++)
	{
		if (List[i].DuplicateComponent != nullptr)
			List[i].DuplicateComponent(SourceEntity, NewEntity);
	}

	return NewEntity;
}

FEEntity* FEScene::ImportEntity(FEEntity* EntityFromDifferentScene, FENaiveSceneGraphNode* TargetParent, std::function<bool(FEEntity*)> Filter)
{
	FEEntity* Result = nullptr;
	if (EntityFromDifferentScene == nullptr)
	{
		LOG.Add("EntityFromDifferentScene is nullptr in FEScene::ImportEntity", "FE_LOG_ECS", FE_LOG_ERROR);
		return Result;
	}

	if (EntityFromDifferentScene->GetParentScene() == this)
	{
		LOG.Add("EntityFromDifferentScene is already in this scene in FEScene::ImportEntity", "FE_LOG_ECS", FE_LOG_WARNING);
		return Result;
	}

	if (TargetParent == nullptr)
		TargetParent = SceneGraph.GetRoot();

	FENaiveSceneGraphNode* OriginalNode = EntityFromDifferentScene->GetParentScene()->SceneGraph.GetNodeByEntityID(EntityFromDifferentScene->GetObjectID());
	FENaiveSceneGraphNode* NewNode = SceneGraph.ImportNode(OriginalNode, TargetParent, Filter);
	if (NewNode == nullptr)
	{
		LOG.Add("Failed to import node in FEScene::ImportEntity", "FE_LOG_ECS", FE_LOG_ERROR);
		return Result;
	}

	Result = NewNode->GetEntity();
	return Result;
}

FEAABB FEScene::GetEntityAABB(std::string ID)
{
	if (EntityMap.find(ID) == EntityMap.end())
	{
		LOG.Add("Entity with ID: " + ID + " not found in FEScene::GetEntityAABB", "FE_LOG_ECS", FE_LOG_WARNING);
		return FEAABB();
	}

	return GetEntityAABB(EntityMap[ID]);
}

FEAABB FEScene::GetEntityAABB(FEEntity* Entity)
{
	FEAABB Result;

	if (Entity == nullptr)
	{
		LOG.Add("Call of FEScene::GetEntityAABB with nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return Result;
	}

	if (Entity->HasComponent<FEGameModelComponent>())
	{
		FEGameModel* GameModel = Entity->GetComponent<FEGameModelComponent>().GetGameModel();
		Result = GameModel->GetMesh()->GetAABB().Transform(Entity->GetComponent<FETransformComponent>().GetWorldMatrix());
	}

	if (Entity->HasComponent<FEInstancedComponent>())
	{
		Result = INSTANCED_RENDERING_SYSTEM.GetAABB(Entity);
	}

	if (Entity->HasComponent<FETerrainComponent>())
	{
		Result = TERRAIN_SYSTEM.GetAABB(Entity);
	}

	if (Entity->HasComponent<FEVirtualUIComponent>())
	{
		Result = Entity->GetComponent<FEVirtualUIComponent>().GetAABB();
	}

	// If entity has no renderable components, we can have FEAABB with zero volume.
	// But with position.
	if (Result.GetVolume() == 0 && GEOMETRY.IsEpsilonEqual(Result.GetSize(), glm::vec3(0.0f)))
	{
		Result.Min = Entity->GetComponent<FETransformComponent>().GetPosition();
		Result.Max = Entity->GetComponent<FETransformComponent>().GetPosition();
	}

	return Result;
}

FEAABB FEScene::GetSceneAABB(std::function<bool(FEEntity*)> Filter)
{
	FEAABB Result;
	SceneGraph.GetNodeAABB(Result, SceneGraph.GetRoot(), Filter);
	return Result;
}