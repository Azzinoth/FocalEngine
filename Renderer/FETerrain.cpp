#include "FETerrain.h"
using namespace FocalEngine;

FETerrainLayer::FETerrainLayer(const std::string Name) : FEObject(FE_TERRAIN_LAYER, Name) {}
FEMaterial* FETerrainLayer::GetMaterial()
{
	return Material;
}

void FETerrainLayer::SetMaterial(FEMaterial* NewValue)
{
	if (NewValue->IsCompackPacking())
		Material = NewValue;
}

FETerrain::FETerrain(const std::string Name) : FEObject(FE_TERRAIN, Name)
{
	SetName(Name);
	NameHash = static_cast<int>(std::hash<std::string>{}(Name));

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

FETerrain::~FETerrain()
{
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		delete Layers[i];
	}

	FE_GL_ERROR(glDeleteBuffers(1, &GPULayersDataBuffer));
}

void FETerrain::Render()
{
	FE_GL_ERROR(glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64));
}

bool FETerrain::IsVisible()
{
	return bVisible;
}

void FETerrain::SetVisibility(const bool NewValue)
{
	bVisible = NewValue;
}

FEAABB FETerrain::GetAABB()
{
	if (Transform.bDirtyFlag)
	{
		Transform.bDirtyFlag = false;
		//#fix it should be optimized.
		FEAABB MeshAABB = AABB;
		// -0.5f it is a little hack, because this -0.5f should be made during tessellation.
		MeshAABB.Min = glm::vec3(-32.0f - 0.5f, AABB.GetMin()[1], -32.0f - 0.5f);
		MeshAABB.Max = glm::vec3(32.0f + 64.0f * (ChunkPerSide - 1) - 0.5f, AABB.GetMax()[1], 32.0f + 64.0f * (ChunkPerSide - 1) - 0.5f);
		MeshAABB = FEAABB(glm::vec3(MeshAABB.GetMin()[0], MeshAABB.GetMin()[1] * 2 * HightScale - HightScale, MeshAABB.GetMin()[2]), glm::vec3(MeshAABB.GetMax()[0], MeshAABB.GetMax()[1] * 2 * HightScale - HightScale, MeshAABB.GetMax()[2]));

		FinalAABB = MeshAABB.Transform(Transform.GetTransformMatrix());

		XSize = FinalAABB.GetMax()[0] - FinalAABB.GetMin()[0];
		ZSize = FinalAABB.GetMax()[2] - FinalAABB.GetMin()[2];
	}

	return FinalAABB;
}

FEAABB FETerrain::GetPureAABB()
{
	FEAABB MeshAABB = AABB;
	MeshAABB.Min = glm::vec3(-32.0f, AABB.GetMin()[1], -32.0f);
	MeshAABB.Max = glm::vec3(32.0f + 64.0f * (ChunkPerSide - 1), AABB.GetMax()[1], 32.0f + 64.0f * (ChunkPerSide - 1));
	MeshAABB = FEAABB(glm::vec3(MeshAABB.GetMin()[0], MeshAABB.GetMin()[1] * 2 * HightScale - HightScale, MeshAABB.GetMin()[2]), glm::vec3(MeshAABB.GetMax()[0], MeshAABB.GetMax()[1] * 2 * HightScale - HightScale, MeshAABB.GetMax()[2]));
	return MeshAABB;
}

bool FETerrain::IsCastingShadows()
{
	return bCastShadows;
}

void FETerrain::SetCastingShadows(const bool NewValue)
{
	bCastShadows = NewValue;
}

bool FETerrain::IsReceivingShadows()
{
	return bReceiveShadows;
}

void FETerrain::SetReceivingShadows(const bool NewValue)
{
	bReceiveShadows = NewValue;
}

void FETerrain::SetWireframeMode(const bool NewValue)
{
	bWireframeMode = NewValue;
}

bool FETerrain::IsWireframeMode()
{
	return bWireframeMode;
}

float FETerrain::GetHightScale()
{
	return HightScale;
}

void FETerrain::SetHightScale(const float NewValue)
{
	if (NewValue <= 0)
		return;

	if (HightScale != NewValue)
		Transform.bDirtyFlag = true;
	HightScale = NewValue;
}

glm::vec2 FETerrain::GetTileMult()
{
	return TileMult;
}

void FETerrain::SetTileMult(const glm::vec2 NewValue)
{
	TileMult = NewValue;
}

float FETerrain::GetLODLevel()
{
	return LODLevel;
}

void FETerrain::SetLODLevel(float NewValue)
{
	if (NewValue < 2.0)
		NewValue = 2.0;

	if (NewValue > 128.0)
		NewValue = 128.0;

	LODLevel = NewValue;
}

float FETerrain::GetChunkPerSide()
{
	return ChunkPerSide;
}

void FETerrain::SetChunkPerSide(float NewValue)
{
	if (NewValue < 1.0f)
		NewValue = 1.0f;

	if (NewValue > 16.0f)
		NewValue = 16.0f;

	if (ChunkPerSide != NewValue)
		Transform.bDirtyFlag = true;
	ChunkPerSide = NewValue;
}

float FETerrain::GetDisplacementScale()
{
	return DisplacementScale;
}

void FETerrain::SetDisplacementScale(const float NewValue)
{
	DisplacementScale = NewValue;
}

float FETerrain::GetHeightAt(glm::vec2 XZWorldPosition)
{
	float LocalX = XZWorldPosition[0];
	float LocalZ = XZWorldPosition[1];

	LocalX -= FinalAABB.GetMin()[0];
	LocalZ -= FinalAABB.GetMin()[2];

	if (XSize == 0 || ZSize == 0)
		GetAABB();

	LocalX = LocalX / XSize;
	LocalZ = LocalZ / ZSize;

	if (LocalX > 0 && LocalZ > 0 && LocalX < 1.0 && LocalZ < 1.0)
	{
		LocalX = static_cast<float>(static_cast<int>(LocalX * this->HeightMap->GetWidth()));
		LocalZ = static_cast<float>(static_cast<int>(LocalZ * this->HeightMap->GetHeight()));

		const int Index = static_cast<int>(LocalZ * this->HeightMap->GetWidth() + LocalX);
		return (HeightMapArray[Index] * 2 * HightScale - HightScale) * Transform.GetScale()[1] + Transform.GetPosition()[1];
	}

	return -FLT_MAX;
}

float FETerrain::GetXSize()
{
	return XSize;
}

float FETerrain::GetZSize()
{
	return ZSize;
}

// ********************************** PointOnTerrain **********************************
bool FETerrain::IsUnderGround(const glm::dvec3 TestPoint)
{
	const float Height = GetHeightAt(glm::vec2(TestPoint.x, TestPoint.z));
	// if we go outside terrain.
	if (Height == -FLT_MAX)
		return true;

	return TestPoint.y < Height ? true : false;
}

glm::dvec3 FETerrain::GetPointOnRay(const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection, const float Distance)
{
	const glm::dvec3 Start = glm::dvec3(MouseRayStart.x, MouseRayStart.y, MouseRayStart.z);
	const glm::dvec3 ScaledRay = glm::dvec3(MouseRayDirection.x * Distance, MouseRayDirection.y * Distance, MouseRayDirection.z * Distance);

	return Start + ScaledRay;
}

bool FETerrain::IntersectionInRange(const float Start, const float Finish, const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection)
{
	const glm::dvec3 StartPoint = GetPointOnRay(MouseRayStart, MouseRayDirection, Start);
	const glm::dvec3 EndPoint = GetPointOnRay(MouseRayStart, MouseRayDirection, Finish);
	return !IsUnderGround(StartPoint) && IsUnderGround(EndPoint) ? true : false;
}

glm::dvec3 FETerrain::BinarySearch(const int Count, const float Start, const float Finish, const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection)
{
	const int RecursionCount = 200;
	const float Half = Start + ((Finish - Start) / 2.0f);
	if (Count >= RecursionCount)
	{
		const glm::dvec3 EndPoint = GetPointOnRay(MouseRayStart, MouseRayDirection, Half);
		if (GetHeightAt(glm::vec2(EndPoint.x, EndPoint.z)) != -1.0f)
		{
			return EndPoint;
		}
		else
		{
			return glm::dvec3(FLT_MAX);
		}
	}

	if (IntersectionInRange(Start, Half, MouseRayStart, MouseRayDirection))
	{
		return BinarySearch(Count + 1, Start, Half, MouseRayStart, MouseRayDirection);
	}
	else
	{
		return BinarySearch(Count + 1, Half, Finish, MouseRayStart, MouseRayDirection);
	}
}

glm::dvec3 FETerrain::GetPointOnTerrain(const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection, const float StartDistance, const float EndDistance)
{
	if (IntersectionInRange(StartDistance, EndDistance, MouseRayStart, MouseRayDirection))
	{
		const glm::dvec3 PointOnTerrain = BinarySearch(0, StartDistance, EndDistance, MouseRayStart, MouseRayDirection);
		// if point is not above terrain, point could be above because isUnderGround returning true if we go out of terrain bounds to fix some bugs.
		if ((GetHeightAt(glm::vec2(PointOnTerrain.x, PointOnTerrain.z)) + 1.0f) > PointOnTerrain.y)
			return PointOnTerrain;

		return glm::dvec3(FLT_MAX);
	}

	return glm::dvec3(FLT_MAX);
}

void FETerrain::UpdateCpuHeightInfo()
{
	/*FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, heightMap->getTextureID()));

	size_t rawDataLenght = heightMap->getWidth() * heightMap->getHeight() * 2;
	unsigned char* rawData = new unsigned char[rawDataLenght];
	glPixelStorei(GL_PACK_ALIGNMENT, 2);
	FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, rawData));
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	heightMap->unBind();*/

	size_t RawDataLenght;
	unsigned char* RawData = HeightMap->GetRawData(&RawDataLenght);

	float Max = FLT_MIN;
	float Min = FLT_MAX;
	int Iterator = 0;
	for (size_t i = 0; i < RawDataLenght; i += 2)
	{
		const unsigned short Temp = *(unsigned short*)(&RawData[i]);
		HeightMapArray[Iterator] = Temp / static_cast<float>(0xFFFF);

		if (Max < HeightMapArray[Iterator])
			Max = HeightMapArray[Iterator];

		if (Min > HeightMapArray[Iterator])
			Min = HeightMapArray[Iterator];

		Iterator++;
	}

	const glm::vec3 MinPoint = glm::vec3(-1.0f, Min, -1.0f);
	const glm::vec3 MaxPoint = glm::vec3(1.0f, Max, 1.0f);
	AABB = FEAABB(MinPoint, MaxPoint);
	Transform.bDirtyFlag = true;

	delete[] RawData;
}
// ********************************** PointOnTerrain END **********************************

// **************************** TERRAIN EDITOR TOOLS ****************************
bool FETerrain::IsBrushActive()
{
	return bBrushActive;
}

void FETerrain::SetBrushActive(const bool NewValue)
{
	bBrushActive = NewValue;
}

FE_TERRAIN_BRUSH_MODE FETerrain::GetBrushMode()
{
	return BrushMode;
}

void FETerrain::SetBrushMode(const FE_TERRAIN_BRUSH_MODE NewValue)
{
	BrushMode = NewValue;
}

float FETerrain::GetBrushSize()
{
	return BrushSize;
}

void FETerrain::SetBrushSize(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.001f;

	if (NewValue > 1000.0f)
		NewValue = 1000.0f;

	BrushSize = NewValue;
}

float FETerrain::GetBrushIntensity()
{
	return BrushIntensity;
}

void FETerrain::SetBrushIntensity(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.001f;

	if (NewValue > 1000.0f)
		NewValue = 1000.0f;

	BrushIntensity = NewValue;
}

void FETerrain::UpdateBrush(const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection)
{
	if (BrushVisualFB == nullptr)
		return;

	if (GetBrushMode() == FE_TERRAIN_BRUSH_NONE)
	{
		if (bBrushVisualFBCleared)
			return;

		// get current clear color.
		GLfloat BkColor[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, BkColor);

		BrushVisualFB->Bind();
		FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));
		BrushVisualFB->UnBind();
		bBrushVisualFBCleared = true;

		FE_GL_ERROR(glClearColor(BkColor[0], BkColor[1], BkColor[2], BkColor[3]));

		return;
	}

	if (bCPUHeightInfoDirtyFlag)
	{
		if (std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::system_clock::now() - LastChangesTimeStamp).count() * 1000.0f > WaitBeforeUpdateMs)
		{
			bCPUHeightInfoDirtyFlag = false;
			UpdateCpuHeightInfo();
			UpdateSnapedInstancedEntities();
		}
	}

	const float Range = GetXSize() * 2.0f;
	const glm::dvec3 CurrentTerrainPoint = GetPointOnTerrain(MouseRayStart, MouseRayDirection, 0, Range);

	float LocalX = static_cast<float>(CurrentTerrainPoint.x);
	float LocalZ = static_cast<float>(CurrentTerrainPoint.z);

	LocalX -= GetAABB().GetMin()[0];
	LocalZ -= GetAABB().GetMin()[2];

	LocalX = LocalX / GetXSize();
	LocalZ = LocalZ / GetZSize();

	if (IsBrushActive())
	{
		LastChangesTimeStamp = std::chrono::system_clock::now();
		bCPUHeightInfoDirtyFlag = true;
		HeightMap->SetDirtyFlag(true);

		if (LocalX > 0 && LocalZ > 0 && LocalX < 1.0 && LocalZ < 1.0)
		{
			BrushOutputShader->GetParameter("brushCenter")->UpdateData(glm::vec2(LocalX, LocalZ));
		}

		BrushOutputShader->GetParameter("brushSize")->UpdateData(BrushSize / (GetXSize() * 2.0f));
		BrushOutputShader->GetParameter("brushMode")->UpdateData(static_cast<float>(GetBrushMode()));

		BrushOutputFB->SetColorAttachment(HeightMap);
		BrushOutputShader->GetParameter("brushIntensity")->UpdateData(BrushIntensity / 10.0f);

		BrushOutputFB->Bind();
		BrushOutputShader->Start();
		
		if (BrushMode == FE_TERRAIN_BRUSH_LAYER_DRAW)
		{
			LayerMaps[0]->Bind(0);
			FE_GL_ERROR(glViewport(0, 0, LayerMaps[0]->GetWidth(), LayerMaps[0]->GetHeight()));

			BrushOutputShader->GetParameter("brushIntensity")->UpdateData(BrushIntensity * 5.0f);
			BrushOutputShader->GetParameter("layerIndex")->UpdateData(static_cast<float>(GetBrushLayerIndex()));
			BrushOutputFB->SetColorAttachment(LayerMaps[0]);
			BrushOutputShader->LoadDataToGPU();

			FE_GL_ERROR(glBindVertexArray(PlaneMesh->GetVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));


			LayerMaps[1]->Bind(0);
			BrushOutputShader->GetParameter("layerIndex")->UpdateData(static_cast<float>(GetBrushLayerIndex() - 4.0f));
			BrushOutputFB->SetColorAttachment(LayerMaps[1]);
			BrushOutputShader->LoadDataToGPU();

			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			BrushOutputShader->Stop();

			// Normalize all layers values to add up to 1.0
			LayersNormalizeShader->Start();
			LayerMaps[0]->Bind(0);
			LayerMaps[1]->Bind(1);

			BrushOutputFB->SetColorAttachment(LayerMaps[0], 0);
			BrushOutputFB->SetColorAttachment(LayerMaps[1], 1);

			FE_GL_ERROR(glBindVertexArray(PlaneMesh->GetVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			LayersNormalizeShader->Stop();
		}
		else
		{
			BrushOutputShader->LoadDataToGPU();
			HeightMap->Bind(0);

			FE_GL_ERROR(glViewport(0, 0, HeightMap->GetWidth(), HeightMap->GetHeight()));

			FE_GL_ERROR(glBindVertexArray(PlaneMesh->GetVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			HeightMap->UnBind();
		}

		BrushOutputShader->Stop();
		BrushOutputFB->UnBind();
	}

	if (LocalX > 0 && LocalZ > 0 && LocalX < 1.0 && LocalZ < 1.0)
	{
		BrushVisualShader->GetParameter("brushCenter")->UpdateData(glm::vec2(LocalX, LocalZ));
	}
	BrushVisualShader->GetParameter("brushSize")->UpdateData(BrushSize / (GetXSize() * 2.0f));

	BrushVisualFB->Bind();
	BrushVisualShader->Start();

	BrushVisualShader->LoadDataToGPU();
	FE_GL_ERROR(glViewport(0, 0, HeightMap->GetWidth(), HeightMap->GetHeight()));

	FE_GL_ERROR(glBindVertexArray(PlaneMesh->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	BrushVisualShader->Stop();
	BrushVisualFB->UnBind();

	bBrushVisualFBCleared = false;
}
// **************************** TERRAIN EDITOR TOOLS END ****************************

void FETerrain::SnapInstancedEntity(FEEntityInstanced* EntityToSnap)
{
	EntityToSnap->SnapToTerrain(this, &FETerrain::GetHeightAt);
	SnapedInstancedEntities.push_back(EntityToSnap);
}

void FETerrain::UpdateSnapedInstancedEntities()
{
	for (size_t i = 0; i < SnapedInstancedEntities.size(); i++)
	{
		// safety check
		if (SnapedInstancedEntities[i] == nullptr)
			continue;
		// if entity is still snapped
		if (SnapedInstancedEntities[i]->GetSnappedToTerrain() != this)
			continue;

		SnapedInstancedEntities[i]->Clear();
		SnapedInstancedEntities[i]->Populate(SnapedInstancedEntities[i]->SpawnInfo);
	}
}

void FETerrain::UnSnapInstancedEntity(FEEntityInstanced* EntityToUnSnap)
{
	for (size_t i = 0; i < SnapedInstancedEntities.size(); i++)
	{
		if (SnapedInstancedEntities[i] == EntityToUnSnap)
		{
			SnapedInstancedEntities.erase(SnapedInstancedEntities.begin() + i);
			break;
		}
	}

	EntityToUnSnap->UnSnapFromTerrain();
}

bool FETerrain::GetNextEmptyLayerSlot(size_t& NextEmptyLayerIndex)
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

FETerrainLayer* FETerrain::ActivateVacantLayerSlot(FEMaterial* Material)
{
	size_t LayerIndex = 0;
	if (!GetNextEmptyLayerSlot(LayerIndex))
	{
		LOG.Add("FETerrain::activateLayerSlot was not able to acquire vacant layer index", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	if (Material == nullptr)
	{
		LOG.Add("FETerrain::activateLayerSlot material is nullptr", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	if (!Material->IsCompackPacking())
	{
		LOG.Add("FETerrain::activateLayerSlot material is not compactly packed", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrain::activateLayerSlot with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	if (Layers[LayerIndex] != nullptr)
	{
		LOG.Add("FETerrain::activateLayerSlot on indicated layer slot layer is already active", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	Layers[LayerIndex] = new FETerrainLayer(std::string("Layer_") + std::to_string(LayerIndex));
	Layers[LayerIndex]->Material = Material;

	if (LayerIndex == 0)
		SetWireframeMode(false);

	return Layers[LayerIndex];
}

FETerrainLayer* FETerrain::GetLayerInSlot(const size_t LayerIndex)
{
	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrain::getLayerInSlot with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return nullptr;
	}

	return Layers[LayerIndex];
}

void FETerrain::DeleteLayerInSlot(const size_t LayerIndex)
{
	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrain::deleteLayerInSlot with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrain::deleteLayerInSlot on indicated layer slot layer is already inactive", "FE_LOG_RENDERING", FE_LOG_WARNING);
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

size_t FETerrain::GetBrushLayerIndex()
{
	return BrushLayerIndex;
}

void FETerrain::SetBrushLayerIndex(const size_t NewValue)
{
	if (NewValue >= FE_TERRAIN_MAX_LAYERS)
		return;

	BrushLayerIndex = NewValue;
}

void FETerrain::LoadLayersDataToGPU()
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
			GPULayersData[index + 3] = CurrentMaterial->GetRoughtness();
			// roughnessMapIntensity
			GPULayersData[index + 4] = CurrentMaterial->GetRoughtnessMapIntensity();
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

int FETerrain::LayersUsed()
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

bool FETerrain::UpdateLayerMapsRawData()
{
	if (TIME.EndTimeStamp(this->GetObjectID()) != -1.0)
	{
		if (TIME.EndTimeStamp(this->GetObjectID()) < 2000)
			return false;
	}

	TIME.BeginTimeStamp(this->GetObjectID());

	for (size_t i = 0; i < LayerMaps.size(); i++)
	{
		if (LayerMapsRawData[i] != nullptr)
		{
			delete LayerMapsRawData[i];
			LayerMapsRawData[i] = nullptr;
		}

		if (LayerMaps[i] == nullptr)
		{
			LayerMapsRawData[i] = nullptr;
		}
		else
		{
			LayerMapsRawData[i] = LayerMaps[i]->GetRawData();
		}
	}

	return true;
}

float FETerrain::GetLayerIntensityAt(glm::vec2 XZWorldPosition, const int LayerIndex)
{
	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrain::getLayerIntensityAt with out of bound \"layerIndex\"", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return 0.0f;
	}

	if (Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrain::getLayerIntensityAt on indicated layer slot layer is nullptr", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return 0.0f;
	}

	UpdateLayerMapsRawData();

	float LocalX = XZWorldPosition[0];
	float LocalZ = XZWorldPosition[1];

	LocalX -= FinalAABB.GetMin()[0];
	LocalZ -= FinalAABB.GetMin()[2];

	if (XSize == 0 || ZSize == 0)
		GetAABB();

	LocalX = LocalX / XSize;
	LocalZ = LocalZ / ZSize;

	if (LocalX > 0 && LocalZ > 0 && LocalX < 1.0 && LocalZ < 1.0)
	{
		const int TextureIndex = LayerIndex / FE_TERRAIN_LAYER_PER_TEXTURE;
		FETexture* Texture = this->LayerMaps[TextureIndex];
		LocalX = static_cast<float>(static_cast<int>(LocalX * Texture->GetWidth()));
		LocalZ = static_cast<float>(static_cast<int>(LocalZ * Texture->GetHeight()));

		const int Index = static_cast<int>(LocalZ * Texture->GetWidth() + LocalX) * 4 + LayerIndex % FE_TERRAIN_LAYER_PER_TEXTURE;

		if (LayerMapsRawData[TextureIndex] != nullptr)
			return LayerMapsRawData[TextureIndex][Index] / 255.0f;

	}

	return 0.0f;
}

void FETerrain::ConnectInstancedEntityToLayer(FEEntityInstanced* Entity, const int LayerIndex)
{
	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrain::connectInstancedEntityToLayer with out of bound \"layerIndex\"", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	if (Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrain::connectInstancedEntityToLayer on indicated layer slot layer is nullptr", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	for (size_t i = 0; i < SnapedInstancedEntities.size(); i++)
	{
		if (SnapedInstancedEntities[i]->GetObjectID() == Entity->GetObjectID())
		{
			Entity->ConnectToTerrainLayer(this, LayerIndex, &FETerrain::GetLayerIntensityAt);
			break;
		}
	}
}

void FETerrain::UnConnectInstancedEntityFromLayer(FEEntityInstanced* Entity)
{
	Entity->UnConnectFromTerrainLayer();
}