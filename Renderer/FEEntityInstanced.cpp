#include "FEEntityInstanced.h"

using namespace FocalEngine;

//float FESpawnInfo::GetMinScale()
//{
//	return MinScale;
//}
//
//void FESpawnInfo::SetMinScale(const float NewValue)
//{
//	if (NewValue >= MaxScale)
//		return;
//
//	MinScale = NewValue;
//}
//
//float FESpawnInfo::GetMaxScale()
//{
//	return MaxScale;
//}
//
//void FESpawnInfo::SetMaxScale(const float NewValue)
//{
//	if (NewValue <= MinScale)
//		return;
//
//	MaxScale = NewValue;
//}
//
//float FESpawnInfo::GetPositionDeviation()
//{
//	const int IntegerPart = rand() % static_cast<int>(Radius);
//	const float FractionalPart = static_cast<float>((rand() % 100) / 100.0f);
//	float result = static_cast<float>(IntegerPart) + FractionalPart;
//
//	result -= Radius / 2.0f;
//
//	return result;
//}
//
//float FESpawnInfo::GetScaleDeviation()
//{
//	const float FinalDeviation = MinScale + ((static_cast<float>(rand() % static_cast<int>((MaxScale - MinScale) * 10000)) / 10000.0f));
//	return FinalDeviation;
//}
//
//int FESpawnInfo::GetRotaionDeviation(const glm::vec3 Axis)
//{
//	if (Axis.x > 0.0f)
//	{
//		const int RotationAngle = static_cast<int>(360 * RotationDeviation.x);
//		if (RotationAngle == 0)
//			return 0;
//		return rand() % RotationAngle;
//	}
//	else if (Axis.y > 0.0f)
//	{
//		const int RotationAngle = static_cast<int>(360 * RotationDeviation.y);
//		if (RotationAngle == 0)
//			return 0;
//		return rand() % RotationAngle;
//	}
//	else
//	{
//		const int RotationAngle = static_cast<int>(360 * RotationDeviation.z);
//		if (RotationAngle == 0)
//			return 0;
//		return rand() % RotationAngle;
//	}
//}

//-
FEInstanceModification::FEInstanceModification()
{

}


//-
FEEntityInstanced::FEEntityInstanced(FEPrefab* Prefab, const std::string Name) : FEEntity(Prefab, Name)
{
	SetType(FE_ENTITY_INSTANCED);

	Renderers.resize(Prefab->Components.size());
	for (int i = 0; i < Prefab->Components.size(); i++)
	{
		InitRender(i);
	}

	Transform.SetScale(glm::vec3(1.0f));
}

//
void FEEntityInstanced::InitRender(const int Index)
{
	Renderers[Index] = new FEGameModelInstancedRenderer;
	Renderers[Index]->LODCounts = new int[Prefab->Components[Index]->GameModel->GetMaxLODCount()];

	Renderers[Index]->LODBuffers = new GLenum[Prefab->Components[Index]->GameModel->GetMaxLODCount()];
	for (size_t j = 0; j < Prefab->Components[Index]->GameModel->GetMaxLODCount(); j++)
	{
		FE_GL_ERROR(glGenBuffers(1, &Renderers[Index]->LODBuffers[j]));
	}

	FE_GL_ERROR(glGenBuffers(1, &Renderers[Index]->SourceDataBuffer));
	FE_GL_ERROR(glGenBuffers(1, &Renderers[Index]->PositionsBuffer));
	FE_GL_ERROR(glGenBuffers(1, &Renderers[Index]->AABBSizesBuffer));
	FE_GL_ERROR(glGenBuffers(1, &Renderers[Index]->LODInfoBuffer));

	Renderers[Index]->IndirectDrawsInfo = new FEDrawElementsIndirectCommand[4];
	for (size_t j = 0; j < Prefab->Components[Index]->GameModel->GetMaxLODCount(); j++)
	{
		Renderers[Index]->IndirectDrawsInfo[j].Count = Prefab->Components[Index]->GameModel->GetLODMesh(j) == nullptr ? 0 : Prefab->Components[Index]->GameModel->GetLODMesh(j)->GetVertexCount();
		Renderers[Index]->IndirectDrawsInfo[j].BaseInstance = 0;
		Renderers[Index]->IndirectDrawsInfo[j].BaseVertex = 0;
		Renderers[Index]->IndirectDrawsInfo[j].FirstIndex = 0;
		Renderers[Index]->IndirectDrawsInfo[j].PrimCount = 0;
	}

	FE_GL_ERROR(glGenBuffers(1, &Renderers[Index]->IndirectDrawInfoBuffer));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, Renderers[Index]->IndirectDrawInfoBuffer));
	FE_GL_ERROR(glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(FEDrawElementsIndirectCommand) * 4, Renderers[Index]->IndirectDrawsInfo, GL_MAP_READ_BIT));
	FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));

	Renderers[Index]->InstancedMatricesLOD.resize(Prefab->Components[Index]->GameModel->GetMaxLODCount());
}

FEEntityInstanced::~FEEntityInstanced()
{
	delete[] Renderers.back()->LODCounts; //
	delete[] Renderers.back()->IndirectDrawsInfo;
}

//
void FEEntityInstanced::Render(const int SubGameModel)
{
	if (InstanceCount == 0)
		return;

	if (Renderers.size() <= SubGameModel || Prefab->Components.size() <= SubGameModel)
		return;

	CheckDirtyFlag(SubGameModel);

	for (size_t i = 0; i < Prefab->Components[SubGameModel]->GameModel->GetMaxLODCount(); i++)
	{
		if (Prefab->Components[SubGameModel]->GameModel->IsLODBillboard(i))
			break;

		if (Prefab->Components[SubGameModel]->GameModel->GetLODMesh(i) != nullptr)
		{
			if (Renderers[SubGameModel]->LODBuffers[i] == 0)
				break;

			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, Renderers[SubGameModel]->LODBuffers[i]));

			FE_GL_ERROR(glBindVertexArray(Prefab->Components[SubGameModel]->GameModel->GetLODMesh(i)->GetVaoID()));

			if ((Prefab->Components[SubGameModel]->GameModel->GetLODMesh(i)->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((Prefab->Components[SubGameModel]->GameModel->GetLODMesh(i)->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((Prefab->Components[SubGameModel]->GameModel->GetLODMesh(i)->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((Prefab->Components[SubGameModel]->GameModel->GetLODMesh(i)->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((Prefab->Components[SubGameModel]->GameModel->GetLODMesh(i)->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((Prefab->Components[SubGameModel]->GameModel->GetLODMesh(i)->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			FE_GL_ERROR(glEnableVertexAttribArray(6));
			FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(0)));
			FE_GL_ERROR(glEnableVertexAttribArray(7));
			FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(8));
			FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
			FE_GL_ERROR(glEnableVertexAttribArray(9));
			FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

			FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, Renderers[SubGameModel]->IndirectDrawInfoBuffer));
			FE_GL_ERROR(glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(i * sizeof(FEDrawElementsIndirectCommand))));
			
			FE_GL_ERROR(glBindVertexArray(0));
			FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));
		}
	}

	if (CullingType == FE_CULLING_NONE)
	{
		FE_GL_ERROR(glBindVertexArray(Prefab->Components[SubGameModel]->GameModel->Mesh->GetVaoID()));
		if ((Prefab->Components[SubGameModel]->GameModel->Mesh->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((Prefab->Components[SubGameModel]->GameModel->Mesh->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((Prefab->Components[SubGameModel]->GameModel->Mesh->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((Prefab->Components[SubGameModel]->GameModel->Mesh->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((Prefab->Components[SubGameModel]->GameModel->Mesh->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((Prefab->Components[SubGameModel]->GameModel->Mesh->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		FE_GL_ERROR(glEnableVertexAttribArray(6));
		FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr)));
		FE_GL_ERROR(glEnableVertexAttribArray(7));
		FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(8));
		FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
		FE_GL_ERROR(glEnableVertexAttribArray(9));
		FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

		FE_GL_ERROR(glDrawElementsInstanced(GL_TRIANGLES, Prefab->Components[SubGameModel]->GameModel->Mesh->GetVertexCount(), GL_UNSIGNED_INT, nullptr, static_cast<int>(InstanceCount)));

		FE_GL_ERROR(glBindVertexArray(0));
	}
}

//
void FEEntityInstanced::RenderOnlyBillbords(glm::vec3 CameraPosition)
{
	for (size_t i = 0; i < Renderers.size(); i++)
	{
		for (size_t j = 0; j < Prefab->Components[i]->GameModel->GetMaxLODCount(); j++)
		{
			if (Prefab->Components[i]->GameModel->IsLODBillboard(j) && Prefab->Components[i]->GameModel->GetLODMesh(j) != nullptr)
			{
				if (Renderers[i]->LODBuffers[j] == 0)
					break;

				FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, Renderers[i]->LODBuffers[j]));

				FE_GL_ERROR(glBindVertexArray(Prefab->Components[i]->GameModel->GetLODMesh(j)->GetVaoID()));

				if ((Prefab->Components[i]->GameModel->GetLODMesh(j)->VertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
				if ((Prefab->Components[i]->GameModel->GetLODMesh(j)->VertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
				if ((Prefab->Components[i]->GameModel->GetLODMesh(j)->VertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
				if ((Prefab->Components[i]->GameModel->GetLODMesh(j)->VertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
				if ((Prefab->Components[i]->GameModel->GetLODMesh(j)->VertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
				if ((Prefab->Components[i]->GameModel->GetLODMesh(j)->VertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

				FE_GL_ERROR(glEnableVertexAttribArray(6));
				FE_GL_ERROR(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), static_cast<void*>(nullptr)));
				FE_GL_ERROR(glEnableVertexAttribArray(7));
				FE_GL_ERROR(glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4))));
				FE_GL_ERROR(glEnableVertexAttribArray(8));
				FE_GL_ERROR(glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4))));
				FE_GL_ERROR(glEnableVertexAttribArray(9));
				FE_GL_ERROR(glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4))));

				FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, Renderers[i]->IndirectDrawInfoBuffer));
				FE_GL_ERROR(glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (GLvoid*)(j * sizeof(FEDrawElementsIndirectCommand))));

				FE_GL_ERROR(glBindVertexArray(0));
				FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

				break;
			}
		}
	}
}

//
void FEEntityInstanced::UpdateBuffers()
{
	for (size_t i = 0; i < Prefab->Components.size(); i++)
	{
		if (Renderers[i]->InstancedBuffer != 0)
		{
			glDeleteBuffers(1, &Renderers[i]->InstancedBuffer);
		}

		FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->InstancedBuffer));
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, Renderers[i]->InstancedBuffer));
		FE_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, InstanceCount * sizeof(glm::mat4), Renderers[i]->InstancedMatrices.data(), GL_DYNAMIC_DRAW));

		const unsigned int VAO = Prefab->Components[i]->GameModel->Mesh->GetVaoID();
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

		for (size_t j = 0; j < Prefab->Components[i]->GameModel->GetMaxLODCount(); j++)
		{
			if (Prefab->Components[i]->GameModel->GetLODMesh(j) != nullptr)
			{
				const unsigned int VAO = Prefab->Components[i]->GameModel->GetLODMesh(j)->GetVaoID();
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

		Renderers[i]->AllInstancesAABB = FEAABB();
		for (size_t j = 0; j < InstanceCount; j++)
		{
			glm::mat4 MatWithoutTranslate = Renderers[i]->TransformedInstancedMatrices[j];
			MatWithoutTranslate[3][0] -= Transform.Position.x;
			MatWithoutTranslate[3][1] -= Transform.Position.y;
			MatWithoutTranslate[3][2] -= Transform.Position.z;

			Renderers[i]->AllInstancesAABB = Renderers[i]->AllInstancesAABB.Merge(Prefab->Components[i]->GameModel->Mesh->AABB.Transform(MatWithoutTranslate));
		}
	}

	Transform.bDirtyFlag = true;
	GetAABB();
}

//
void FEEntityInstanced::Clear()
{
	InstanceCount = 0;

	for (size_t i = 0; i < Renderers.size(); i++)
	{
		delete[] Renderers[i]->LODCounts;

		Renderers[i]->InstancedAABBSizes.resize(0);
		Renderers[i]->InstancedMatrices.resize(0);
		Renderers[i]->TransformedInstancedMatrices.resize(0);
		Renderers[i]->InstancePositions.resize(0);
		
		Renderers[i]->LODCounts = new int[Prefab->Components[i]->GameModel->GetMaxLODCount()];
		for (size_t j = 0; j < Prefab->Components[i]->GameModel->GetMaxLODCount(); j++)
		{
			Renderers[i]->LODCounts[j] = 0;
		}

		Renderers[i]->InstancedMatricesLOD.resize(Prefab->Components[i]->GameModel->GetMaxLODCount());
	}
	
	Transform.SetScale(glm::vec3(1.0f));
	Modifications.clear();
}

//
void FEEntityInstanced::AddInstanceInternal(const glm::mat4 InstanceMatrix)
{
	for (size_t i = 0; i < Prefab->Components.size(); i++)
	{
		Renderers[i]->InstancedAABBSizes.push_back(-FEAABB(Prefab->Components[i]->GameModel->GetMesh()->GetAABB(), InstanceMatrix).LongestAxisLength);
		Renderers[i]->InstancedMatrices.push_back(InstanceMatrix);
		Renderers[i]->TransformedInstancedMatrices.push_back(Transform.TransformMatrix * InstanceMatrix);
		Renderers[i]->InstancePositions.push_back(Renderers[i]->TransformedInstancedMatrices.back()[3]);

		for (size_t j = 0; j < Prefab->Components[i]->GameModel->GetMaxLODCount(); j++)
		{
			Renderers[i]->InstancedMatricesLOD[j].resize(InstanceCount);
		}
	}

	InstanceCount++;
	SetDirtyFlag(true);
}

//
void FEEntityInstanced::AddInstances(const glm::mat4* InstanceMatrix, const size_t Count)
{
	for (size_t i = 0; i < Prefab->Components.size(); i++)
	{
		const size_t StartIndex = Renderers[i]->InstancedAABBSizes.size();

		Renderers[i]->InstancedAABBSizes.resize(Renderers[i]->InstancedAABBSizes.size() + Count);
		const FEAABB OriginalAABB = Prefab->Components[i]->GameModel->GetMesh()->GetAABB();
		Renderers[i]->InstancedMatrices.resize(Renderers[i]->InstancedMatrices.size() + Count);
		Renderers[i]->TransformedInstancedMatrices.resize(Renderers[i]->TransformedInstancedMatrices.size() + Count);
		Renderers[i]->InstancePositions.resize(Renderers[i]->InstancePositions.size() + Count);

		for (size_t j = StartIndex; j < Count; j++)
		{
			Renderers[i]->InstancedAABBSizes[j] = -FEAABB(OriginalAABB, InstanceMatrix[j]).LongestAxisLength;
			Renderers[i]->InstancedMatrices[j] = InstanceMatrix[j];
			Renderers[i]->TransformedInstancedMatrices[j] = Transform.TransformMatrix * InstanceMatrix[j];

			Renderers[i]->InstancePositions[j] = Renderers[i]->TransformedInstancedMatrices[j][3];
			if (i == 0)
				InstanceCount++;
		}

		for (size_t j = 0; j < Prefab->Components[i]->GameModel->GetMaxLODCount(); j++)
		{
			Renderers[i]->InstancedMatricesLOD[j].resize(InstanceCount);
		}
	}

	SetDirtyFlag(true);
}

//
FEAABB FEEntityInstanced::GetAABB()
{
	if (Transform.bDirtyFlag)
	{
		if (Prefab != nullptr)
		{
			if (!Renderers.empty())
			{
				EntityAABB = Renderers[0]->AllInstancesAABB.Transform(Transform.GetTransformMatrix());
				for (size_t i = 1; i < Renderers.size(); i++)
				{
					EntityAABB = EntityAABB.Merge(Renderers[i]->AllInstancesAABB.Transform(Transform.GetTransformMatrix()));
				}
			}
			else
			{
				EntityAABB = FEAABB();
			}
		}

		UpdateMatrices();
		Transform.bDirtyFlag = false;
	}

	return EntityAABB;
}

//
int FEEntityInstanced::GetInstanceCount()
{
	return static_cast<int>(InstanceCount);
}

//
void FEEntityInstanced::UpdateMatrices()
{
	for (size_t i = 0; i < Renderers.size(); i++)
	{
		if (Renderers[i]->InstancedMatrices.size() != Renderers[i]->TransformedInstancedMatrices.size())
		{
			LOG.Add("InstancedMatrices size and TransformedInstancedMatrices size is not equal!", "FE_LOG_RENDERING", FE_LOG_ERROR);
			return;
		}

		for (size_t j = 0; j < Renderers[i]->InstancedMatrices.size(); j++)
		{
			Renderers[i]->TransformedInstancedMatrices[j] = Transform.GetTransformMatrix() * Renderers[i]->InstancedMatrices[j];
			Renderers[i]->InstancePositions[j] = Renderers[i]->TransformedInstancedMatrices[j][3];
		}

		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Renderers[i]->SourceDataBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(glm::mat4), Renderers[i]->TransformedInstancedMatrices.data(), GL_DYNAMIC_DRAW));

		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Renderers[i]->PositionsBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(float) * 3, Renderers[i]->InstancePositions.data(), GL_DYNAMIC_DRAW));
	}
}

//
bool FEEntityInstanced::Populate(FESpawnInfo SpawnInfo)
{
	if (SpawnInfo.Radius <= 0.0f || SpawnInfo.ObjectCount < 1 || SpawnInfo.ObjectCount > 1000000 || Prefab == nullptr)
		return false;

	this->SpawnInfo = SpawnInfo;
	srand(SpawnInfo.Seed);

	const glm::vec3 Min = Prefab->GetAABB().GetMin();
	const glm::vec3 Max = Prefab->GetAABB().GetMax();

	float YSize = sqrt((Max.y - Min.y) * (Max.y - Min.y));
	YSize *= Prefab->Components[0]->GameModel->GetScaleFactor();

	std::vector<glm::mat4> NewMats;
	NewMats.resize(SpawnInfo.ObjectCount);

	for (size_t i = 0; i < NewMats.size(); i++)
	{
		glm::mat4 NewMat = glm::mat4(1.0);
		// spawner transformation would be taken in account later so consider center in 0
		float x = SpawnInfo.GetPositionDeviation();
		float z = SpawnInfo.GetPositionDeviation();
		float y = SpawnInfo.GetPositionDeviation();

		if (TerrainToSnap != nullptr)
		{
			y = std::invoke(GetTerrainY, TerrainToSnap, glm::vec2(Transform.Position.x + x, Transform.Position.z + z));

			if (TerrainLayer != -1 && y != -FLT_MAX)
			{
				const float LayerIntensity = std::invoke(GetTerrainLayerIntensity, TerrainToSnap, glm::vec2(Transform.Position.x + x, Transform.Position.z + z), TerrainLayer);
				if (LayerIntensity < MinLayerIntensity)
					y = -FLT_MAX;
			}

			int CountOfTries = 0;
			while (y == -FLT_MAX)
			{
				x = SpawnInfo.GetPositionDeviation();
				z = SpawnInfo.GetPositionDeviation();
				y = std::invoke(GetTerrainY, TerrainToSnap, glm::vec2(Transform.Position.x + x, Transform.Position.z + z));

				if (TerrainLayer != -1 && y != -FLT_MAX)
				{
					const float LayerIntensity = std::invoke(GetTerrainLayerIntensity, TerrainToSnap, glm::vec2(Transform.Position.x + x, Transform.Position.z + z), TerrainLayer);
					if (LayerIntensity < MinLayerIntensity)
						y = -FLT_MAX;
				}

				CountOfTries++;
				if (CountOfTries > 300)
					break;
			}

			if (CountOfTries > 300)
			{
				y = Transform.Position.y + SpawnInfo.GetPositionDeviation();
			}
		}

		NewMat = glm::translate(NewMat, glm::vec3(x, y, z));

		NewMat = glm::rotate(NewMat, SpawnInfo.GetRotaionDeviation(glm::vec3(1, 0, 0)) * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
		NewMat = glm::rotate(NewMat, SpawnInfo.GetRotaionDeviation(glm::vec3(0, 1, 0)) * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
		NewMat = glm::rotate(NewMat, SpawnInfo.GetRotaionDeviation(glm::vec3(0, 0, 1)) * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));

		float FinalScale = Prefab->Components[0]->GameModel->GetScaleFactor() + Prefab->Components[0]->GameModel->GetScaleFactor() * SpawnInfo.GetScaleDeviation();
		if (FinalScale < 0.0f)
			FinalScale = 0.01f;
		NewMat = glm::scale(NewMat, glm::vec3(FinalScale));

		NewMats[i] = NewMat;
	}
	AddInstances(NewMats.data(), NewMats.size());

	if (TerrainToSnap != nullptr)
	{
		// terrain.y could be not 0.0f but here we should indicate 0.0f as Y.
		Transform.SetPosition(glm::vec3(Transform.Position.x, 0.0f, Transform.Position.z));
	}

	srand(static_cast<unsigned>(time(nullptr)));

	SetDirtyFlag(true);
	return true;
}

//
FETerrain* FEEntityInstanced::GetSnappedToTerrain()
{
	return TerrainToSnap;
}

//
void FEEntityInstanced::UpdateSelectModeAABBData()
{
	InstancedAABB.clear();
	InstancedAABB.resize(InstanceCount);

	for (size_t i = 0; i < InstanceCount; i++)
	{
		InstancedAABB[i] = FEAABB(Prefab->GetAABB(), Renderers[0]->TransformedInstancedMatrices[i]);
	}

	SetDirtyFlag(true);
}

//
bool FEEntityInstanced::IsSelectMode()
{
	return bSelectionMode;
}

//
void FEEntityInstanced::SetSelectMode(const bool NewValue)
{
	if (NewValue)
		UpdateSelectModeAABBData();
	
	bSelectionMode = NewValue;
}

//
void FEEntityInstanced::DeleteInstance(const size_t InstanceIndex)
{
	if (InstanceIndex < 0 || InstanceIndex >= Renderers[0]->InstancedMatrices.size())
		return;

	Modifications.push_back(FEInstanceModification(FE_CHANGE_DELETED, static_cast<int>(InstanceIndex), glm::mat4()));

	InstanceCount--;
	for (size_t i = 0; i < Renderers.size(); i++)
	{
		Renderers[i]->InstancedAABBSizes.erase(Renderers[i]->InstancedAABBSizes.begin() + InstanceIndex);
		Renderers[i]->InstancedMatrices.erase(Renderers[i]->InstancedMatrices.begin() + InstanceIndex);
		Renderers[i]->TransformedInstancedMatrices.erase(Renderers[i]->TransformedInstancedMatrices.begin() + InstanceIndex);
		Renderers[i]->InstancePositions.erase(Renderers[i]->InstancePositions.begin() + InstanceIndex);

		for (size_t j = 0; j < Prefab->Components[i]->GameModel->GetMaxLODCount(); j++)
		{
			Renderers[i]->InstancedMatricesLOD[j].resize(InstanceCount);
		}
	}

	if (InstancedAABB.empty())
	{
		UpdateSelectModeAABBData();
	}
	else
	{
		InstancedAABB.erase(InstancedAABB.begin() + InstanceIndex);
	}
	
	SetDirtyFlag(true);
}

//
glm::mat4 FEEntityInstanced::GetTransformedInstancedMatrix(const size_t InstanceIndex)
{
	if (InstanceIndex < 0 || InstanceIndex >= Renderers[0]->TransformedInstancedMatrices.size())
		return glm::identity<glm::mat4>();

	return Renderers[0]->TransformedInstancedMatrices[InstanceIndex];
}

//
void FEEntityInstanced::ModifyInstance(const size_t InstanceIndex, glm::mat4 NewMatrix)
{
	if (InstanceIndex < 0 || InstanceIndex >= Renderers[0]->TransformedInstancedMatrices.size())
		return;

	if (glm::all(glm::epsilonEqual(Renderers[0]->TransformedInstancedMatrices[InstanceIndex][0], NewMatrix[0], 0.001f)) &&
		glm::all(glm::epsilonEqual(Renderers[0]->TransformedInstancedMatrices[InstanceIndex][1], NewMatrix[1], 0.001f)) &&
		glm::all(glm::epsilonEqual(Renderers[0]->TransformedInstancedMatrices[InstanceIndex][2], NewMatrix[2], 0.001f)) &&
		glm::all(glm::epsilonEqual(Renderers[0]->TransformedInstancedMatrices[InstanceIndex][3], NewMatrix[3], 0.001f)))
		return;

	if (!Modifications.empty() && Modifications.back().Index == InstanceIndex && Modifications.back().Type == FE_CHANGE_MODIFIED)
	{
		Modifications.back().Modification = NewMatrix;
	}
	else
	{
		Modifications.push_back(FEInstanceModification(FE_CHANGE_MODIFIED, static_cast<int>(InstanceIndex), NewMatrix));
	}
	
	for (size_t i = 0; i < Renderers.size(); i++)
	{
		Renderers[i]->TransformedInstancedMatrices[InstanceIndex] = NewMatrix;
		Renderers[i]->InstancedMatrices[InstanceIndex] = glm::inverse(Transform.GetTransformMatrix()) * NewMatrix;

		if (InstancedAABB.size() > InstanceIndex)
			InstancedAABB[InstanceIndex] = FEAABB(Prefab->GetAABB(), NewMatrix);
		Renderers[i]->InstancedAABBSizes[InstanceIndex] = -FEAABB(Prefab->GetAABB(), NewMatrix).LongestAxisLength;
	}

	SetDirtyFlag(true);
}

//
int FEEntityInstanced::GetSpawnModificationCount()
{
	return static_cast<int>(Modifications.size());
}

//
std::vector<FEInstanceModification> FEEntityInstanced::GetSpawnModifications()
{
	return Modifications;
}

//
void FEEntityInstanced::AddInstance(const glm::mat4 InstanceMatrix)
{
	AddInstanceInternal(glm::inverse(Transform.TransformMatrix) * InstanceMatrix);

	if (InstancedAABB.empty())
	{
		UpdateSelectModeAABBData();
	}
	else
	{
		InstancedAABB.push_back(FEAABB(Prefab->GetAABB(), Renderers[0]->TransformedInstancedMatrices.back()));
	}

	Modifications.push_back(FEInstanceModification(FE_CHANGE_ADDED, static_cast<int>(InstanceCount), InstanceMatrix));
	SetDirtyFlag(true);
}

//
bool FEEntityInstanced::TryToSnapInstance(const size_t InstanceIndex)
{
	if (InstanceIndex < 0 || InstanceIndex >= Renderers[0]->TransformedInstancedMatrices.size() || TerrainToSnap == nullptr)
		return false;

	if (!IsSelectMode())
		return false;

	const float y = std::invoke(GetTerrainY, TerrainToSnap, glm::vec2(Renderers[0]->TransformedInstancedMatrices[InstanceIndex][3][0], Renderers[0]->TransformedInstancedMatrices[InstanceIndex][3][2]));
	if (y == -FLT_MAX)
		return false;

	const float LayerIntensity = std::invoke(GetTerrainLayerIntensity, TerrainToSnap, glm::vec2(Renderers[0]->TransformedInstancedMatrices[InstanceIndex][3][0], Renderers[0]->TransformedInstancedMatrices[InstanceIndex][3][2]), TerrainLayer);
	if (LayerIntensity < MinLayerIntensity)
		return false;

	if (abs(Renderers[0]->TransformedInstancedMatrices[InstanceIndex][3][1] - y) < 0.01f)
		return true;

	glm::mat4 Copy = Renderers[0]->TransformedInstancedMatrices[InstanceIndex];
	Copy[3][1] = y;
	ModifyInstance(InstanceIndex, Copy);
	SetDirtyFlag(true);
	return true;
}

//
void FEEntityInstanced::InitializeGPUCulling()
{
	for (size_t i = 0; i < Prefab->Components.size(); i++)
	{
		if (Renderers[i]->SourceDataBuffer != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &Renderers[i]->SourceDataBuffer));
			FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->SourceDataBuffer));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Renderers[i]->SourceDataBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(glm::mat4), Renderers[i]->TransformedInstancedMatrices.data(), GL_DYNAMIC_DRAW));

		if (Renderers[i]->PositionsBuffer != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &Renderers[i]->PositionsBuffer));
			FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->PositionsBuffer));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Renderers[i]->PositionsBuffer));

		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(float) * 3, Renderers[i]->InstancePositions.data(), GL_DYNAMIC_DRAW));

		if (Renderers[i]->LODBuffers[0] != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &Renderers[i]->LODBuffers[0]));
			FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->LODBuffers[0]));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, Renderers[i]->LODBuffers[0]));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

		if (Renderers[i]->LODBuffers[1] != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &Renderers[i]->LODBuffers[1]));
			FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->LODBuffers[1]));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, Renderers[i]->LODBuffers[1]));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

		if (Renderers[i]->LODBuffers[2] != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &Renderers[i]->LODBuffers[2]));
			FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->LODBuffers[2]));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, Renderers[i]->LODBuffers[2]));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

		if (Renderers[i]->LODBuffers[3] != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &Renderers[i]->LODBuffers[3]));
			FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->LODBuffers[3]));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, Renderers[i]->LODBuffers[3]));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW));

		if (Renderers[i]->AABBSizesBuffer != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &Renderers[i]->AABBSizesBuffer));
			FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->AABBSizesBuffer));
		}
		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, Renderers[i]->AABBSizesBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, InstanceCount * sizeof(float), Renderers[i]->InstancedAABBSizes.data(), GL_DYNAMIC_DRAW));

		std::vector<float> LODInfoData;
		LODInfoData.push_back(Prefab->Components[i]->GameModel->GetCullDistance());
		LODInfoData.push_back(Prefab->Components[i]->GameModel->GetLODMaxDrawDistance(0));
		LODInfoData.push_back(Prefab->Components[i]->GameModel->GetLODMaxDrawDistance(1));
		LODInfoData.push_back(Prefab->Components[i]->GameModel->GetLODMaxDrawDistance(2));

		// does it have billboard ?
		unsigned int BillboardIndex = 5;
		for (size_t j = 0; j < Prefab->Components[i]->GameModel->GetMaxLODCount(); j++)
		{
			if (Prefab->Components[i]->GameModel->IsLODBillboard(j) && Prefab->Components[i]->GameModel->GetLODMesh(j) != nullptr)
			{
				BillboardIndex = static_cast<int>(j);
			}
		}

		LODInfoData.push_back(static_cast<float>(BillboardIndex));
		// this should not be here, instead normal of plane should align with vector to camera
		LODInfoData.push_back(1.5708f * 3.0f + Prefab->Components[i]->GameModel->GetBillboardZeroRotaion() * ANGLE_TORADIANS_COF);
		LODInfoData.push_back(static_cast<float>(InstanceCount));

		FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, Renderers[i]->LODInfoBuffer));
		FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, 7 * sizeof(float), LODInfoData.data(), GL_DYNAMIC_DRAW));

		if (Renderers[i]->IndirectDrawsInfo == nullptr)
			Renderers[i]->IndirectDrawsInfo = new FEDrawElementsIndirectCommand[4];
		for (size_t j = 0; j < Prefab->Components[i]->GameModel->GetMaxLODCount(); j++)
		{
			Renderers[i]->IndirectDrawsInfo[j].Count = Prefab->Components[i]->GameModel->GetLODMesh(j) == nullptr ? 0 : Prefab->Components[i]->GameModel->GetLODMesh(j)->GetVertexCount();
			Renderers[i]->IndirectDrawsInfo[j].BaseInstance = 0;
			Renderers[i]->IndirectDrawsInfo[j].BaseVertex = 0;
			Renderers[i]->IndirectDrawsInfo[j].FirstIndex = 0;
			Renderers[i]->IndirectDrawsInfo[j].PrimCount = 0;
		}

		if (Renderers[i]->IndirectDrawInfoBuffer != 0)
		{
			FE_GL_ERROR(glDeleteBuffers(1, &Renderers[i]->IndirectDrawInfoBuffer));
			FE_GL_ERROR(glGenBuffers(1, &Renderers[i]->IndirectDrawInfoBuffer));
		}

		FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, Renderers[i]->IndirectDrawInfoBuffer));
		FE_GL_ERROR(glBufferStorage(GL_DRAW_INDIRECT_BUFFER, sizeof(FEDrawElementsIndirectCommand) * 4, Renderers[i]->IndirectDrawsInfo, GL_MAP_READ_BIT));
		FE_GL_ERROR(glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0));
	}
}

//
void FEEntityInstanced::SnapToTerrain(FETerrain* Terrain, float(FETerrain::* GetTerrainY)(glm::vec2))
{
	TerrainToSnap = Terrain;
	this->GetTerrainY = GetTerrainY;
}

//
void FEEntityInstanced::UnSnapFromTerrain()
{
	TerrainToSnap = nullptr;
}

// -
void FEEntityInstanced::ClearRenderers()
{
	if (Prefab->Components.size() > Renderers.size())
	{
		const int CountBefore = static_cast<int>(Renderers.size());
		Renderers.resize(Prefab->Components.size());
		for (int i = CountBefore; i < Renderers.size(); i++)
		{
			InitRender(i);

			Renderers[i]->InstancedMatrices = Renderers[0]->InstancedMatrices;
			Renderers[i]->TransformedInstancedMatrices = Renderers[0]->TransformedInstancedMatrices;
			Renderers[i]->InstancePositions = Renderers[0]->InstancePositions;
		}
	}
	else if (Prefab->Components.size() < Renderers.size())
	{
		Renderers.erase(Renderers.begin() + Renderers.size() - 1, Renderers.end());
	}
}

//
void FEEntityInstanced::CheckDirtyFlag(const int SubGameModel)
{
	if (Renderers[SubGameModel]->LastFramePrefab != Prefab || Prefab->IsDirty())
	{
		SetDirtyFlag(true);
		Renderers[SubGameModel]->LastFramePrefab = Prefab;
	}

	for (int i = 0; i < Prefab->ComponentsCount(); i++)
	{
		if (Prefab->GetComponent(i)->GameModel->IsDirty() || Prefab->GetComponent(i)->Transform.IsDirty())
		{
			SetDirtyFlag(true);
			break;
		}
	}

	if (IsDirty())
	{
		ClearRenderers();
		UpdateBuffers();
		SetDirtyFlag(false);
		InitializeGPUCulling();
		Prefab->SetDirtyFlag(false);
		for (int i = 0; i < Prefab->ComponentsCount(); i++)
		{
			Prefab->GetComponent(i)->GameModel->SetDirtyFlag(false);
			Prefab->GetComponent(i)->Transform.SetDirtyFlag(false);
		}
	}

	if (Transform.bDirtyFlag)
	{
		UpdateMatrices();
	}
}

//
void FEEntityInstanced::ConnectToTerrainLayer(FETerrain* Terrain, const int LayerIndex, float(FETerrain::* GetTerrainLayerIntensity)(glm::vec2, int))
{
	this->GetTerrainLayerIntensity = GetTerrainLayerIntensity;
	TerrainLayer = LayerIndex;
}

//
int FEEntityInstanced::GetTerrainLayer()
{
	return TerrainLayer;
}

//
void FEEntityInstanced::UnConnectFromTerrainLayer()
{
	TerrainLayer = -1;
}

//
float FEEntityInstanced::GetMinimalLayerIntensity()
{
	return MinLayerIntensity;
}

//
void FEEntityInstanced::SetMinimalLayerIntensity(float NewValue)
{
	if (NewValue < 0.0001f)
		NewValue = 0.0001f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	MinLayerIntensity = NewValue;
}