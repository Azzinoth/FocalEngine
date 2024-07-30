#include "FEInstancedSystem.h"
using namespace FocalEngine;

FEInstancedSystem* FEInstancedSystem::Instance = nullptr;
FEInstancedSystem::FEInstancedSystem()
{
	RegisterOnComponentCallbacks();
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

	if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>() || !Entity->HasComponent<FEInstancedComponent>())
		return;

	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	INSTANCED_RENDERING_SYSTEM.InitializeBuffers(GameModelComponent, InstancedComponent);
}

// There should be third system to manage that.
#include "../SubSystems/Scene/Components/Systems/FETerrainSystem.h"
void FEInstancedSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (bIsSceneClearing)
		return;

	if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>() || !Entity->HasComponent<FEInstancedComponent>())
		return;

	FEGameModelComponent& GameModelComponent = Entity->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = Entity->GetComponent<FEInstancedComponent>();
	if (InstancedComponent.TerrainToSnap != nullptr)
	{
		TERRAIN_SYSTEM.UnSnapInstancedEntity(InstancedComponent.TerrainToSnap, Entity);
	}
}

FEInstancedSystem::~FEInstancedSystem() {};

void FEInstancedSystem::InitializeBuffers(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	InstancedComponent.LODCounts = new int[GameModelComponent.GameModel->GetMaxLODCount()];

	InstancedComponent.LODBuffers = new GLenum[GameModelComponent.GameModel->GetMaxLODCount()];
	for (size_t i = 0; i < GameModelComponent.GameModel->GetMaxLODCount(); i++)
	{
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.LODBuffers[i]));
	}

	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.SourceDataBuffer));
	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.PositionsBuffer));
	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.AABBSizesBuffer));
	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.LODInfoBuffer));

	InstancedComponent.IndirectDrawsInfo = new FEDrawElementsIndirectCommand[4];
	for (size_t i = 0; i < GameModelComponent.GameModel->GetMaxLODCount(); i++)
	{
		InstancedComponent.IndirectDrawsInfo[i].Count = GameModelComponent.GameModel->GetLODMesh(i) == nullptr ? 0 : GameModelComponent.GameModel->GetLODMesh(i)->GetVertexCount();
		InstancedComponent.IndirectDrawsInfo[i].BaseInstance = 0;
		InstancedComponent.IndirectDrawsInfo[i].BaseVertex = 0;
		InstancedComponent.IndirectDrawsInfo[i].FirstIndex = 0;
		InstancedComponent.IndirectDrawsInfo[i].PrimCount = 0;
	}

	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.IndirectDrawInfoBuffer));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, InstancedComponent.IndirectDrawInfoBuffer));
	FE_GL_ERROR(glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(FEDrawElementsIndirectCommand) * 4, InstancedComponent.IndirectDrawsInfo, GL_MAP_READ_BIT));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

	InstancedComponent.InstancedMatricesLOD.resize(GameModelComponent.GameModel->GetMaxLODCount());
}

void FEInstancedSystem::InitializeGPUCullingBuffers(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	if (InstancedComponent.SourceDataBuffer != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.SourceDataBuffer));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.SourceDataBuffer));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, InstancedComponent.SourceDataBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), InstancedComponent.TransformedInstancedMatrices.data(), GL_DYNAMIC_DRAW));

	if (InstancedComponent.PositionsBuffer != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.PositionsBuffer));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.PositionsBuffer));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, InstancedComponent.PositionsBuffer));

	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(float) * 3, InstancedComponent.InstancePositions.data(), GL_DYNAMIC_DRAW));

	if (InstancedComponent.LODBuffers[0] != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.LODBuffers[0]));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.LODBuffers[0]));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, InstancedComponent.LODBuffers[0]));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

	if (InstancedComponent.LODBuffers[1] != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.LODBuffers[1]));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.LODBuffers[1]));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, InstancedComponent.LODBuffers[1]));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

	if (InstancedComponent.LODBuffers[2] != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.LODBuffers[2]));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.LODBuffers[2]));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, InstancedComponent.LODBuffers[2]));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

	if (InstancedComponent.LODBuffers[3] != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.LODBuffers[3]));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.LODBuffers[3]));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, InstancedComponent.LODBuffers[3]));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

	if (InstancedComponent.AABBSizesBuffer != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.AABBSizesBuffer));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.AABBSizesBuffer));
	}
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, InstancedComponent.AABBSizesBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(float), InstancedComponent.InstancedAABBSizes.data(), GL_DYNAMIC_DRAW));

	std::vector<float> LODInfoData;
	LODInfoData.push_back(GameModelComponent.GameModel->GetCullDistance());
	LODInfoData.push_back(GameModelComponent.GameModel->GetLODMaxDrawDistance(0));
	LODInfoData.push_back(GameModelComponent.GameModel->GetLODMaxDrawDistance(1));
	LODInfoData.push_back(GameModelComponent.GameModel->GetLODMaxDrawDistance(2));

	// does it have billboard ?
	unsigned int BillboardIndex = 5;
	for (size_t j = 0; j < GameModelComponent.GameModel->GetMaxLODCount(); j++)
	{
		if (GameModelComponent.GameModel->IsLODBillboard(j) && GameModelComponent.GameModel->GetLODMesh(j) != nullptr)
		{
			BillboardIndex = static_cast<int>(j);
		}
	}

	LODInfoData.push_back(static_cast<float>(BillboardIndex));
	// This should not be here, instead normal of plane should align with vector to camera.
	LODInfoData.push_back(1.5708f * 3.0f + GameModelComponent.GameModel->GetBillboardZeroRotaion() * ANGLE_TORADIANS_COF);
	LODInfoData.push_back(static_cast<float>(InstancedComponent.InstanceCount));

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, InstancedComponent.LODInfoBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, 7 * sizeof(float), LODInfoData.data(), GL_DYNAMIC_DRAW));

	if (InstancedComponent.IndirectDrawsInfo == nullptr)
		InstancedComponent.IndirectDrawsInfo = new FEDrawElementsIndirectCommand[4];

	for (size_t j = 0; j < GameModelComponent.GameModel->GetMaxLODCount(); j++)
	{
		InstancedComponent.IndirectDrawsInfo[j].Count = GameModelComponent.GameModel->GetLODMesh(j) == nullptr ? 0 : GameModelComponent.GameModel->GetLODMesh(j)->GetVertexCount();
		InstancedComponent.IndirectDrawsInfo[j].BaseInstance = 0;
		InstancedComponent.IndirectDrawsInfo[j].BaseVertex = 0;
		InstancedComponent.IndirectDrawsInfo[j].FirstIndex = 0;
		InstancedComponent.IndirectDrawsInfo[j].PrimCount = 0;
	}

	if (InstancedComponent.IndirectDrawInfoBuffer != 0)
	{
		FE_GL_ERROR(glDeleteBuffers(1, &InstancedComponent.IndirectDrawInfoBuffer));
		FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.IndirectDrawInfoBuffer));
	}

	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, InstancedComponent.IndirectDrawInfoBuffer));
	FE_GL_ERROR(glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(FEDrawElementsIndirectCommand) * 4, InstancedComponent.IndirectDrawsInfo, GL_MAP_READ_BIT));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
}

void FEInstancedSystem::UpdateBuffers(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	UpdateBuffers(TransformComponent, GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::UpdateBuffers(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	if (InstancedComponent.InstancedBuffer != 0)
	{
		glDeleteBuffers(1, &InstancedComponent.InstancedBuffer);
	}

	FE_GL_ERROR(glGenBuffers(1, &InstancedComponent.InstancedBuffer));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, InstancedComponent.InstancedBuffer));
	FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), InstancedComponent.InstancedMatrices.data(), GL_DYNAMIC_DRAW));

	const unsigned int VAO = GameModelComponent.GameModel->Mesh->GetVaoID();
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

	for (size_t i = 0; i < GameModelComponent.GameModel->GetMaxLODCount(); i++)
	{
		if (GameModelComponent.GameModel->GetLODMesh(i) != nullptr)
		{
			const unsigned int VAO = GameModelComponent.GameModel->GetLODMesh(i)->GetVaoID();
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

	InstancedComponent.AllInstancesAABB = FEAABB();
	glm::vec3 Position = TransformComponent.GetPosition();
	for (size_t i = 0; i < InstancedComponent.InstanceCount; i++)
	{
		glm::mat4 MatWithoutTranslate = InstancedComponent.TransformedInstancedMatrices[i];
		MatWithoutTranslate[3][0] -= Position.x;
		MatWithoutTranslate[3][1] -= Position.y;
		MatWithoutTranslate[3][2] -= Position.z;

		InstancedComponent.AllInstancesAABB = InstancedComponent.AllInstancesAABB.Merge(GameModelComponent.GameModel->Mesh->AABB.Transform(MatWithoutTranslate));
	}
	
	TransformComponent.SetDirtyFlag(true);
	GetAABB(TransformComponent, GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::DuplicateInstancedComponent(FEEntity* EntityWithInstancedComponent, FEEntity* NewEntity)
{
	if (EntityWithInstancedComponent == nullptr || NewEntity == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>() || !NewEntity->HasComponent<FEGameModelComponent>())
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEInstancedComponent>())
		return;

	FEInstancedComponent& OriginalInstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	bInternalAdd = true;
	NewEntity->AddComponent<FEInstancedComponent>();
	bInternalAdd = false;

	FEGameModelComponent& GameModelComponent = NewEntity->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& NewInstancedComponent = NewEntity->GetComponent<FEInstancedComponent>();
	NewInstancedComponent = OriginalInstancedComponent;
	INSTANCED_RENDERING_SYSTEM.InitializeBuffers(GameModelComponent, NewInstancedComponent);
}

void FEInstancedSystem::AddInstanceInternal(FEEntity* EntityWithInstancedComponent, const glm::mat4 InstanceMatrix)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	AddInstanceInternal(TransformComponent, GameModelComponent, InstancedComponent, InstanceMatrix);
}

void FEInstancedSystem::AddInstanceInternal(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, glm::mat4 InstanceMatrix)
{
	InstancedComponent.InstancedAABBSizes.push_back(-FEAABB(GameModelComponent.GameModel->GetMesh()->GetAABB(), InstanceMatrix).GetLongestAxisLength());
	InstancedComponent.InstancedMatrices.push_back(InstanceMatrix);
	InstancedComponent.TransformedInstancedMatrices.push_back(TransformComponent.GetWorldMatrix() * InstanceMatrix);
	InstancedComponent.InstancePositions.push_back(InstancedComponent.TransformedInstancedMatrices.back()[3]);

	for (size_t j = 0; j < GameModelComponent.GameModel->GetMaxLODCount(); j++)
	{
		InstancedComponent.InstancedMatricesLOD[j].resize(InstancedComponent.InstanceCount);
	}

	InstancedComponent.InstanceCount++;
	InstancedComponent.bDirtyFlag = true;
}

void FEInstancedSystem::AddInstances(FEEntity* EntityWithInstancedComponent, const glm::mat4* InstanceMatrix, size_t Count)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	AddInstances(TransformComponent, GameModelComponent, InstancedComponent, InstanceMatrix, Count);
}

void FEInstancedSystem::AddInstances(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const glm::mat4* InstanceMatrix, size_t Count)
{
	const size_t StartIndex = InstancedComponent.InstancedAABBSizes.size();

	InstancedComponent.InstancedAABBSizes.resize(InstancedComponent.InstancedAABBSizes.size() + Count);
	const FEAABB OriginalAABB = GameModelComponent.GameModel->GetMesh()->GetAABB();
	InstancedComponent.InstancedMatrices.resize(InstancedComponent.InstancedMatrices.size() + Count);
	InstancedComponent.TransformedInstancedMatrices.resize(InstancedComponent.TransformedInstancedMatrices.size() + Count);
	InstancedComponent.InstancePositions.resize(InstancedComponent.InstancePositions.size() + Count);

	for (size_t j = StartIndex; j < Count; j++)
	{
		InstancedComponent.InstancedAABBSizes[j] = -FEAABB(OriginalAABB, InstanceMatrix[j]).GetLongestAxisLength();
		InstancedComponent.InstancedMatrices[j] = InstanceMatrix[j];
		InstancedComponent.TransformedInstancedMatrices[j] = TransformComponent.GetWorldMatrix() * InstanceMatrix[j];

		InstancedComponent.InstancePositions[j] = InstancedComponent.TransformedInstancedMatrices[j][3];
		InstancedComponent.InstanceCount++;
	}

	for (size_t j = 0; j < GameModelComponent.GameModel->GetMaxLODCount(); j++)
	{
		InstancedComponent.InstancedMatricesLOD[j].resize(InstancedComponent.InstanceCount);
	}

	InstancedComponent.bDirtyFlag = true;
}

FEAABB FEInstancedSystem::GetAABB(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return FEAABB();

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return FEAABB();

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	return GetAABB(TransformComponent, GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::UpdateMatrices(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	UpdateMatrices(TransformComponent, GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::UpdateMatrices(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{	
	if (InstancedComponent.InstancedMatrices.size() != InstancedComponent.TransformedInstancedMatrices.size())
	{
		LOG.Add("InstancedMatrices size and TransformedInstancedMatrices size is not equal in FEInstancedSystem::UpdateMatrices", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	for (size_t i = 0; i < InstancedComponent.InstancedMatrices.size(); i++)
	{
		InstancedComponent.TransformedInstancedMatrices[i] = TransformComponent.GetWorldMatrix() * InstancedComponent.InstancedMatrices[i];
		InstancedComponent.InstancePositions[i] = InstancedComponent.TransformedInstancedMatrices[i][3];
	}

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, InstancedComponent.SourceDataBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(glm::mat4), InstancedComponent.TransformedInstancedMatrices.data(), GL_DYNAMIC_DRAW));

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, InstancedComponent.PositionsBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstancedComponent.InstanceCount * sizeof(float) * 3, InstancedComponent.InstancePositions.data(), GL_DYNAMIC_DRAW));
}

FEAABB FEInstancedSystem::GetAABB(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	FEAABB Result = InstancedComponent.AllInstancesAABB.Transform(TransformComponent.GetWorldMatrix());
	if (TransformComponent.IsDirty())
	{
		UpdateMatrices(TransformComponent, GameModelComponent, InstancedComponent);
		TransformComponent.SetDirtyFlag(false);
	}

	return Result;
}

void FEInstancedSystem::Render(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	Render(TransformComponent, GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::Render(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	if (InstancedComponent.InstanceCount == 0)
		return;

	CheckDirtyFlag(TransformComponent, GameModelComponent, InstancedComponent);

	for (size_t i = 0; i < GameModelComponent.GameModel->GetMaxLODCount(); i++)
	{
		if (GameModelComponent.GameModel->IsLODBillboard(i))
			break;

		if (GameModelComponent.GameModel->GetLODMesh(i) != nullptr)
		{
			if (InstancedComponent.LODBuffers[i] == 0)
				break;

			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, InstancedComponent.LODBuffers[i]));

			FE_GL_ERROR(glBindVertexArray(GameModelComponent.GameModel->GetLODMesh(i)->GetVaoID()));

			if ((GameModelComponent.GameModel->GetLODMesh(i)->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((GameModelComponent.GameModel->GetLODMesh(i)->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((GameModelComponent.GameModel->GetLODMesh(i)->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((GameModelComponent.GameModel->GetLODMesh(i)->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((GameModelComponent.GameModel->GetLODMesh(i)->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((GameModelComponent.GameModel->GetLODMesh(i)->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(0)));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, InstancedComponent.IndirectDrawInfoBuffer));
			FE_GL_ERROR(glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(i * sizeof(FEDrawElementsIndirectCommand))));

			FE_GL_ERROR(glBindVertexArray(0));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
		}
	}

	if (InstancedComponent.CullingType == FE_CULLING_NONE)
	{
		FE_GL_ERROR(glBindVertexArray(GameModelComponent.GameModel->Mesh->GetVaoID()));
		if ((GameModelComponent.GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((GameModelComponent.GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((GameModelComponent.GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((GameModelComponent.GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((GameModelComponent.GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((GameModelComponent.GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		FE_GL_ERROR(glEnableVertexAttribArray(6));
		FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr)));
		FE_GL_ERROR(glEnableVertexAttribArray(7));
		FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(8));
		FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(9));
		FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, GameModelComponent.GameModel->Mesh->GetVertexCount(), GL_UNSIGNED_INT, nullptr, static_cast<int>(InstancedComponent.InstanceCount)));

		FE_GL_ERROR(glBindVertexArray(0));
	}
}

void FEInstancedSystem::RenderOnlyBillbords(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	RenderOnlyBillbords(GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::RenderOnlyBillbords(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	for (size_t j = 0; j < GameModelComponent.GameModel->GetMaxLODCount(); j++)
	{
		if (GameModelComponent.GameModel->IsLODBillboard(j) && GameModelComponent.GameModel->GetLODMesh(j) != nullptr)
		{
			if (InstancedComponent.LODBuffers[j] == 0)
				break;

			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, InstancedComponent.LODBuffers[j]));

			FE_GL_ERROR(glBindVertexArray(GameModelComponent.GameModel->GetLODMesh(j)->GetVaoID()));

			if ((GameModelComponent.GameModel->GetLODMesh(j)->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((GameModelComponent.GameModel->GetLODMesh(j)->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((GameModelComponent.GameModel->GetLODMesh(j)->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((GameModelComponent.GameModel->GetLODMesh(j)->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((GameModelComponent.GameModel->GetLODMesh(j)->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((GameModelComponent.GameModel->GetLODMesh(j)->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr)));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, InstancedComponent.IndirectDrawInfoBuffer));
			FE_GL_ERROR(glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(j * sizeof(FEDrawElementsIndirectCommand))));

			FE_GL_ERROR(glBindVertexArray(0));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

			break;
		}
	}
}

void FEInstancedSystem::AddIndividualInstance(FEEntity* EntityWithInstancedComponent, glm::mat4 InstanceMatrix)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	AddIndividualInstance(TransformComponent, GameModelComponent, InstancedComponent, InstanceMatrix);
}

void FEInstancedSystem::AddIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, glm::mat4 InstanceMatrix)
{
	AddInstanceInternal(TransformComponent, GameModelComponent, InstancedComponent, glm::inverse(TransformComponent.GetWorldMatrix()) * InstanceMatrix);

	if (InstancedComponent.IndividualInstancedAABB.empty())
	{
		UpdateIndividualSelectModeAABBData(GameModelComponent, InstancedComponent);
	}
	else
	{
		InstancedComponent.IndividualInstancedAABB.push_back(FEAABB(GameModelComponent.GameModel->GetMesh()->GetAABB(), InstancedComponent.TransformedInstancedMatrices.back()));
	}

	InstancedComponent.Modifications.push_back(FEInstanceModification(FE_CHANGE_ADDED, static_cast<int>(InstancedComponent.InstanceCount), InstanceMatrix));
	InstancedComponent.bDirtyFlag = true;
}

bool FEInstancedSystem::TryToSnapIndividualInstance(FEEntity* EntityWithInstancedComponent, size_t InstanceIndex)
{
	if (EntityWithInstancedComponent == nullptr)
		return false;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return false;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	return TryToSnapIndividualInstance(TransformComponent, GameModelComponent, InstancedComponent, InstanceIndex);
}

// Not elegant solution, I need third system to handle this.
#include "../SubSystems/Scene/Components/Systems/FETerrainSystem.h"
bool FEInstancedSystem::TryToSnapIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, size_t InstanceIndex)
{
	if (InstanceIndex < 0 || InstanceIndex >= InstancedComponent.TransformedInstancedMatrices.size() || InstancedComponent.TerrainToSnap == nullptr)
		return false;

	if (!IsIndividualSelectMode(InstancedComponent))
		return false;

	const float Y = TERRAIN_SYSTEM.GetHeightAt(InstancedComponent.TerrainToSnap, glm::vec2(InstancedComponent.TransformedInstancedMatrices[InstanceIndex][3][0], InstancedComponent.TransformedInstancedMatrices[InstanceIndex][3][2]));
	if (Y == -FLT_MAX)
		return false;

	if (InstancedComponent.TerrainLayer != -1)
	{
		FETerrainComponent& TerrainComponent = InstancedComponent.TerrainToSnap->GetComponent<FETerrainComponent>();
		const float LayerIntensity = TERRAIN_SYSTEM.GetLayerIntensityAt(InstancedComponent.TerrainToSnap, glm::vec2(InstancedComponent.TransformedInstancedMatrices[InstanceIndex][3][0], InstancedComponent.TransformedInstancedMatrices[InstanceIndex][3][2]), InstancedComponent.TerrainLayer);
		if (LayerIntensity < InstancedComponent.MinLayerIntensityToSpawn)
			return false;
	}

	if (abs(InstancedComponent.TransformedInstancedMatrices[InstanceIndex][3][1] - Y) < 0.01f)
		return true;

	glm::mat4 Copy = InstancedComponent.TransformedInstancedMatrices[InstanceIndex];
	Copy[3][1] = Y;
	ModifyIndividualInstance(TransformComponent, GameModelComponent, InstancedComponent, InstanceIndex, Copy);
	InstancedComponent.bDirtyFlag =  true;
	return true;
}

void FEInstancedSystem::DeleteIndividualInstance(FEEntity* EntityWithInstancedComponent, const size_t InstanceIndex)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	DeleteIndividualInstance(GameModelComponent, InstancedComponent, InstanceIndex);
}

void FEInstancedSystem::DeleteIndividualInstance(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const size_t InstanceIndex)
{
	if (InstanceIndex < 0 || InstanceIndex >= InstancedComponent.InstancedMatrices.size())
		return;

	InstancedComponent.Modifications.push_back(FEInstanceModification(FE_CHANGE_DELETED, static_cast<int>(InstanceIndex), glm::mat4()));
	InstancedComponent.InstanceCount--;

	InstancedComponent.InstancedAABBSizes.erase(InstancedComponent.InstancedAABBSizes.begin() + InstanceIndex);
	InstancedComponent.InstancedMatrices.erase(InstancedComponent.InstancedMatrices.begin() + InstanceIndex);
	InstancedComponent.TransformedInstancedMatrices.erase(InstancedComponent.TransformedInstancedMatrices.begin() + InstanceIndex);
	InstancedComponent.InstancePositions.erase(InstancedComponent.InstancePositions.begin() + InstanceIndex);

	for (size_t i = 0; i < GameModelComponent.GameModel->GetMaxLODCount(); i++)
	{
		InstancedComponent.InstancedMatricesLOD[i].resize(InstancedComponent.InstanceCount);
	}

	if (InstancedComponent.IndividualInstancedAABB.empty())
	{
		UpdateIndividualSelectModeAABBData(GameModelComponent, InstancedComponent);
	}
	else
	{
		InstancedComponent.IndividualInstancedAABB.erase(InstancedComponent.IndividualInstancedAABB.begin() + InstanceIndex);
	}

	InstancedComponent.bDirtyFlag = true;
}

void FEInstancedSystem::ModifyIndividualInstance(FEEntity* EntityWithInstancedComponent, const size_t InstanceIndex, glm::mat4 NewMatrix)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	ModifyIndividualInstance(TransformComponent, GameModelComponent, InstancedComponent, InstanceIndex, NewMatrix);
}

void FEInstancedSystem::ModifyIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const size_t InstanceIndex, glm::mat4 NewMatrix)
{
	if (InstanceIndex < 0 || InstanceIndex >= InstancedComponent.TransformedInstancedMatrices.size())
		return;

	if (glm::all(glm::epsilonEqual(InstancedComponent.TransformedInstancedMatrices[InstanceIndex][0], NewMatrix[0], 0.001f)) &&
		glm::all(glm::epsilonEqual(InstancedComponent.TransformedInstancedMatrices[InstanceIndex][1], NewMatrix[1], 0.001f)) &&
		glm::all(glm::epsilonEqual(InstancedComponent.TransformedInstancedMatrices[InstanceIndex][2], NewMatrix[2], 0.001f)) &&
		glm::all(glm::epsilonEqual(InstancedComponent.TransformedInstancedMatrices[InstanceIndex][3], NewMatrix[3], 0.001f)))
		return;

	if (!InstancedComponent.Modifications.empty() && InstancedComponent.Modifications.back().Index == InstanceIndex && InstancedComponent.Modifications.back().Type == FE_CHANGE_MODIFIED)
	{
		InstancedComponent.Modifications.back().Modification = NewMatrix;
	}
	else
	{
		InstancedComponent.Modifications.push_back(FEInstanceModification(FE_CHANGE_MODIFIED, static_cast<int>(InstanceIndex), NewMatrix));
	}

	InstancedComponent.TransformedInstancedMatrices[InstanceIndex] = NewMatrix;
	InstancedComponent.InstancedMatrices[InstanceIndex] = glm::inverse(TransformComponent.GetWorldMatrix()) * NewMatrix;

	if (InstancedComponent.IndividualInstancedAABB.size() > InstanceIndex)
		InstancedComponent.IndividualInstancedAABB[InstanceIndex] = FEAABB(GameModelComponent.GameModel->GetMesh()->GetAABB(), NewMatrix);
	InstancedComponent.InstancedAABBSizes[InstanceIndex] = -FEAABB(GameModelComponent.GameModel->GetMesh()->GetAABB(), NewMatrix).GetLongestAxisLength();

	InstancedComponent.bDirtyFlag = true;
}

void FEInstancedSystem::Update()
{
	
}

void FEInstancedSystem::ClearInstance(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	ClearInstance(TransformComponent, GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::ClearInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	InstancedComponent.Clear();

	InstancedComponent.LODCounts = new int[GameModelComponent.GameModel->GetMaxLODCount()];
	for (size_t i = 0; i < GameModelComponent.GameModel->GetMaxLODCount(); i++)
		InstancedComponent.LODCounts[i] = 0;

	InstancedComponent.InstancedMatricesLOD.resize(GameModelComponent.GameModel->GetMaxLODCount());

	TransformComponent.SetScale(glm::vec3(1.0f));
}

bool FEInstancedSystem::PopulateInstance(FEEntity* EntityWithInstancedComponent, FESpawnInfo SpawnInfo)
{
	if (EntityWithInstancedComponent == nullptr)
		return false;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return false;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	return PopulateInstance(TransformComponent, GameModelComponent, InstancedComponent, SpawnInfo);
}

bool FEInstancedSystem::PopulateInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, FESpawnInfo SpawnInfo)
{
	if (SpawnInfo.Radius <= 0.0f || SpawnInfo.ObjectCount < 1 || SpawnInfo.ObjectCount > 1000000 || GameModelComponent.GameModel == nullptr)
		return false;

	InstancedComponent.SpawnInfo = SpawnInfo;
	srand(SpawnInfo.Seed);

	const glm::vec3 Min = GameModelComponent.GameModel->GetMesh()->GetAABB().GetMin();
	const glm::vec3 Max = GameModelComponent.GameModel->GetMesh()->GetAABB().GetMax();

	float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	YSize *= GameModelComponent.GameModel->GetScaleFactor();

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
			//Y = InstancedComponent.TerrainToSnap->GetHeightAt(glm::vec2(Position.x + X, Position.z + Z));
			//Y = std::invoke(InstancedComponent.GetTerrainY, InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z));

			if (InstancedComponent.TerrainLayer != -1 && Y != -FLT_MAX)
			{
				FETerrainComponent& TerrainComponent = InstancedComponent.TerrainToSnap->GetComponent<FETerrainComponent>();
				const float LayerIntensity = TERRAIN_SYSTEM.GetLayerIntensityAt(InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z), InstancedComponent.TerrainLayer);
				//const float LayerIntensity = InstancedComponent.TerrainToSnap->GetLayerIntensityAt(glm::vec2(Position.x + X, Position.z + Z), InstancedComponent.TerrainLayer);
				//const float LayerIntensity = std::invoke(InstancedComponent.GetTerrainLayerIntensity, InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z), InstancedComponent.TerrainLayer);
				if (LayerIntensity < InstancedComponent.MinLayerIntensityToSpawn)
					Y = -FLT_MAX;
			}

			int CountOfTries = 0;
			while (Y == -FLT_MAX)
			{
				X = SpawnInfo.GetPositionDeviation();
				Z = SpawnInfo.GetPositionDeviation();
				Y = TERRAIN_SYSTEM.GetHeightAt(InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z));
				//Y = InstancedComponent.TerrainToSnap->GetHeightAt(glm::vec2(Position.x + X, Position.z + Z));
				//Y = std::invoke(InstancedComponent.GetTerrainY, InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z));

				if (InstancedComponent.TerrainLayer != -1 && Y != -FLT_MAX)
				{
					FETerrainComponent& TerrainComponent = InstancedComponent.TerrainToSnap->GetComponent<FETerrainComponent>();
					const float LayerIntensity = TERRAIN_SYSTEM.GetLayerIntensityAt(InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z), InstancedComponent.TerrainLayer);
					//const float LayerIntensity = InstancedComponent.TerrainToSnap->GetLayerIntensityAt(glm::vec2(Position.x + X, Position.z + Z), InstancedComponent.TerrainLayer);
					//const float LayerIntensity = std::invoke(InstancedComponent.GetTerrainLayerIntensity, InstancedComponent.TerrainToSnap, glm::vec2(Position.x + X, Position.z + Z), InstancedComponent.TerrainLayer);
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

		float FinalScale = GameModelComponent.GameModel->GetScaleFactor() + GameModelComponent.GameModel->GetScaleFactor() * SpawnInfo.GetScaleDeviation();
		if (FinalScale < 0.0f)
			FinalScale = 0.01f;
		NewMat = glm::scale(NewMat, glm::vec3(FinalScale));

		NewMats[i] = NewMat;
	}
	AddInstances(TransformComponent, GameModelComponent, InstancedComponent, NewMats.data(), NewMats.size());

	if (InstancedComponent.TerrainToSnap != nullptr)
	{
		// terrain.Y could be not 0.0f but here we should indicate 0.0f as Y.
		TransformComponent.SetPosition(glm::vec3(Position.x, 0.0f, Position.z));
	}

	srand(static_cast<unsigned>(time(nullptr)));

	InstancedComponent.bDirtyFlag = true;
	return true;
}

void FEInstancedSystem::CheckDirtyFlag(FEEntity* EntityWithInstancedComponent)
{
	if (EntityWithInstancedComponent == nullptr)
		return;

	if (!EntityWithInstancedComponent->HasComponent<FEGameModelComponent>())
		return;

	FETransformComponent& TransformComponent = EntityWithInstancedComponent->GetComponent<FETransformComponent>();
	FEGameModelComponent& GameModelComponent = EntityWithInstancedComponent->GetComponent<FEGameModelComponent>();
	FEInstancedComponent& InstancedComponent = EntityWithInstancedComponent->GetComponent<FEInstancedComponent>();
	CheckDirtyFlag(TransformComponent, GameModelComponent, InstancedComponent);
}

void FEInstancedSystem::CheckDirtyFlag(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent)
{
	if (InstancedComponent.LastFrameGameModel != GameModelComponent.GameModel || GameModelComponent.GameModel->IsDirty())
	{
		InstancedComponent.bDirtyFlag = true;
		InstancedComponent.LastFrameGameModel = GameModelComponent.GameModel;
	}

	if (TransformComponent.IsDirty())
		InstancedComponent.bDirtyFlag = true;

	if (InstancedComponent.bDirtyFlag)
	{
		UpdateBuffers(TransformComponent, GameModelComponent, InstancedComponent);
		InstancedComponent.bDirtyFlag = false;
		InitializeGPUCullingBuffers(GameModelComponent, InstancedComponent);
		// It is not correct to set dirty flags here.
		GameModelComponent.GameModel->SetDirtyFlag(false);
	}

	if (TransformComponent.IsDirty())
	{
		TransformComponent.SetDirtyFlag(false);
		UpdateMatrices(TransformComponent, GameModelComponent, InstancedComponent);
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
		InstancedComponent.IndividualInstancedAABB[i] = FEAABB(GameModelComponent.GameModel->GetMesh()->GetAABB(), InstancedComponent.TransformedInstancedMatrices[i]);
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