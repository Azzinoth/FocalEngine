#include "FEInstancedRenderingComponent.h"
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
	float result = static_cast<float>(IntegerPart) + FractionalPart;

	result -= Radius / 2.0f;

	return result;
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

FEInstancedRenderingComponent::FEInstancedRenderingComponent()
{

}

FEInstancedRenderingComponent::~FEInstancedRenderingComponent()
{
	//delete[] LODCounts;
}

size_t FEInstancedRenderingComponent::GetInstanceCount()
{
	return InstanceCount;
}

void FEInstancedRenderingComponent::Clear()
{
	InstanceCount = 0;

	delete[] LODCounts;

	InstancedAABBSizes.resize(0);
	InstancedMatrices.resize(0);
	TransformedInstancedMatrices.resize(0);
	InstancePositions.resize(0);

	Modifications.clear();
}

FETerrain* FEInstancedRenderingComponent::GetSnappedToTerrain()
{
	return TerrainToSnap;
}

int FEInstancedRenderingComponent::GetTerrainLayer()
{
	return TerrainLayer;
}

float FEInstancedRenderingComponent::GetMinimalLayerIntensityToSpawn()
{
	return MinLayerIntensityToSpawn;
}

void FEInstancedRenderingComponent::SetMinimalLayerIntensityToSpawn(float NewValue)
{
	if (NewValue < 0.0001f)
		NewValue = 0.0001f;

	if (NewValue > 1.0f)
		NewValue = 1.0f;

	MinLayerIntensityToSpawn = NewValue;
}

void FEInstancedRenderingComponent::SnapToTerrain(FETerrain* Terrain, float(FETerrain::* GetTerrainY)(glm::vec2))
{
	TerrainToSnap = Terrain;
	this->GetTerrainY = GetTerrainY;
}

void FEInstancedRenderingComponent::UnSnapFromTerrain()
{
	TerrainToSnap = nullptr;
}

void FEInstancedRenderingComponent::ConnectToTerrainLayer(FETerrain* Terrain, int LayerIndex, float(FETerrain::* GetTerrainLayerIntensity)(glm::vec2, int))
{
	TerrainToSnap = Terrain;
	TerrainLayer = LayerIndex;
	this->GetTerrainLayerIntensity = GetTerrainLayerIntensity;
}

void FEInstancedRenderingComponent::UnConnectFromTerrainLayer()
{
	TerrainLayer = -1;
}

glm::mat4 FEInstancedRenderingComponent::GetTransformedInstancedMatrix(size_t InstanceIndex)
{
	if (InstanceIndex < 0 || InstanceIndex >= TransformedInstancedMatrices.size())
		return glm::identity<glm::mat4>();

	return TransformedInstancedMatrices[InstanceIndex];
}

size_t FEInstancedRenderingComponent::GetSpawnModificationCount()
{
	return Modifications.size();
}

std::vector<FEInstanceModification> FEInstancedRenderingComponent::GetSpawnModifications()
{
	return Modifications;
}