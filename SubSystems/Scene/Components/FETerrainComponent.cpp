#include "FETerrainComponent.h"
using namespace FocalEngine;

FETerrainLayer::FETerrainLayer(const std::string Name) {}
FEMaterial* FETerrainLayer::GetMaterial()
{
	return Material;
}

void FETerrainLayer::SetMaterial(FEMaterial* NewValue)
{
	if (NewValue->IsCompackPacking())
		Material = NewValue;
}

FETerrainComponent::FETerrainComponent()
{
	Layers.resize(FE_TERRAIN_MAX_LAYERS);
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		Layers[i] = nullptr;
	}

	LayerMaps.resize(FE_TERRAIN_MAX_LAYERS / FE_TERRAIN_LAYER_PER_TEXTURE);
	LayerMapsRawData.resize(FE_TERRAIN_MAX_LAYERS / FE_TERRAIN_LAYER_PER_TEXTURE);
	LayerMaps[0] = nullptr;
	LayerMaps[1] = nullptr;

	FE_GL_ERROR(glGenBuffers(1, &GPULayersDataBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, GPULayersDataBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER * FE_TERRAIN_MAX_LAYERS, nullptr, GL_DYNAMIC_DRAW));
	
	GPULayersData.resize(FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER * FE_TERRAIN_MAX_LAYERS);
	OldGPULayersData.resize(FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER * FE_TERRAIN_MAX_LAYERS);
}

FETerrainComponent::~FETerrainComponent()
{
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		delete Layers[i];
		Layers[i] = nullptr;
	}

	FE_GL_ERROR(glDeleteBuffers(1, &GPULayersDataBuffer));
}

bool FETerrainComponent::IsVisible()
{
	return bVisible;
}

void FETerrainComponent::SetVisibility(const bool NewValue)
{
	bVisible = NewValue;
}

bool FETerrainComponent::IsCastingShadows()
{
	return bCastShadows;
}

void FETerrainComponent::SetCastingShadows(const bool NewValue)
{
	bCastShadows = NewValue;
}

bool FETerrainComponent::IsReceivingShadows()
{
	return bReceiveShadows;
}

void FETerrainComponent::SetReceivingShadows(const bool NewValue)
{
	bReceiveShadows = NewValue;
}

void FETerrainComponent::SetWireframeMode(const bool NewValue)
{
	bWireframeMode = NewValue;
}

bool FETerrainComponent::IsWireframeMode()
{
	return bWireframeMode;
}

float FETerrainComponent::GetHightScale()
{
	return HightScale;
}

void FETerrainComponent::SetHightScale(const float NewValue)
{
	if (NewValue <= 0)
		return;

	if (HightScale != NewValue)
		bDirtyFlag = true;

	HightScale = NewValue;
}

glm::vec2 FETerrainComponent::GetTileMult()
{
	return TileMult;
}

void FETerrainComponent::SetTileMult(const glm::vec2 NewValue)
{
	TileMult = NewValue;
}

float FETerrainComponent::GetLODLevel()
{
	return LODLevel;
}

void FETerrainComponent::SetLODLevel(float NewValue)
{
	if (NewValue < 2.0)
		NewValue = 2.0;

	if (NewValue > 128.0)
		NewValue = 128.0;

	LODLevel = NewValue;
}

float FETerrainComponent::GetChunkPerSide()
{
	return ChunkPerSide;
}

void FETerrainComponent::SetChunkPerSide(float NewValue)
{
	if (NewValue < 1.0f)
		NewValue = 1.0f;

	if (NewValue > 16.0f)
		NewValue = 16.0f;

	if (ChunkPerSide != NewValue)
		bDirtyFlag = true;

	ChunkPerSide = NewValue;
}

float FETerrainComponent::GetDisplacementScale()
{
	return DisplacementScale;
}

void FETerrainComponent::SetDisplacementScale(const float NewValue)
{
	DisplacementScale = NewValue;
}

float FETerrainComponent::GetXSize()
{
	return XSize;
}

float FETerrainComponent::GetZSize()
{
	return ZSize;
}

bool FETerrainComponent::GetNextEmptyLayerSlot(size_t& NextEmptyLayerIndex)
{
	for (size_t i = 0; i < Layers.size(); i++)
	{
		if (Layers[i] == nullptr)
		{
			NextEmptyLayerIndex = i;
			return true;
		}
	}

	return false;
}

FETerrainLayer* FETerrainComponent::ActivateVacantLayerSlot(FEMaterial* Material)
{
	size_t LayerIndex = 0;
	if (!GetNextEmptyLayerSlot(LayerIndex))
	{
		LOG.Add("FETerrainComponent::activateLayerSlot was not able to acquire vacant layer index", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	if (Material == nullptr)
	{
		LOG.Add("FETerrainComponent::activateLayerSlot material is nullptr", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	if (!Material->IsCompackPacking())
	{
		LOG.Add("FETerrainComponent::activateLayerSlot material is not compactly packed", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainComponent::activateLayerSlot with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	if (Layers[LayerIndex] != nullptr)
	{
		LOG.Add("FETerrainComponent::activateLayerSlot on indicated layer slot layer is already active", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	Layers[LayerIndex] = new FETerrainLayer(std::string("Layer_") + std::to_string(LayerIndex));
	Layers[LayerIndex]->Material = Material;

	if (LayerIndex == 0)
		SetWireframeMode(false);

	return Layers[LayerIndex];
}

FETerrainLayer* FETerrainComponent::GetLayerInSlot(const size_t LayerIndex)
{
	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainComponent::getLayerInSlot with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	return Layers[LayerIndex];
}

void FETerrainComponent::DeleteLayerInSlot(const size_t LayerIndex)
{
	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainComponent::deleteLayerInSlot with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrainComponent::deleteLayerInSlot on indicated layer slot layer is already inactive", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	delete Layers[LayerIndex];
	Layers[LayerIndex] = nullptr;

	if (LayerIndex == 0 && Layers[LayerIndex + 1] == nullptr)
		SetWireframeMode(true);

	if (LayerIndex + 1 >= FE_TERRAIN_MAX_LAYERS)
		return;

	size_t CurrentIndex = LayerIndex + 1;
	while (true)
	{
		if (CurrentIndex >= FE_TERRAIN_MAX_LAYERS || Layers[CurrentIndex] == nullptr)
			return;

		Layers[CurrentIndex - 1] = Layers[CurrentIndex];
		Layers[CurrentIndex] = nullptr;

		CurrentIndex++;
	}
}

void FETerrainComponent::LoadLayersDataToGPU()
{
	bool GPUDataIsStale = false;
	for (size_t i = 0; i < Layers.size(); i++)
	{
		const size_t index = i * FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER;
		if (Layers[i] != nullptr && Layers[i]->GetMaterial()->IsCompackPacking())
		{
			const FEMaterial* CurrentMaterial = Layers[i]->GetMaterial();
			// normalMapIntensity
			GPULayersData[index] = CurrentMaterial->GetNormalMapIntensity();
			// AOIntensity
			GPULayersData[index + 1] = CurrentMaterial->GetAmbientOcclusionIntensity();
			// AOMapIntensity
			GPULayersData[index + 2] = CurrentMaterial->GetAmbientOcclusionMapIntensity();
			// roughness
			GPULayersData[index + 3] = CurrentMaterial->GetRoughness();
			// roughnessMapIntensity
			GPULayersData[index + 4] = CurrentMaterial->GetRoughnessMapIntensity();
			// metalness
			GPULayersData[index + 5] = CurrentMaterial->GetMetalness();
			// metalnessMapIntensity
			GPULayersData[index + 6] = CurrentMaterial->GetMetalnessMapIntensity();
			// displacementMapIntensity
			GPULayersData[index + 7] = CurrentMaterial->GetDisplacementMapIntensity();
			// tiling
			GPULayersData[index + 8] = CurrentMaterial->GetTiling();
		}
		else
		{
			for (size_t j = 0; j < FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER; j++)
			{
				GPULayersData[index + j] = -1.0f;
			}
		}
	}

	for (size_t i = 0; i < FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER * FE_TERRAIN_MAX_LAYERS; i++)
	{
		if (GPULayersData[i] != OldGPULayersData[i])
		{
			GPUDataIsStale = true;
			OldGPULayersData = GPULayersData;
			break;
		}
	}

	if (GPUDataIsStale)
	{
		float* TerrainLayersDataPtr = static_cast<float*>(glMapNamedBufferRange(GPULayersDataBuffer, 0,
			sizeof(float) * FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER *
			FE_TERRAIN_MAX_LAYERS,
			GL_MAP_WRITE_BIT/* |
			GL_MAP_INVALIDATE_BUFFER_BIT |
			GL_MAP_UNSYNCHRONIZED_BIT*/));
		for (size_t i = 0; i < FE_TERRAIN_MATERIAL_PROPERTIES_PER_LAYER * FE_TERRAIN_MAX_LAYERS; i++)
		{
			TerrainLayersDataPtr[i] = GPULayersData[i];
		}
		FE_GL_ERROR(glUnmapNamedBuffer(GPULayersDataBuffer));
	}
}

int FETerrainComponent::LayersUsed()
{
	int LayersUsed = 0;
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		if (GetLayerInSlot(i) == nullptr)
			break;
		LayersUsed++;
	}

	return LayersUsed;
}