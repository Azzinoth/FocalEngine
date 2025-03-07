#include "FEInstancedComponent.h"
using namespace FocalEngine;

float FESpawnInfo::GetMinScale()
{
	return MinScale;
}

void FESpawnInfo::SetMinScale(const float NewValue)
{
	if (NewValue >= MaxScale)
		return;

	MinScale = NewValue;
}

float FESpawnInfo::GetMaxScale()
{
	return MaxScale;
}

void FESpawnInfo::SetMaxScale(const float NewValue)
{
	if (NewValue <= MinScale)
		return;

	MaxScale = NewValue;
}

float FESpawnInfo::GetPositionDeviation()
{
	const int IntegerPart = rand() % static_cast<int>(Radius);
	const float FractionalPart = static_cast<float>((rand() % 100) / 100.0f);
	float Result = static_cast<float>(IntegerPart) + FractionalPart;

	Result -= Radius / 2.0f;

	return Result;
}

float FESpawnInfo::GetScaleDeviation()
{
	const float FinalDeviation = MinScale + ((static_cast<float>(rand() % static_cast<int>((MaxScale - MinScale) * 10000)) / 10000.0f));
	return FinalDeviation;
}

int FESpawnInfo::GetRotaionDeviation(const glm::vec3 Axis)
{
	if (Axis.x > 0.0f)
	{
		const int RotationAngle = static_cast<int>(360 * RotationDeviation.x);
		if (RotationAngle == 0)
			return 0;
		return rand() % RotationAngle;
	}
	else if (Axis.y > 0.0f)
	{
		const int RotationAngle = static_cast<int>(360 * RotationDeviation.y);
		if (RotationAngle == 0)
			return 0;
		return rand() % RotationAngle;
	}
	else
	{
		const int RotationAngle = static_cast<int>(360 * RotationDeviation.z);
		if (RotationAngle == 0)
			return 0;
		return rand() % RotationAngle;
	}
}

FEInstancedComponent::FEInstancedComponent()
{

}

FEInstancedComponent::~FEInstancedComponent()
{
	//delete[] LODCounts;
}

size_t FEInstancedComponent::GetInstanceCount()
{
	return InstanceCount;
}

void FEInstancedComponent::Clear()
{
	InstanceCount = 0;

	for (size_t i = 0; i < InstancedElementsData.size(); i++)
	{
		delete[] InstancedElementsData[i]->LODCounts;

		InstancedElementsData[i]->InstancedAABBSizes.resize(0);
		InstancedElementsData[i]->InstancedMatrices.resize(0);
		InstancedElementsData[i]->TransformedInstancedMatrices.resize(0);
		InstancedElementsData[i]->InstancePositions.resize(0);

		delete InstancedElementsData[i];
	}
	InstancedElementsData.resize(0);

	Modifications.clear();
}

FEEntity* FEInstancedComponent::GetSnappedToTerrain()
{
	return TerrainToSnap;
}

int FEInstancedComponent::GetTerrainLayer()
{
	return TerrainLayer;
}

float FEInstancedComponent::GetMinimalLayerIntensityToSpawn()
{
	return MinLayerIntensityToSpawn;
}

void FEInstancedComponent::SetMinimalLayerIntensityToSpawn(float NewValue)
{
	if (NewValue < 0.0001f)
		NewValue = 0.0001f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	MinLayerIntensityToSpawn = NewValue;
}

void FEInstancedComponent::SnapToTerrain(FEEntity* Terrain)
{
	TerrainToSnap = Terrain;
}

void FEInstancedComponent::UnSnapFromTerrain()
{
	TerrainToSnap = nullptr;
}

void FEInstancedComponent::ConnectToTerrainLayer(FEEntity* Terrain, int LayerIndex)
{
	TerrainToSnap = Terrain;
	TerrainLayer = LayerIndex;
}

void FEInstancedComponent::UnConnectFromTerrainLayer()
{
	TerrainLayer = -1;
}

glm::mat4 FEInstancedComponent::GetTransformedInstancedMatrix(size_t InstanceIndex)
{
	if (InstanceIndex < 0 || InstanceIndex >= InstancedElementsData[0]->TransformedInstancedMatrices.size())
		return glm::identity<glm::mat4>();

	return InstancedElementsData[0]->TransformedInstancedMatrices[InstanceIndex];
}

size_t FEInstancedComponent::GetSpawnModificationCount()
{
	return Modifications.size();
}

std::vector<FEInstanceModification> FEInstancedComponent::GetSpawnModifications()
{
	return Modifications;
}