#include "FEInstancedSystem.h"
using namespace FocalEngine;

FEInstancedSystem* FEInstancedSystem::Instance = nullptr;
FEInstancedSystem::FEInstancedSystem()
{
	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FEInstancedComponent>(InstanceComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FEInstancedComponent>(InstanceComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FEInstancedComponent>(DuplicateInstancedComponent);
}

void FEInstancedSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FEInstancedComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FEInstancedComponent>(OnMyComponentDestroy);
}

void FEInstancedSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (INSTANCED_RENDERING_SYSTEM.bInternalAdd)
		return;

	if (Entity == nullptr || !Entity->HasComponent<FEInstancedComponent>())
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
	{
		LOG.Add("FEInstancedComponent should be added to entity with FEGameModelComponent or FEPrefabInstanceComponent", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	INSTANCED_RENDERING_SYSTEM.InitializeBuffers(Entity);
}

// There should be third system to manage that.
#include "../SubSystems/Scene/Components/Systems/FETerrainSystem.h"
void FEInstancedSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (bIsSceneClearing)
		return;

	if (Entity == nullptr || !Entity->HasComponent<FEInstancedComponent>())
		return;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	if (InstancedComponent.TerrainToSnap != nullptr)
	{
		TERRAIN_SYSTEM.UnSnapInstancedEntity(InstancedComponent.TerrainToSnap, Entity);
	}
}

FEInstancedSystem::~FEInstancedSystem() {};

void FEInstancedSystem::InitializeBuffers(FEEntity* Entity)
{
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	size_t CurrentBufferIndex = 0;
	if (Entity->HasComponent<FEPrefabInstanceComponent>())
	{
		FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
		FEScene* PrefabScene = PrefabInstanceComponent.GetPrefab()->GetScene();

		std::vector<std::string> AllPrefabEntities = PrefabScene->GetEntityIDListWith<FEGameModelComponent>();
		for (size_t i = 0; i < AllPrefabEntities.size(); i++)
		{
			FEEntity* CurrentPrefabEntity = PrefabScene->GetEntity(AllPrefabEntities[i]);
			if (CurrentPrefabEntity != nullptr)
			{
				FEGameModelComponent& GameModelComponent = CurrentPrefabEntity->GetComponent<FEGameModelComponent>();
				FEInstancedElementData* NewData = new FEInstancedElementData();
				InstancedComponent.InstancedElementsData.push_back(NewData);

				InstancedComponent.InstancedElementsData[CurrentBufferIndex]->EntityIDWithGameModelComponent = CurrentPrefabEntity->GetObjectID();
				InitializeBuffer(Entity, GameModelComponent, CurrentBufferIndex);
				CurrentBufferIndex++;
			}
		}
	}
	else
	{
		FEInstancedElementData* NewData = new FEInstancedElementData();
		InstancedComponent.InstancedElementsData.push_back(NewData);

		InstancedComponent.InstancedElementsData[CurrentBufferIndex]->EntityIDWithGameModelComponent = Entity->GetObjectID();
		InitializeBuffer(Entity, Entity->GetComponent<FEGameModelComponent>(), CurrentBufferIndex);
	}
}

void FEInstancedSystem::InitializeBuffer(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex)
{
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	InstancedComponent.InstancedElementsData[BufferIndex]->LODCounts = new int[GameModelComponent.GetGameModel()->GetMaxLODCount()];

	InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers = new GLenum[GameModelComponent.GetGameModel()->GetMaxLODCount()];
	for (size_t i = 0; i < GameModelComponent.GetGameModel()->GetMaxLODCount(); i++)
	{
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[i]));
	}

	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->SourceDataBuffer));
	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->PositionsBuffer));
	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->AABBSizesBuffer));
	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODInfoBuffer));

	InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo = new FEDrawElementsIndirectCommand[4];
	for (size_t i = 0; i < GameModelComponent.GetGameModel()->GetMaxLODCount(); i++)
	{
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[i].Count = GameModelComponent.GetGameModel()->GetLODMesh(i) == nullptr ? 0 : GameModelComponent.GetGameModel()->GetLODMesh(i)->GetVertexCount();
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[i].BaseInstance = 0;
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[i].BaseVertex = 0;
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[i].FirstIndex = 0;
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[i].PrimCount = 0;
	}

	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer));
	FE_GL_ERROR(glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(FEDrawElementsIndirectCommand) * 4, InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo, GL_MAP_READ_BIT));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

	InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatricesLOD.resize(GameModelComponent.GetGameModel()->GetMaxLODCount());
}

void FEInstancedSystem::InitializeGPUCullingBuffers(FEEntity* Entity)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	size_t CurrentBufferIndex = 0;
	if (Entity->HasComponent<FEPrefabInstanceComponent>())
	{
		FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
		FEScene* PrefabScene = PrefabInstanceComponent.GetPrefab()->GetScene();

		std::vector<std::string> AllPrefabEntities = PrefabScene->GetEntityIDListWith<FEGameModelComponent>();
		for (size_t i = 0; i < AllPrefabEntities.size(); i++)
		{
			FEEntity* CurrentPrefabEntity = PrefabScene->GetEntity(AllPrefabEntities[i]);
			if (CurrentPrefabEntity != nullptr)
			{
				FEGameModelComponent& GameModelComponent = CurrentPrefabEntity->GetComponent<FEGameModelComponent>();
				InitializeGPUCullingBuffer(Entity, GameModelComponent, CurrentBufferIndex);
				CurrentBufferIndex++;
			}
		}
	}
	else
	{
		InitializeGPUCullingBuffer(Entity, Entity->GetComponent<FEGameModelComponent>(), CurrentBufferIndex);
	}
}

void FEInstancedSystem::InitializeGPUCullingBuffer(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex)
{
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	if (InstancedComponent.InstancedElementsData[BufferIndex]->SourceDataBuffer != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->SourceDataBuffer));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->SourceDataBuffer));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, InstancedComponent.InstancedElementsData[BufferIndex]->SourceDataBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices.data(), GL_DYNAMIC_DRAW));

	if (InstancedComponent.InstancedElementsData[BufferIndex]->PositionsBuffer != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->PositionsBuffer));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->PositionsBuffer));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, InstancedComponent.InstancedElementsData[BufferIndex]->PositionsBuffer));

	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(float) * 3, InstancedComponent.InstancedElementsData[BufferIndex]->InstancePositions.data(), GL_DYNAMIC_DRAW));

	if (InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[0] != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[0]));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[0]));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[0]));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

	if (InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[1] != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[1]));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[1]));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[1]));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

	if (InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[2] != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[2]));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[2]));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[2]));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

	if (InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[3] != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[3]));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[3]));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[3]));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

	if (InstancedComponent.InstancedElementsData[BufferIndex]->AABBSizesBuffer != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->AABBSizesBuffer));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->AABBSizesBuffer));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, InstancedComponent.InstancedElementsData[BufferIndex]->AABBSizesBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(float), InstancedComponent.InstancedElementsData[BufferIndex]->InstancedAABBSizes.data(), GL_DYNAMIC_DRAW));

	std::vector<float> LODInfoData;
	LODInfoData.push_back(GameModelComponent.GetGameModel()->GetCullDistance());
	LODInfoData.push_back(GameModelComponent.GetGameModel()->GetLODMaxDrawDistance(0));
	LODInfoData.push_back(GameModelComponent.GetGameModel()->GetLODMaxDrawDistance(1));
	LODInfoData.push_back(GameModelComponent.GetGameModel()->GetLODMaxDrawDistance(2));

	// does it have billboard ?
	unsigned int BillboardIndex = 5;
	for (size_t j = 0; j < GameModelComponent.GetGameModel()->GetMaxLODCount(); j++)
	{
		if (GameModelComponent.GetGameModel()->IsLODBillboard(j) && GameModelComponent.GetGameModel()->GetLODMesh(j) != nullptr)
		{
			BillboardIndex = static_cast<int>(j);
		}
	}

	LODInfoData.push_back(static_cast<float>(BillboardIndex));
	// This should not be here, instead normal of plane should align with vector to camera.
	LODInfoData.push_back(1.5708f * 3.0f + GameModelComponent.GetGameModel()->GetBillboardZeroRotaion() * ANGLE_TORADIANS_COF);
	LODInfoData.push_back(static_cast<float>(InstancedComponent.InstanceCount));

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, InstancedComponent.InstancedElementsData[BufferIndex]->LODInfoBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, 7 * sizeof(float), LODInfoData.data(), GL_DYNAMIC_DRAW));

	if (InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo == nullptr)
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo = new FEDrawElementsIndirectCommand[4];

	for (size_t j = 0; j < GameModelComponent.GetGameModel()->GetMaxLODCount(); j++)
	{
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[j].Count = GameModelComponent.GetGameModel()->GetLODMesh(j) == nullptr ? 0 : GameModelComponent.GetGameModel()->GetLODMesh(j)->GetVertexCount();
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[j].BaseInstance = 0;
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[j].BaseVertex = 0;
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[j].FirstIndex = 0;
		InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo[j].PrimCount = 0;
	}

	if (InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer));
	}

	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer));
	FE_GL_ERROR(glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(FEDrawElementsIndirectCommand) * 4, InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawsInfo, GL_MAP_READ_BIT));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
}

void FEInstancedSystem::UpdateBuffers(FEEntity* Entity)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;
	
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	size_t CurrentBufferIndex = 0;
	if (Entity->HasComponent<FEPrefabInstanceComponent>())
	{
		FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
		FEScene* PrefabScene = PrefabInstanceComponent.GetPrefab()->GetScene();

		std::vector<std::string> AllPrefabEntities = PrefabScene->GetEntityIDListWith<FEGameModelComponent>();
		for (size_t i = 0; i < AllPrefabEntities.size(); i++)
		{
			FEEntity* CurrentPrefabEntity = PrefabScene->GetEntity(AllPrefabEntities[i]);
			if (CurrentPrefabEntity != nullptr)
			{
				FEGameModelComponent& GameModelComponent = CurrentPrefabEntity->GetComponent<FEGameModelComponent>();
				UpdateBuffer(Entity, GameModelComponent, CurrentBufferIndex);
				CurrentBufferIndex++;
			}
		}
	}
	else
	{
		UpdateBuffer(Entity, Entity->GetComponent<FEGameModelComponent>(), CurrentBufferIndex);
	}

	//UpdateBuffers(TransformComponent, GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::UpdateBuffer(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex)
{
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	if (InstancedComponent.InstancedElementsData[BufferIndex]->InstancedBuffer != 0)
	{
		glDeleteBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->InstancedBuffer);
	}

	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedElementsData[BufferIndex]->InstancedBuffer));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, InstancedComponent.InstancedElementsData[BufferIndex]->InstancedBuffer));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatrices.data(), GL_DYNAMIC_DRAW));

	const unsigned int VAO = GameModelComponent.GetGameModel()->Mesh->GetVaoID();
	FE_GL_ERROR(glBindVertexArray(VAO));
	// set attribute pointers for matrix (4 times vec4)
	FE_GL_ERROR(glEnableVertexAttribArray(6));
	FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr)));
	FE_GL_ERROR(glEnableVertexAttribArray(7));
	FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
	FE_GL_ERROR(glEnableVertexAttribArray(8));
	FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
	FE_GL_ERROR(glEnableVertexAttribArray(9));
	FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

	FE_GL_ERROR(glVertexAttribDivisor(6, 1));
	FE_GL_ERROR(glVertexAttribDivisor(7, 1));
	FE_GL_ERROR(glVertexAttribDivisor(8, 1));
	FE_GL_ERROR(glVertexAttribDivisor(9, 1));

	FE_GL_ERROR(glBindVertexArray(0));

	for (size_t i = 0; i < GameModelComponent.GetGameModel()->GetMaxLODCount(); i++)
	{
		if (GameModelComponent.GetGameModel()->GetLODMesh(i) != nullptr)
		{
			const unsigned int VAO = GameModelComponent.GetGameModel()->GetLODMesh(i)->GetVaoID();
			FE_GL_ERROR(glBindVertexArray(VAO));
			// set attribute pointers for matrix (4 times vec4)
			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr)));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glVertexAttribDivisor(6, 1));
			FE_GL_ERROR(glVertexAttribDivisor(7, 1));
			FE_GL_ERROR(glVertexAttribDivisor(8, 1));
			FE_GL_ERROR(glVertexAttribDivisor(9, 1));

			FE_GL_ERROR(glBindVertexArray(0));
		}
	}

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	InstancedComponent.InstancedElementsData[BufferIndex]->AllInstancesAABB = FEAABB();
	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	glm::vec3 Position = TransformComponent.GetPosition();
	for (size_t i = 0; i < InstancedComponent.InstanceCount; i++)
	{
		glm::mat4 MatWithoutTranslate = InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices[i];
		MatWithoutTranslate[3][0] -= Position.x;
		MatWithoutTranslate[3][1] -= Position.y;
		MatWithoutTranslate[3][2] -= Position.z;

		InstancedComponent.InstancedElementsData[BufferIndex]->AllInstancesAABB = InstancedComponent.InstancedElementsData[BufferIndex]->AllInstancesAABB.Merge(GameModelComponent.GetGameModel()->Mesh->AABB.Transform(MatWithoutTranslate));
	}
	
	TransformComponent.SetDirtyFlag(true);
	GetAABB(Entity);
}

void FEInstancedSystem::DuplicateInstancedComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr)
		return;

	if (!SourceEntity->HasComponent<FEGameModelComponent>() || !TargetEntity->HasComponent<FEGameModelComponent>())
		return;

	if (!SourceEntity->HasComponent<FEInstancedComponent>())
		return;

	FEInstancedComponent& OriginalInstancedComponent = SourceEntity->GetComponent<FEInstancedComponent>();
	INSTANCED_RENDERING_SYSTEM.bInternalAdd = true;
	TargetEntity->AddComponent<FEInstancedComponent>();
	INSTANCED_RENDERING_SYSTEM.bInternalAdd = false;

	FEInstancedComponent& NewInstancedComponent = TargetEntity->GetComponent<FEInstancedComponent>();
	NewInstancedComponent = OriginalInstancedComponent;
	// FIX ME! This is not correct, it should be copied from source entity, but with reinitalized buffers.
	//NewInstancedComponent.InstancedElementsData.clear();
	INSTANCED_RENDERING_SYSTEM.InitializeBuffers(TargetEntity);
}

void FEInstancedSystem::AddInstanceInternal(FEEntity* Entity, const glm::mat4 InstanceMatrix)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
	{
		FEEntity* EntityWithGameModel = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
		if (EntityWithGameModel == nullptr)
			continue;

		FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
		AddInstanceInternal(Entity, GameModelComponent, InstanceMatrix, i);
	}

	InstancedComponent.InstanceCount++;
	InstancedComponent.bDirtyFlag = true;
}

void FEInstancedSystem::AddInstanceInternal(FEEntity* Entity, FEGameModelComponent& GameModelComponent, glm::mat4 InstanceMatrix, size_t BufferIndex)
{
	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	InstancedComponent.InstancedElementsData[BufferIndex]->InstancedAABBSizes.push_back(-FEAABB(GameModelComponent.GetGameModel()->GetMesh()->GetAABB(), InstanceMatrix).GetLongestAxisLength());
	InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatrices.push_back(InstanceMatrix);
	InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices.push_back(TransformComponent.GetWorldMatrix() * InstanceMatrix);
	InstancedComponent.InstancedElementsData[BufferIndex]->InstancePositions.push_back(InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices.back()[3]);

	for (size_t j = 0; j < GameModelComponent.GetGameModel()->GetMaxLODCount(); j++)
	{
		InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatricesLOD[j].resize(InstancedComponent.InstanceCount);
	}
}

void FEInstancedSystem::AddInstances(FEEntity* Entity, const glm::mat4* InstanceMatrix, size_t Count)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	InstancedComponent.InstanceCount += Count;
	for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
	{
		FEEntity* EntityWithGameModel = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
		if (EntityWithGameModel == nullptr)
			continue;

		FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
		AddInstances(Entity, GameModelComponent, InstanceMatrix, Count, i);
	}

	InstancedComponent.bDirtyFlag = true;
}

void FEInstancedSystem::AddInstances(FEEntity* Entity, FEGameModelComponent& GameModelComponent, const glm::mat4* InstanceMatrix, size_t Count, size_t BufferIndex)
{
	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	const size_t StartIndex = InstancedComponent.InstancedElementsData[BufferIndex]->InstancedAABBSizes.size();

	InstancedComponent.InstancedElementsData[BufferIndex]->InstancedAABBSizes.resize(InstancedComponent.InstancedElementsData[BufferIndex]->InstancedAABBSizes.size() + Count);
	const FEAABB OriginalAABB = GameModelComponent.GetGameModel()->GetMesh()->GetAABB();
	InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatrices.resize(InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatrices.size() + Count);
	InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices.resize(InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices.size() + Count);
	InstancedComponent.InstancedElementsData[BufferIndex]->InstancePositions.resize(InstancedComponent.InstancedElementsData[BufferIndex]->InstancePositions.size() + Count);

	for (size_t j = StartIndex; j < Count; j++)
	{
		InstancedComponent.InstancedElementsData[BufferIndex]->InstancedAABBSizes[j] = -FEAABB(OriginalAABB, InstanceMatrix[j]).GetLongestAxisLength();
		InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatrices[j] = InstanceMatrix[j];
		InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices[j] = TransformComponent.GetWorldMatrix() * InstanceMatrix[j];

		InstancedComponent.InstancedElementsData[BufferIndex]->InstancePositions[j] = InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices[j][3];
	}

	for (size_t j = 0; j < GameModelComponent.GetGameModel()->GetMaxLODCount(); j++)
	{
		InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatricesLOD[j].resize(InstancedComponent.InstanceCount);
	}
}

FEAABB FEInstancedSystem::GetAABB(FEEntity* Entity)
{
	if (Entity == nullptr)
		return FEAABB();

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return FEAABB();

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	FEAABB Result = FEAABB();
	for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
	{
		Result = Result.Merge(InstancedComponent.InstancedElementsData[i]->AllInstancesAABB.Transform(TransformComponent.GetWorldMatrix()));
	}

	if (TransformComponent.IsDirty())
	{
		UpdateMatrices(Entity);
		TransformComponent.SetDirtyFlag(false);
	}
	
	return Result;
}

void FEInstancedSystem::UpdateMatrices(FEEntity* Entity)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	size_t CurrentBufferIndex = 0;
	if (Entity->HasComponent<FEPrefabInstanceComponent>())
	{
		FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
		FEScene* PrefabScene = PrefabInstanceComponent.GetPrefab()->GetScene();

		std::vector<std::string> AllPrefabEntities = PrefabScene->GetEntityIDListWith<FEGameModelComponent>();
		for (size_t i = 0; i < AllPrefabEntities.size(); i++)
		{
			FEEntity* CurrentPrefabEntity = PrefabScene->GetEntity(AllPrefabEntities[i]);
			if (CurrentPrefabEntity != nullptr)
			{
				UpdateMatrix(Entity, CurrentBufferIndex);
				CurrentBufferIndex++;
			}
		}
	}
	else
	{
		UpdateMatrix(Entity, CurrentBufferIndex);
	}
}

void FEInstancedSystem::UpdateMatrix(FEEntity* Entity, size_t BufferIndex)
{
	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	if (InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatrices.size() != InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices.size())
	{
		LOG.Add("InstancedMatrices size and TransformedInstancedMatrices size is not equal in FEInstancedSystem::UpdateMatrix", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	for (size_t i = 0; i < InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatrices.size(); i++)
	{
		InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices[i] = TransformComponent.GetWorldMatrix() * InstancedComponent.InstancedElementsData[BufferIndex]->InstancedMatrices[i];
		InstancedComponent.InstancedElementsData[BufferIndex]->InstancePositions[i] = InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices[i][3];
	}

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, InstancedComponent.InstancedElementsData[BufferIndex]->SourceDataBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), InstancedComponent.InstancedElementsData[BufferIndex]->TransformedInstancedMatrices.data(), GL_DYNAMIC_DRAW));

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, InstancedComponent.InstancedElementsData[BufferIndex]->PositionsBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(float) * 3, InstancedComponent.InstancedElementsData[BufferIndex]->InstancePositions.data(), GL_DYNAMIC_DRAW));
}

void FEInstancedSystem::Render(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	if (InstancedComponent.InstanceCount == 0)
		return;

	CheckDirtyFlag(Entity);

	RenderGameModelComponent(GameModelComponent, InstancedComponent, BufferIndex);
}

void FEInstancedSystem::RenderGameModelComponent(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, size_t BufferIndex)
{
	for (size_t i = 0; i < GameModelComponent.GetGameModel()->GetMaxLODCount(); i++)
	{
		if (GameModelComponent.GetGameModel()->IsLODBillboard(i))
			break;

		if (GameModelComponent.GetGameModel()->GetLODMesh(i) != nullptr)
		{
			if (InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[i] == 0)
				break;

			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[i]));

			FE_GL_ERROR(glBindVertexArray(GameModelComponent.GetGameModel()->GetLODMesh(i)->GetVaoID()));

			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(0)));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer));
			FE_GL_ERROR(glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(i * sizeof(FEDrawElementsIndirectCommand))));

			FE_GL_ERROR(glBindVertexArray(0));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
		}
	}

	if (InstancedComponent.CullingType == FE_CULLING_NONE)
	{
		FE_GL_ERROR(glBindVertexArray(GameModelComponent.GetGameModel()->Mesh->GetVaoID()));
		if ((GameModelComponent.GetGameModel()->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((GameModelComponent.GetGameModel()->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((GameModelComponent.GetGameModel()->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((GameModelComponent.GetGameModel()->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((GameModelComponent.GetGameModel()->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((GameModelComponent.GetGameModel()->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		FE_GL_ERROR(glEnableVertexAttribArray(6));
		FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr)));
		FE_GL_ERROR(glEnableVertexAttribArray(7));
		FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(8));
		FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(9));
		FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, GameModelComponent.GetGameModel()->Mesh->GetVertexCount(), GL_UNSIGNED_INT, nullptr, static_cast<int>(InstancedComponent.InstanceCount)));

		FE_GL_ERROR(glBindVertexArray(0));
	}
}

void FEInstancedSystem::RenderOnlyBillbords(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	RenderOnlyBillbordsInternal(Entity, GameModelComponent, BufferIndex);
}

void FEInstancedSystem::RenderOnlyBillbordsInternal(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex)
{
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	for (size_t i = 0; i < GameModelComponent.GetGameModel()->GetMaxLODCount(); i++)
	{
		if (GameModelComponent.GetGameModel()->IsLODBillboard(i) && GameModelComponent.GetGameModel()->GetLODMesh(i) != nullptr)
		{
			if (InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[i] == 0)
				break;

			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, InstancedComponent.InstancedElementsData[BufferIndex]->LODBuffers[i]));

			FE_GL_ERROR(glBindVertexArray(GameModelComponent.GetGameModel()->GetLODMesh(i)->GetVaoID()));

			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((GameModelComponent.GetGameModel()->GetLODMesh(i)->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr)));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, InstancedComponent.InstancedElementsData[BufferIndex]->IndirectDrawInfoBuffer));
			FE_GL_ERROR(glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(i * sizeof(FEDrawElementsIndirectCommand))));

			FE_GL_ERROR(glBindVertexArray(0));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

			break;
		}
	}
}

void FEInstancedSystem::AddIndividualInstance(FEEntity* Entity, glm::mat4 InstanceMatrix)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
	{
		FEEntity* EntityWithGameModel = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
		if (EntityWithGameModel == nullptr)
			continue;

		FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
		AddInstanceInternal(Entity, GameModelComponent, glm::inverse(TransformComponent.GetWorldMatrix()) * InstanceMatrix, i);

		if (InstancedComponent.IndividualInstancedAABB.empty())
		{
			UpdateIndividualSelectModeAABBData(GameModelComponent, InstancedComponent);
		}
		else
		{
			FEAABB NewAABB;
			for (size_t j = 0; j < InstancedComponent.InstancedElementsData.size(); j++)
			{
				FEEntity* EntityWithGameModel = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[j]->EntityIDWithGameModelComponent);
				if (EntityWithGameModel == nullptr)
					continue;

				FEGameModelComponent& LocalGameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
				
				NewAABB = NewAABB.Merge(FEAABB(LocalGameModelComponent.GetGameModel()->GetMesh()->GetAABB(), InstancedComponent.InstancedElementsData[j]->TransformedInstancedMatrices.back()));
			}

			InstancedComponent.IndividualInstancedAABB.push_back(NewAABB/*FEAABB(GameModelComponent.GetGameModel()->GetMesh()->GetAABB(), InstancedComponent.TransformedInstancedMatrices.back())*/);
		}
	}

	InstancedComponent.Modifications.push_back(FEInstanceModification(FE_CHANGE_ADDED, static_cast<int>(InstancedComponent.InstanceCount), InstanceMatrix));
	InstancedComponent.bDirtyFlag = true;
}

// Not elegant solution, I need third system to handle this.
#include "../SubSystems/Scene/Components/Systems/FETerrainSystem.h"
bool FEInstancedSystem::TryToSnapIndividualInstance(FEEntity* Entity, size_t InstanceIndex)
{
	if (Entity == nullptr)
		return false;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return false;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	FEGameModelComponent& GameModelComponent = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[0]->EntityIDWithGameModelComponent)->GetComponent<FEGameModelComponent>();
	
	if (InstanceIndex < 0 || InstanceIndex >= InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices.size() || InstancedComponent.TerrainToSnap == nullptr)
		return false;

	if (!IsIndividualSelectMode(InstancedComponent))
		return false;

	const float Y = TERRAIN_SYSTEM.GetHeightAt(InstancedComponent.TerrainToSnap, glm::vec2(InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][3][0], InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][3][2]));
	if (Y == -FLT_MAX)
		return false;

	if (InstancedComponent.TerrainLayer != -1)
	{
		FETerrainComponent& TerrainComponent = InstancedComponent.TerrainToSnap->GetComponent<FETerrainComponent>();
		const float LayerIntensity = TERRAIN_SYSTEM.GetLayerIntensityAt(InstancedComponent.TerrainToSnap, glm::vec2(InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][3][0], InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][3][2]), InstancedComponent.TerrainLayer);
		if (LayerIntensity < InstancedComponent.MinLayerIntensityToSpawn)
			return false;
	}

	if (abs(InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][3][1] - Y) < 0.01f)
		return true;

	glm::mat4 Copy = InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex];
	Copy[3][1] = Y;
	ModifyIndividualInstance(Entity, InstanceIndex, Copy);
	InstancedComponent.bDirtyFlag = true;
	return true;
}

void FEInstancedSystem::DeleteIndividualInstance(FEEntity* Entity, const size_t InstanceIndex)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	if (InstanceIndex < 0 || InstanceIndex >= InstancedComponent.InstancedElementsData[0]->InstancedMatrices.size())
		return;

	InstancedComponent.Modifications.push_back(FEInstanceModification(FE_CHANGE_DELETED, static_cast<int>(InstanceIndex), glm::mat4()));
	InstancedComponent.InstanceCount--;
	


	for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
	{
		FEEntity* EntityWithGameModel = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
		if (EntityWithGameModel == nullptr)
			continue;

		FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
		
		InstancedComponent.InstancedElementsData[i]->InstancedAABBSizes.erase(InstancedComponent.InstancedElementsData[i]->InstancedAABBSizes.begin() + InstanceIndex);
		InstancedComponent.InstancedElementsData[i]->InstancedMatrices.erase(InstancedComponent.InstancedElementsData[i]->InstancedMatrices.begin() + InstanceIndex);
		InstancedComponent.InstancedElementsData[i]->TransformedInstancedMatrices.erase(InstancedComponent.InstancedElementsData[i]->TransformedInstancedMatrices.begin() + InstanceIndex);
		InstancedComponent.InstancedElementsData[i]->InstancePositions.erase(InstancedComponent.InstancedElementsData[i]->InstancePositions.begin() + InstanceIndex);

		for (size_t j = 0; j < GameModelComponent.GetGameModel()->GetMaxLODCount(); j++)
		{
			InstancedComponent.InstancedElementsData[i]->InstancedMatricesLOD[j].resize(InstancedComponent.InstanceCount);
		}
	}

	if (InstancedComponent.IndividualInstancedAABB.empty())
	{
		UpdateIndividualSelectModeAABBData(Entity);
	}
	else
	{
		InstancedComponent.IndividualInstancedAABB.erase(InstancedComponent.IndividualInstancedAABB.begin() + InstanceIndex);
	}

	InstancedComponent.bDirtyFlag = true;
}

void FEInstancedSystem::ModifyIndividualInstance(FEEntity* Entity, const size_t InstanceIndex, glm::mat4 NewMatrix)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	if (InstanceIndex < 0 || InstanceIndex >= InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices.size())
		return;

	if (glm::all(glm::epsilonEqual(InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][0], NewMatrix[0], 0.001f)) &&
		glm::all(glm::epsilonEqual(InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][1], NewMatrix[1], 0.001f)) &&
		glm::all(glm::epsilonEqual(InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][2], NewMatrix[2], 0.001f)) &&
		glm::all(glm::epsilonEqual(InstancedComponent.InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex][3], NewMatrix[3], 0.001f)))
		return;

	if (!InstancedComponent.Modifications.empty() && InstancedComponent.Modifications.back().Index == InstanceIndex && InstancedComponent.Modifications.back().Type == FE_CHANGE_MODIFIED)
	{
		InstancedComponent.Modifications.back().Modification = NewMatrix;
	}
	else
	{
		InstancedComponent.Modifications.push_back(FEInstanceModification(FE_CHANGE_MODIFIED, static_cast<int>(InstanceIndex), NewMatrix));
	}

	for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
	{
		FEEntity* EntityWithGameModel = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
		if (EntityWithGameModel == nullptr)
			continue;

		FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
		
		InstancedComponent.InstancedElementsData[i]->TransformedInstancedMatrices[InstanceIndex] = NewMatrix;
		InstancedComponent.InstancedElementsData[i]->InstancedMatrices[InstanceIndex] = glm::inverse(TransformComponent.GetWorldMatrix()) * NewMatrix;

		if (InstancedComponent.IndividualInstancedAABB.size() > InstanceIndex)
			InstancedComponent.IndividualInstancedAABB[InstanceIndex] = FEAABB(GameModelComponent.GetGameModel()->GetMesh()->GetAABB(), NewMatrix);
		InstancedComponent.InstancedElementsData[i]->InstancedAABBSizes[InstanceIndex] = -FEAABB(GameModelComponent.GetGameModel()->GetMesh()->GetAABB(), NewMatrix).GetLongestAxisLength();

	}

	InstancedComponent.bDirtyFlag = true;
}

void FEInstancedSystem::Update()
{
	for (int i = 0; i < EnitityIDListToInitialize.size(); i++)
	{
		std::string SceneID = EnitityIDListToInitialize[i].first;
		std::string EntityID = EnitityIDListToInitialize[i].second;

		FEScene* Scene = SCENE_MANAGER.GetScene(SceneID);
		if (Scene == nullptr)
		{
			EnitityIDListToInitialize.erase(EnitityIDListToInitialize.begin() + i);
			i--;
			continue;
		}

		FEEntity* EntityToWorkWith = Scene->GetEntity(EntityID);
		if (EntityToWorkWith == nullptr)
		{
			EnitityIDListToInitialize.erase(EnitityIDListToInitialize.begin() + i);
			i--;
			continue;
		}

		if (!EntityToWorkWith->HasComponent<FEInstancedComponent>())
		{
			EnitityIDListToInitialize.erase(EnitityIDListToInitialize.begin() + i);
			i--;
			continue;
		}

		FEInstancedComponent& InstancedComponent = EntityToWorkWith->GetComponent<FEInstancedComponent>();

		if (InstancedComponent.TerrainToSnap == nullptr && !InstancedComponent.PostponedTerrainToSnapID.empty())
		{
			FEEntity* TerrainEntity = Scene->GetEntity(InstancedComponent.PostponedTerrainToSnapID);
			if (TerrainEntity != nullptr)
			{
				TERRAIN_SYSTEM.SnapInstancedEntity(TerrainEntity, EntityToWorkWith);

				if (InstancedComponent.PostponedTerrainLayer != -1)
					TERRAIN_SYSTEM.ConnectInstancedEntityToLayer(TerrainEntity, EntityToWorkWith, InstancedComponent.PostponedTerrainLayer);
			}
		}

		PopulateInstance(EntityToWorkWith, InstancedComponent.SpawnInfo);

		if (!InstancedComponent.PostponedModificationsData.isNull())
		{
			Json::Value EntityFileRoot = InstancedComponent.PostponedModificationsData;

			size_t Count = EntityFileRoot.size();
			for (int j = 0; j < Count; j++)
			{
				if (EntityFileRoot[j]["Type"].asInt() == FE_CHANGE_DELETED)
				{
					INSTANCED_RENDERING_SYSTEM.DeleteIndividualInstance(EntityToWorkWith, EntityFileRoot[j]["Index"].asInt());
				}
				else if (EntityFileRoot[j]["Type"].asInt() == FE_CHANGE_MODIFIED)
				{
					glm::mat4 ModifedMatrix;
					for (int k = 0; k < 4; k++)
					{
						for (int p = 0; p < 4; p++)
						{
							ModifedMatrix[k][p] = EntityFileRoot[j]["Modification"][k][p].asFloat();
						}
					}

					INSTANCED_RENDERING_SYSTEM.ModifyIndividualInstance(EntityToWorkWith, EntityFileRoot[j]["Index"].asInt(), ModifedMatrix);
				}
				else if (EntityFileRoot[j]["Type"].asInt() == FE_CHANGE_ADDED)
				{
					glm::mat4 ModifedMatrix;
					for (int k = 0; k < 4; k++)
					{
						for (int p = 0; p < 4; p++)
						{
							ModifedMatrix[k][p] = EntityFileRoot[j]["Modification"][k][p].asFloat();
						}
					}

					INSTANCED_RENDERING_SYSTEM.AddIndividualInstance(EntityToWorkWith, ModifedMatrix);
				}
			}
		}

		EnitityIDListToInitialize.erase(EnitityIDListToInitialize.begin() + i);
		i--;
	}
}

void FEInstancedSystem::ClearInstance(FEEntity* Entity)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	InstancedComponent.Clear();

	for (size_t i = 0; i < InstancedComponent.InstancedElementsData.size(); i++)
	{
		FEEntity* EntityWithGameModel = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[i]->EntityIDWithGameModelComponent);
		if (EntityWithGameModel == nullptr)
			continue;

		FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
		InstancedComponent.InstancedElementsData[i]->LODCounts = new int[GameModelComponent.GetGameModel()->GetMaxLODCount()];
		for (size_t i = 0; i < GameModelComponent.GetGameModel()->GetMaxLODCount(); i++)
			InstancedComponent.InstancedElementsData[i]->LODCounts[i] = 0;

		InstancedComponent.InstancedElementsData[i]->InstancedMatricesLOD.resize(GameModelComponent.GetGameModel()->GetMaxLODCount());
	}

	Entity->GetComponent<FETransformComponent>().SetScale(glm::vec3(1.0f));
}

bool FEInstancedSystem::PopulateInstance(FEEntity* Entity, FESpawnInfo SpawnInfo)
{
	if (Entity == nullptr)
		return false;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return false;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	FEEntity* GameModelEntity = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[0]->EntityIDWithGameModelComponent);
	if (GameModelEntity == nullptr)
		return false;

	FEGameModelComponent& GameModelComponent = GameModelEntity->GetComponent<FEGameModelComponent>();
	return PopulateInstanceInternal(Entity, GameModelComponent, SpawnInfo);
}

bool FEInstancedSystem::PopulateInstanceInternal(FEEntity* Entity, FEGameModelComponent& GameModelComponent, FESpawnInfo SpawnInfo)
{
	if (SpawnInfo.Radius <= 0.0f || SpawnInfo.ObjectCount < 1 || SpawnInfo.ObjectCount > 1000000 || GameModelComponent.GetGameModel() == nullptr)
		return false;

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	InstancedComponent.SpawnInfo = SpawnInfo;
	srand(SpawnInfo.Seed);

	const glm::vec3 Min = GameModelComponent.GetGameModel()->GetMesh()->GetAABB().GetMin();
	const glm::vec3 Max = GameModelComponent.GetGameModel()->GetMesh()->GetAABB().GetMax();

	float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	YSize *= GameModelComponent.GetGameModel()->GetScaleFactor();

	std::vector<glm::mat4> NewMats;
	NewMats.resize(SpawnInfo.ObjectCount);

	glm::vec3 Position = TransformComponent.GetPosition();

	for (size_t i = 0; i < NewMats.size(); i++)
	{
		glm::mat4 NewMat = glm::mat4(1.0);
		// spawner transformation would be taken in account later so consider center in 0
		float X = SpawnInfo.GetPositionDeviation();
		float Z = SpawnInfo.GetPositionDeviation();
		float Y = SpawnInfo.GetPositionDeviation();

		if (InstancedComponent.TerrainToSnap != nullptr)
		{
			Y = TERRAIN_SYSTEM.GetHeightAt(InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z));

			if (InstancedComponent.TerrainLayer != -1 && Y != -FLT_MAX)
			{
				FETerrainComponent& TerrainComponent = InstancedComponent.TerrainToSnap->GetComponent<FETerrainComponent>();
				const float LayerIntensity = TERRAIN_SYSTEM.GetLayerIntensityAt(InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z), InstancedComponent.TerrainLayer);
				if (LayerIntensity < InstancedComponent.MinLayerIntensityToSpawn)
					Y = -FLT_MAX;
			}

			int CountOfTries = 0;
			while (Y == -FLT_MAX)
			{
				X = SpawnInfo.GetPositionDeviation();
				Z = SpawnInfo.GetPositionDeviation();
				Y = TERRAIN_SYSTEM.GetHeightAt(InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z));

				if (InstancedComponent.TerrainLayer != -1 && Y != -FLT_MAX)
				{
					FETerrainComponent& TerrainComponent = InstancedComponent.TerrainToSnap->GetComponent<FETerrainComponent>();
					const float LayerIntensity = TERRAIN_SYSTEM.GetLayerIntensityAt(InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z), InstancedComponent.TerrainLayer);
					if (LayerIntensity < InstancedComponent.MinLayerIntensityToSpawn)
						Y = -FLT_MAX;
				}

				CountOfTries++;
				if (CountOfTries > 300)
					break;
			}

			if (CountOfTries > 300)
			{
				Y = Position.y + SpawnInfo.GetPositionDeviation();
			}
		}

		NewMat = glm::translate(NewMat, glm::vec3(X, Y, Z));

		NewMat = glm::rotate(NewMat, SpawnInfo.GetRotaionDeviation(glm::vec3(1, 0, 0)) * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
		NewMat = glm::rotate(NewMat, SpawnInfo.GetRotaionDeviation(glm::vec3(0, 1, 0)) * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
		NewMat = glm::rotate(NewMat, SpawnInfo.GetRotaionDeviation(glm::vec3(0, 0, 1)) * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));

		float FinalScale = GameModelComponent.GetGameModel()->GetScaleFactor() + GameModelComponent.GetGameModel()->GetScaleFactor() * SpawnInfo.GetScaleDeviation();
		if (FinalScale < 0.0f)
			FinalScale = 0.01f;
		NewMat = glm::scale(NewMat, glm::vec3(FinalScale));

		NewMats[i] = NewMat;
	}
	AddInstances(Entity, NewMats.data(), NewMats.size());

	if (InstancedComponent.TerrainToSnap != nullptr)
	{
		// terrain.Y could be not 0.0f but here we should indicate 0.0f as Y.
		TransformComponent.SetPosition(glm::vec3(Position.x, 0.0f, Position.z));
	}

	srand(static_cast<unsigned>(time(nullptr)));

	InstancedComponent.bDirtyFlag = true;
	return true;
}

void FEInstancedSystem::CheckDirtyFlag(FEEntity* Entity)
{
	if (Entity == nullptr)
		return;

	if (!Entity->HasComponent<FEGameModelComponent>() && !Entity->HasComponent<FEPrefabInstanceComponent>())
		return;

	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	size_t CurrentBufferIndex = 0;
	if (Entity->HasComponent<FEPrefabInstanceComponent>())
	{
		FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
		FEScene* PrefabScene = PrefabInstanceComponent.GetPrefab()->GetScene();

		std::vector<std::string> AllPrefabEntities = PrefabScene->GetEntityIDListWith<FEGameModelComponent>();
		for (size_t i = 0; i < AllPrefabEntities.size(); i++)
		{
			FEEntity* CurrentPrefabEntity = PrefabScene->GetEntity(AllPrefabEntities[i]);
			if (CurrentPrefabEntity != nullptr)
			{
				FEGameModelComponent& GameModelComponent = CurrentPrefabEntity->GetComponent<FEGameModelComponent>();
				if (InstancedComponent.InstancedElementsData[CurrentBufferIndex]->LastFrameGameModel != GameModelComponent.GetGameModel() || GameModelComponent.GetGameModel()->IsDirty())
				{
					InstancedComponent.bDirtyFlag = true;
					InstancedComponent.InstancedElementsData[CurrentBufferIndex]->LastFrameGameModel = GameModelComponent.GetGameModel();
					break;
				}
				CurrentBufferIndex++;
			}
		}
	}
	else
	{
		FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
		if (InstancedComponent.InstancedElementsData[CurrentBufferIndex]->LastFrameGameModel != GameModelComponent.GetGameModel() || GameModelComponent.GetGameModel()->IsDirty())
		{
			InstancedComponent.bDirtyFlag = true;
			InstancedComponent.InstancedElementsData[CurrentBufferIndex]->LastFrameGameModel = GameModelComponent.GetGameModel();
		}
	}

	FETransformComponent& TransformComponent = Entity->GetComponent<FETransformComponent>();

	if (TransformComponent.IsDirty())
		InstancedComponent.bDirtyFlag = true;

	if (InstancedComponent.bDirtyFlag)
	{
		UpdateBuffers(Entity);
		InstancedComponent.bDirtyFlag = false;
		InitializeGPUCullingBuffers(Entity);
		// It is not correct to set dirty flags here.
		//GameModelComponent.GetGameModel()->SetDirtyFlag(false);



		if (Entity->HasComponent<FEPrefabInstanceComponent>())
		{
			FEPrefabInstanceComponent& PrefabInstanceComponent = Entity->GetComponent<FEPrefabInstanceComponent>();
			FEScene* PrefabScene = PrefabInstanceComponent.GetPrefab()->GetScene();

			std::vector<std::string> AllPrefabEntities = PrefabScene->GetEntityIDListWith<FEGameModelComponent>();
			for (size_t i = 0; i < AllPrefabEntities.size(); i++)
			{
				FEEntity* CurrentPrefabEntity = PrefabScene->GetEntity(AllPrefabEntities[i]);
				if (CurrentPrefabEntity != nullptr)
				{
					FEGameModelComponent& GameModelComponent = CurrentPrefabEntity->GetComponent<FEGameModelComponent>();
					GameModelComponent.GetGameModel()->SetDirtyFlag(false);
				}
			}
		}
		else
		{
			FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
			GameModelComponent.GetGameModel()->SetDirtyFlag(false);
		}
	}

	if (TransformComponent.IsDirty())
	{
		TransformComponent.SetDirtyFlag(false);
		UpdateMatrices(Entity);
	}
}

void FEInstancedSystem::UpdateIndividualSelectModeAABBData(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	UpdateIndividualSelectModeAABBData(GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::UpdateIndividualSelectModeAABBData(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	InstancedComponent.IndividualInstancedAABB.clear();
	InstancedComponent.IndividualInstancedAABB.resize(InstancedComponent.InstanceCount);

	for (size_t i = 0; i < InstancedComponent.InstanceCount; i++)
	{
		for (size_t j = 0; j < InstancedComponent.InstancedElementsData.size(); j++)
		{
			FEEntity* EntityWithGameModel = GetEntityWithGameModelComponent(InstancedComponent.InstancedElementsData[j]->EntityIDWithGameModelComponent);
			if (EntityWithGameModel == nullptr)
				continue;

			FEGameModelComponent& GameModelComponent = EntityWithGameModel->GetComponent<FEGameModelComponent>();
			InstancedComponent.IndividualInstancedAABB[i] = InstancedComponent.IndividualInstancedAABB[i].Merge(FEAABB(GameModelComponent.GetGameModel()->GetMesh()->GetAABB(), InstancedComponent.InstancedElementsData[j]->TransformedInstancedMatrices[i]));
		}
	}

	InstancedComponent.bDirtyFlag = true;
}

bool FEInstancedSystem::IsIndividualSelectMode(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return false;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return false;

	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	return IsIndividualSelectMode(InstancedComponent);
}

bool FEInstancedSystem::IsIndividualSelectMode(FEInstancedComponent& InstancedComponent)
{
	return InstancedComponent.bSelectionMode;
}

void FEInstancedSystem::SetIndividualSelectMode(FEEntity* EntityWithInstancedComponent, const bool NewValue)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	SetIndividualSelectMode(GameModelComponent, InstancedComponent, NewValue);
}

void FEInstancedSystem::SetIndividualSelectMode(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const bool NewValue)
{
	if (NewValue)
		UpdateIndividualSelectModeAABBData(GameModelComponent, InstancedComponent);

	InstancedComponent.bSelectionMode = NewValue;
}

Json::Value FEInstancedSystem::InstanceComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	if (Entity == nullptr || !Entity->HasComponent<FEInstancedComponent>())
	{
		LOG.Add("FEInstancedSystem::InstanceComponentToJson Entity is nullptr or does not have FEInstancedComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return Root;
	}
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	Root["Modifications to spawn"] = InstancedComponent.GetSpawnModificationCount() == 0 ? false : true;
	if (InstancedComponent.GetSpawnModificationCount())
	{
		Json::Value ModificationsData;
		auto ModificationList = InstancedComponent.GetSpawnModifications();
		for (int j = 0; j < ModificationList.size(); j++)
		{
			ModificationsData[j]["Type"] = static_cast<int>(ModificationList[j].Type);
			ModificationsData[j]["Index"] = ModificationList[j].Index;
			if (ModificationList[j].Type != FE_CHANGE_DELETED)
			{
				for (int k = 0; k < 4; k++)
				{
					for (int p = 0; p < 4; p++)
					{
						ModificationsData[j]["Modification"][k][p] = ModificationList[j].Modification[k][p];
					}
				}
			}
		}
		Root["Modifications"] = ModificationsData;
	}

	Root["Spawn info"]["Seed"] = InstancedComponent.SpawnInfo.Seed;
	Root["Spawn info"]["Object count"] = InstancedComponent.SpawnInfo.ObjectCount;
	Root["Spawn info"]["Radius"] = InstancedComponent.SpawnInfo.Radius;
	Root["Spawn info"]["Min scale"] = InstancedComponent.SpawnInfo.GetMinScale();
	Root["Spawn info"]["Max scale"] = InstancedComponent.SpawnInfo.GetMaxScale();
	Root["Spawn info"]["RotationDeviation"]["X"] = InstancedComponent.SpawnInfo.RotationDeviation.x;
	Root["Spawn info"]["RotationDeviation"]["Y"] = InstancedComponent.SpawnInfo.RotationDeviation.y;
	Root["Spawn info"]["RotationDeviation"]["Z"] = InstancedComponent.SpawnInfo.RotationDeviation.z;

	if (InstancedComponent.GetSnappedToTerrain() == nullptr)
	{
		Root["Snapped to terrain ID"] = "none";
	}
	else
	{
		Root["Snapped to terrain ID"] = InstancedComponent.GetSnappedToTerrain()->GetObjectID();
		Root["Terrain layer"] = InstancedComponent.GetTerrainLayer();
		Root["Min layer intensity to spawn"] = InstancedComponent.GetMinimalLayerIntensityToSpawn();
	}

	return Root;
}

void FEInstancedSystem::InstanceComponentFromJson(FEEntity* Entity, Json::Value Data)
{
	if (Entity == nullptr)
	{
		LOG.Add("FEInstancedSystem::InstanceComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}
	Entity->AddComponent<FEInstancedComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();

	InstancedComponent.SpawnInfo.Seed = Data["Spawn info"]["Seed"].asInt();
	InstancedComponent.SpawnInfo.ObjectCount = Data["Spawn info"]["Object count"].asInt();
	InstancedComponent.SpawnInfo.Radius = Data["Spawn info"]["Radius"].asFloat();
	InstancedComponent.SpawnInfo.SetMinScale(Data["Spawn info"]["Min scale"].asFloat());
	InstancedComponent.SpawnInfo.SetMaxScale(Data["Spawn info"]["Max scale"].asFloat());
	InstancedComponent.SpawnInfo.RotationDeviation.x = Data["Spawn info"]["RotationDeviation"]["X"].asFloat();
	InstancedComponent.SpawnInfo.RotationDeviation.y = Data["Spawn info"]["RotationDeviation"]["Y"].asFloat();
	InstancedComponent.SpawnInfo.RotationDeviation.z = Data["Spawn info"]["RotationDeviation"]["Z"].asFloat();

	if (Data["Snapped to terrain ID"].asString() != "none")
	{
		InstancedComponent.PostponedTerrainToSnapID = Data["Snapped to terrain ID"].asString();
		InstancedComponent.TerrainToSnap = nullptr;

		if (Data.isMember("Terrain layer"))
		{
			InstancedComponent.PostponedTerrainLayer = Data["Terrain layer"].asInt();
			InstancedComponent.SetMinimalLayerIntensityToSpawn(Data["Min layer intensity to spawn"].asFloat());
		}
	}

	// Postponing this operation when loading is finished.
	INSTANCED_RENDERING_SYSTEM.EnitityIDListToInitialize.push_back(std::make_pair(Entity->GetParentScene()->GetObjectID(), Entity->GetObjectID()));

	if (Data["Modifications to spawn"].asBool())
	{
		InstancedComponent.PostponedModificationsData = Data["Modifications"];
	}
}