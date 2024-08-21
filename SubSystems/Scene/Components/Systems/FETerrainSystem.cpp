#include "FETerrainSystem.h"
#include "../../Renderer/FERenderer.h"
using namespace FocalEngine;

FETerrainSystem::FETerrainSystem()
{
	std::string EngineFolder = RESOURCE_MANAGER.EngineFolder;
	FEShader* TerrainShader = RESOURCE_MANAGER.CreateShader("FETerrainShader", RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_VS.glsl").c_str()).c_str(),
																			   RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_FS_GBUFFER.glsl").c_str()).c_str(),
																			   RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_TCS.glsl").c_str()).c_str(),
																			   RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_TES.glsl").c_str()).c_str(),
																			   RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//FE_Terrain_GS.glsl").c_str()).c_str(),
																			   nullptr,
																			   "5A3E4F5C13115856401F1D1C");

	RESOURCE_MANAGER.SetTagIternal(TerrainShader, ENGINE_RESOURCE_TAG);

	FEShader* ShadowMapTerrainShader = RESOURCE_MANAGER.CreateShader("FESMTerrainShader", RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_VS.glsl").c_str()).c_str(),
																						  RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_FS.glsl").c_str()).c_str(),
																						  RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_TCS.glsl").c_str()).c_str(),
																						  RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//ShadowMapShader//FE_SMTerrain_TES.glsl").c_str()).c_str(),
																						  nullptr, nullptr,
																					      "50064D3C4D0B537F0846274F");
	RESOURCE_MANAGER.SetTagIternal(ShadowMapTerrainShader, ENGINE_RESOURCE_TAG);

	const FEShaderParam ColorParam(glm::vec3(1.0f, 1.0f, 1.0f), "baseColor");
	RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/)->AddParameter(ColorParam);

	RESOURCE_MANAGER.SetTagIternal(RESOURCE_MANAGER.GetShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/), ENGINE_RESOURCE_TAG);

	FEShader* TerrainBrushOutput = RESOURCE_MANAGER.CreateShader("terrainBrushOutput", RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushOutput_VS.glsl").c_str()).c_str(),
																					   RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushOutput_FS.glsl").c_str()).c_str(),
																					   nullptr, nullptr, nullptr, nullptr,
																					   "49654A4A10604C2A1221426B");
	RESOURCE_MANAGER.SetTagIternal(TerrainBrushOutput, ENGINE_RESOURCE_TAG);

	FEShader* TerrainBrushVisual = RESOURCE_MANAGER.CreateShader("terrainBrushVisual", RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushVisual_VS.glsl").c_str()).c_str(),
																					   RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushVisual_FS.glsl").c_str()).c_str(),
																					   nullptr, nullptr, nullptr, nullptr,
																					   "40064B7B4287805B296E526E");
	RESOURCE_MANAGER.SetTagIternal(TerrainBrushVisual, ENGINE_RESOURCE_TAG);

	FEShader* TerrainLayersNormalize = RESOURCE_MANAGER.CreateShader("terrainLayersNormalize", RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_BrushOutput_VS.glsl").c_str()).c_str(),
																							   RESOURCE_MANAGER.LoadGLSL((EngineFolder + "CoreExtensions//StandardMaterial//TerrainMaterial//EditTools//FE_LayersNormalize_FS.glsl").c_str()).c_str(),
																							   nullptr, nullptr, nullptr, nullptr,
																							   "19294C00394A346A576F401C");
	RESOURCE_MANAGER.SetTagIternal(TerrainLayersNormalize, ENGINE_RESOURCE_TAG);

	BrushOutputShader = RESOURCE_MANAGER.GetShader("49654A4A10604C2A1221426B"/*"terrainBrushOutput"*/);
	LayersNormalizeShader = RESOURCE_MANAGER.GetShader("19294C00394A346A576F401C"/*"terrainLayersNormalize"*/);
	BrushVisualShader = RESOURCE_MANAGER.GetShader("40064B7B4287805B296E526E"/*"terrainBrushVisual"*/);

	PlaneMesh = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);

	RegisterOnComponentCallbacks();
	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FETerrainComponent>(TerrainComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FETerrainComponent>(TerrainComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FETerrainComponent>(DuplicateTerrainComponent);
}

void FETerrainSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FETerrainComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FETerrainComponent>(OnMyComponentDestroy);
}

void FETerrainSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FETerrainComponent>())
		return;

	FETerrainComponent& TerrainComponent = Entity->GetComponent<FETerrainComponent>();
	TerrainComponent.Shader = RESOURCE_MANAGER.GetShaderByName("FETerrainShader")[0];
}

void FETerrainSystem::DuplicateTerrainComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr)
		return;

	if (!SourceEntity->HasComponent<FETerrainComponent>())
		return;

	FETerrainComponent& OriginalTerrainComponent = SourceEntity->GetComponent<FETerrainComponent>();
	TargetEntity->AddComponent<FETerrainComponent>();

	FETerrainComponent& NewTerrainComponent = TargetEntity->GetComponent<FETerrainComponent>();
	NewTerrainComponent = OriginalTerrainComponent;

	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		if (OriginalTerrainComponent.Layers[i] != nullptr)
		{
			NewTerrainComponent.Layers[i] = new FETerrainLayer(OriginalTerrainComponent.Layers[i]->GetName());
			NewTerrainComponent.Layers[i]->Material = OriginalTerrainComponent.Layers[i]->Material;
		}
	}

	TERRAIN_SYSTEM.InitTerrainEditTools(TargetEntity);
}

void FETerrainSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FETerrainComponent>())
		return;

	FETerrainComponent& TerrainComponent = Entity->GetComponent<FETerrainComponent>();
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		delete TerrainComponent.Layers[i];
		TerrainComponent.Layers[i] = nullptr;
	}

	FE_GL_ERROR(glDeleteBuffers(1, &TerrainComponent.GPULayersDataBuffer));

	if (bIsSceneClearing)
		return;
	
	for (size_t i = 0; i < TerrainComponent.SnapedInstancedEntities.size(); i++)
	{
		FEInstancedComponent& InstancedComponent = TerrainComponent.SnapedInstancedEntities[i]->GetComponent<FEInstancedComponent>();
		InstancedComponent.UnSnapFromTerrain();
	}
	TerrainComponent.SnapedInstancedEntities.clear();
}

FETerrainSystem::~FETerrainSystem() {};

FEAABB FETerrainSystem::GetAABB(FEEntity* TerrainEntity)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
		return FEAABB();

	FETransformComponent& TransformComponent = TerrainEntity->GetComponent<FETransformComponent>();
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();
	
	if (TransformComponent.IsDirty() || TerrainComponent.bDirtyFlag)
	{
		if (TerrainComponent.bDirtyFlag)
			TerrainComponent.bDirtyFlag = true;

		if (TransformComponent.IsDirty())
			TransformComponent.SetDirtyFlag(false);

		FEAABB Result = TerrainComponent.AABB;
		// -0.5f it is a little hack, because this -0.5f should be made during tessellation.
		glm::vec3 Min = glm::vec3(-32.0f - 0.5f, TerrainComponent.AABB.GetMin()[1], -32.0f - 0.5f);
		glm::vec3 Max = glm::vec3(32.0f + 64.0f * (TerrainComponent.ChunkPerSide - 1) - 0.5f, TerrainComponent.AABB.GetMax()[1], 32.0f + 64.0f * (TerrainComponent.ChunkPerSide - 1) - 0.5f);
		Result = FEAABB(Min, Max);
		Result = FEAABB(glm::vec3(Result.GetMin()[0], Result.GetMin()[1] * 2 * TerrainComponent.HightScale - TerrainComponent.HightScale, Result.GetMin()[2]),
						glm::vec3(Result.GetMax()[0], Result.GetMax()[1] * 2 * TerrainComponent.HightScale - TerrainComponent.HightScale, Result.GetMax()[2]));
	
		TerrainComponent.FinalAABB = Result.Transform(TransformComponent.GetWorldMatrix());
	
		TerrainComponent.XSize = TerrainComponent.FinalAABB.GetMax()[0] - TerrainComponent.FinalAABB.GetMin()[0];
		TerrainComponent.ZSize = TerrainComponent.FinalAABB.GetMax()[2] - TerrainComponent.FinalAABB.GetMin()[2];
	}
	
	return TerrainComponent.FinalAABB;
}

FEAABB FETerrainSystem::GetPureAABB(FEEntity* TerrainEntity)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
		return FEAABB();

	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	FEAABB Result = TerrainComponent.AABB;
	glm::vec3 Min = glm::vec3(-32.0f, TerrainComponent.AABB.GetMin()[1], -32.0f);
	glm::vec3 Max = glm::vec3(32.0f + 64.0f * (TerrainComponent.ChunkPerSide - 1), TerrainComponent.AABB.GetMax()[1], 32.0f + 64.0f * (TerrainComponent.ChunkPerSide - 1));
	Result = FEAABB(Min, Max);
	Result = FEAABB(glm::vec3(Result.GetMin()[0], Result.GetMin()[1] * 2 * TerrainComponent.HightScale - TerrainComponent.HightScale, Result.GetMin()[2]),
					glm::vec3(Result.GetMax()[0], Result.GetMax()[1] * 2 * TerrainComponent.HightScale - TerrainComponent.HightScale, Result.GetMax()[2]));
	
	return Result;
}

// **************************** TERRAIN EDITOR TOOLS ****************************
bool FETerrainSystem::IsBrushActive()
{
	return bBrushActive;
}

void FETerrainSystem::SetBrushActive(const bool NewValue)
{
	bBrushActive = NewValue;
}

FE_TERRAIN_BRUSH_MODE FETerrainSystem::GetBrushMode()
{
	return BrushMode;
}

void FETerrainSystem::SetBrushMode(FEEntity* TerrainEntity, const FE_TERRAIN_BRUSH_MODE NewValue)
{
	if (NewValue == FE_TERRAIN_BRUSH_NONE)
	{
		TerrainEntityIDWithBrushModeOn = "";
	}
	else
	{
		if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
			return;

		TerrainEntityIDWithBrushModeOn = TerrainEntity->GetObjectID();
	}

	BrushMode = NewValue;
}

float FETerrainSystem::GetBrushSize()
{
	return BrushSize;
}

void FETerrainSystem::SetBrushSize(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.001f;

	if (NewValue > 1000.0f)
		NewValue = 1000.0f;

	BrushSize = NewValue;
}

float FETerrainSystem::GetBrushIntensity()
{
	return BrushIntensity;
}

void FETerrainSystem::SetBrushIntensity(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.001f;

	if (NewValue > 1000.0f)
		NewValue = 1000.0f;

	BrushIntensity = NewValue;
}

size_t FETerrainSystem::GetBrushLayerIndex()
{
	return BrushLayerIndex;
}

void FETerrainSystem::SetBrushLayerIndex(const size_t NewValue)
{
	if (NewValue >= FE_TERRAIN_MAX_LAYERS)
		return;

	BrushLayerIndex = NewValue;
}

// ********************************** PointOnTerrain **********************************
float FETerrainSystem::GetHeightAt(FEEntity* TerrainEntity, glm::vec2 XZWorldPosition)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
		return -FLT_MAX;

	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	float LocalX = XZWorldPosition[0];
	float LocalZ = XZWorldPosition[1];

	LocalX -= TerrainComponent.FinalAABB.GetMin()[0];
	LocalZ -= TerrainComponent.FinalAABB.GetMin()[2];

	if (TerrainComponent.XSize == 0 || TerrainComponent.ZSize == 0)
		GetAABB(TerrainEntity);

	LocalX = LocalX / TerrainComponent.XSize;
	LocalZ = LocalZ / TerrainComponent.ZSize;

	if (LocalX > 0 && LocalZ > 0 && LocalX < 1.0 && LocalZ < 1.0)
	{
		LocalX = static_cast<float>(static_cast<int>(LocalX * TerrainComponent.HeightMap->GetWidth()));
		LocalZ = static_cast<float>(static_cast<int>(LocalZ * TerrainComponent.HeightMap->GetHeight()));

		const int Index = static_cast<int>(LocalZ * TerrainComponent.HeightMap->GetWidth() + LocalX);
		return (TerrainComponent.HeightMapArray[Index] * 2 * TerrainComponent.HightScale - TerrainComponent.HightScale) * TerrainEntity->GetComponent<FETransformComponent>().GetScale()[1] + TerrainEntity->GetComponent<FETransformComponent>().GetPosition()[1];
	}

	return -FLT_MAX;
}

glm::dvec3 FETerrainSystem::GetPointOnTerrain(FEEntity* TerrainEntity, const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection, const float StartDistance, const float EndDistance)
{
	if (IntersectionInRange(TerrainEntity, StartDistance, EndDistance, MouseRayStart, MouseRayDirection))
	{
		const glm::dvec3 PointOnTerrain = BinarySearch(TerrainEntity, 0, StartDistance, EndDistance, MouseRayStart, MouseRayDirection);
		// if point is not above terrain, point could be above because isUnderGround returning true if we go out of terrain bounds to fix some bugs.
		if ((GetHeightAt(TerrainEntity, glm::vec2(PointOnTerrain.x, PointOnTerrain.z)) + 1.0f) > PointOnTerrain.y)
			return PointOnTerrain;

		return glm::dvec3(FLT_MAX);
	}

	return glm::dvec3(FLT_MAX);
}

bool FETerrainSystem::IsUnderGround(FEEntity* TerrainEntity, const glm::dvec3 TestPoint)
{
	const float Height = GetHeightAt(TerrainEntity, glm::vec2(TestPoint.x, TestPoint.z));
	// If we go outside terrain.
	if (Height == -FLT_MAX)
		return true;

	return TestPoint.y < Height ? true : false;
}

glm::dvec3 FETerrainSystem::GetPointOnRay(FEEntity* TerrainEntity, const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection, const float Distance)
{
	const glm::dvec3 Start = glm::dvec3(MouseRayStart.x, MouseRayStart.y, MouseRayStart.z);
	const glm::dvec3 ScaledRay = glm::dvec3(MouseRayDirection.x * Distance, MouseRayDirection.y * Distance, MouseRayDirection.z * Distance);

	return Start + ScaledRay;
}

bool FETerrainSystem::IntersectionInRange(FEEntity* TerrainEntity, const float Start, const float Finish, const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection)
{
	const glm::dvec3 StartPoint = GetPointOnRay(TerrainEntity, MouseRayStart, MouseRayDirection, Start);
	const glm::dvec3 EndPoint = GetPointOnRay(TerrainEntity, MouseRayStart, MouseRayDirection, Finish);
	return !IsUnderGround(TerrainEntity, StartPoint) && IsUnderGround(TerrainEntity, EndPoint) ? true : false;
}

glm::dvec3 FETerrainSystem::BinarySearch(FEEntity* TerrainEntity, const int Count, const float Start, const float Finish, const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection)
{
	const int RecursionCount = 200;
	const float Half = Start + ((Finish - Start) / 2.0f);
	if (Count >= RecursionCount)
	{
		const glm::dvec3 EndPoint = GetPointOnRay(TerrainEntity, MouseRayStart, MouseRayDirection, Half);
		if (GetHeightAt(TerrainEntity, glm::vec2(EndPoint.x, EndPoint.z)) != -1.0f)
		{
			return EndPoint;
		}
		else
		{
			return glm::dvec3(FLT_MAX);
		}
	}

	if (IntersectionInRange(TerrainEntity, Start, Half, MouseRayStart, MouseRayDirection))
	{
		return BinarySearch(TerrainEntity, Count + 1, Start, Half, MouseRayStart, MouseRayDirection);
	}
	else
	{
		return BinarySearch(TerrainEntity, Count + 1, Half, Finish, MouseRayStart, MouseRayDirection);
	}
}

void FETerrainSystem::SnapInstancedEntity(FEEntity* TerrainEntity, FEEntity* EntityToSnap)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::SnapInstancedEntity TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	if (!EntityToSnap->HasComponent<FEInstancedComponent>())
	{
		LOG.Add("FETerrainSystem::SnapInstancedEntity EntityToSnap does not have FEInstancedComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	EntityToSnap->GetComponent<FEInstancedComponent>().SnapToTerrain(TerrainEntity);
	TerrainEntity->GetComponent<FETerrainComponent>().SnapedInstancedEntities.push_back(EntityToSnap);
}

// There should be third system to manage that.
#include "../SubSystems/Scene/Components/Systems/FEInstancedSystem.h"
void FETerrainSystem::UpdateSnapedInstancedEntities(FEEntity* TerrainEntity)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::SnapInstancedEntity TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();
	for (size_t i = 0; i < TerrainComponent.SnapedInstancedEntities.size(); i++)
	{
		if (TerrainComponent.SnapedInstancedEntities[i] == nullptr)
			continue;

		// If entity is still snapped.
		if (TerrainComponent.SnapedInstancedEntities[i]->GetComponent<FEInstancedComponent>().GetSnappedToTerrain() != TerrainEntity)
			continue;

		INSTANCED_RENDERING_SYSTEM.ClearInstance(TerrainComponent.SnapedInstancedEntities[i]);
		INSTANCED_RENDERING_SYSTEM.PopulateInstance(TerrainComponent.SnapedInstancedEntities[i], TerrainComponent.SnapedInstancedEntities[i]->GetComponent<FEInstancedComponent>().SpawnInfo);
		//SnapedInstancedEntities[i]->Clear();
		//SnapedInstancedEntities[i]->Populate(SnapedInstancedEntities[i]->SpawnInfo);
	}
}

void FETerrainSystem::UnSnapInstancedEntity(FEEntity* TerrainEntity, FEEntity* EntityToUnSnap)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::SnapInstancedEntity TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();
	for (size_t i = 0; i < TerrainComponent.SnapedInstancedEntities.size(); i++)
	{
		if (TerrainComponent.SnapedInstancedEntities[i] == EntityToUnSnap)
		{
			TerrainComponent.SnapedInstancedEntities.erase(TerrainComponent.SnapedInstancedEntities.begin() + i);
			break;
		}
	}

	EntityToUnSnap->GetComponent<FEInstancedComponent>().UnSnapFromTerrain();
}

void FETerrainSystem::UpdateBrush(const glm::dvec3 MouseRayStart, const glm::dvec3 MouseRayDirection)
{
	if (TerrainEntityIDWithBrushModeOn.empty())
		return;

	FEObject* TerrainObject = OBJECT_MANAGER.GetFEObject(TerrainEntityIDWithBrushModeOn);
	if (TerrainObject == nullptr)
	{
		TerrainEntityIDWithBrushModeOn = "";
		return;
	}

	FEEntity* TerrainEntityWithBrushModeOn = reinterpret_cast<FEEntity*>(OBJECT_MANAGER.GetFEObject(TerrainEntityIDWithBrushModeOn));
	FETerrainComponent& TerrainComponent = TerrainEntityWithBrushModeOn->GetComponent<FETerrainComponent>();

	if (TerrainComponent.BrushVisualFB == nullptr)
		return;

	if (GetBrushMode() == FE_TERRAIN_BRUSH_NONE)
	{
		if (bBrushVisualFBCleared)
			return;

		// Get current clear color.
		GLfloat BackgroundColor[4];
		glGetFloatv(GL_COLOR_CLEAR_VALUE, BackgroundColor);

		TerrainComponent.BrushVisualFB->Bind();
		FE_GL_ERROR(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT));
		TerrainComponent.BrushVisualFB->UnBind();
		bBrushVisualFBCleared = true;

		FE_GL_ERROR(glClearColor(BackgroundColor[0], BackgroundColor[1], BackgroundColor[2], BackgroundColor[3]));

		return;
	}

	if (bCPUHeightInfoDirtyFlag)
	{
		if (std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::system_clock::now() - LastChangesTimeStamp).count() * 1000.0f > WaitBeforeUpdateMs)
		{
			bCPUHeightInfoDirtyFlag = false;
			UpdateCPUHeightInfo(TerrainEntityWithBrushModeOn);
			UpdateSnapedInstancedEntities(TerrainEntityWithBrushModeOn);
		}
	}

	const float Range = TerrainComponent.GetXSize() * 2.0f;
	const glm::dvec3 CurrentTerrainPoint = GetPointOnTerrain(TerrainEntityWithBrushModeOn, MouseRayStart, MouseRayDirection, 0, Range);

	float LocalX = static_cast<float>(CurrentTerrainPoint.x);
	float LocalZ = static_cast<float>(CurrentTerrainPoint.z);

	LocalX -= GetAABB(TerrainEntityWithBrushModeOn).GetMin()[0];
	LocalZ -= GetAABB(TerrainEntityWithBrushModeOn).GetMin()[2];

	LocalX = LocalX / TerrainComponent.GetXSize();
	LocalZ = LocalZ / TerrainComponent.GetZSize();

	if (IsBrushActive())
	{
		LastChangesTimeStamp = std::chrono::system_clock::now();
		bCPUHeightInfoDirtyFlag = true;
		TerrainComponent.HeightMap->SetDirtyFlag(true);

		if (LocalX > 0 && LocalZ > 0 && LocalX < 1.0 && LocalZ < 1.0)
		{
			BrushOutputShader->UpdateParameterData("brushCenter", glm::vec2(LocalX, LocalZ));
		}

		BrushOutputShader->UpdateParameterData("brushSize", BrushSize / (TerrainComponent.GetXSize() * 2.0f));
		BrushOutputShader->UpdateParameterData("brushMode", static_cast<float>(GetBrushMode()));

		TerrainComponent.BrushOutputFB->SetColorAttachment(TerrainComponent.HeightMap);
		BrushOutputShader->UpdateParameterData("brushIntensity", BrushIntensity / 10.0f);

		TerrainComponent.BrushOutputFB->Bind();
		BrushOutputShader->Start();
		
		if (BrushMode == FE_TERRAIN_BRUSH_LAYER_DRAW)
		{
			TerrainComponent.LayerMaps[0]->Bind(0);
			RENDERER.SetViewport(0, 0, TerrainComponent.LayerMaps[0]->GetWidth(), TerrainComponent.LayerMaps[0]->GetHeight());

			BrushOutputShader->UpdateParameterData("brushIntensity", BrushIntensity * 5.0f);
			BrushOutputShader->UpdateParameterData("layerIndex", static_cast<float>(GetBrushLayerIndex()));
			TerrainComponent.BrushOutputFB->SetColorAttachment(TerrainComponent.LayerMaps[0]);
			BrushOutputShader->LoadDataToGPU();

			FE_GL_ERROR(glBindVertexArray(PlaneMesh->GetVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));

			TerrainComponent.LayerMaps[1]->Bind(0);
			BrushOutputShader->UpdateParameterData("layerIndex", static_cast<float>(GetBrushLayerIndex() - 4.0f));
			TerrainComponent.BrushOutputFB->SetColorAttachment(TerrainComponent.LayerMaps[1]);
			BrushOutputShader->LoadDataToGPU();

			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			BrushOutputShader->Stop();

			// Normalize all layers values to add up to 1.0
			LayersNormalizeShader->Start();
			TerrainComponent.LayerMaps[0]->Bind(0);
			TerrainComponent.LayerMaps[1]->Bind(1);

			TerrainComponent.BrushOutputFB->SetColorAttachment(TerrainComponent.LayerMaps[0], 0);
			TerrainComponent.BrushOutputFB->SetColorAttachment(TerrainComponent.LayerMaps[1], 1);

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
			TerrainComponent.HeightMap->Bind(0);

			RENDERER.SetViewport(0, 0, TerrainComponent.HeightMap->GetWidth(), TerrainComponent.HeightMap->GetHeight());

			FE_GL_ERROR(glBindVertexArray(PlaneMesh->GetVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			TerrainComponent.HeightMap->UnBind();
		}

		BrushOutputShader->Stop();
		TerrainComponent.BrushOutputFB->UnBind();
	}

	if (LocalX > 0 && LocalZ > 0 && LocalX < 1.0 && LocalZ < 1.0)
	{
		BrushVisualShader->UpdateParameterData("brushCenter", glm::vec2(LocalX, LocalZ));
	}
	BrushVisualShader->UpdateParameterData("brushSize", BrushSize / (TerrainComponent.GetXSize() * 2.0f));

	TerrainComponent.BrushVisualFB->Bind();
	BrushVisualShader->Start();

	BrushVisualShader->LoadDataToGPU();
	RENDERER.SetViewport(0, 0, TerrainComponent.HeightMap->GetWidth(), TerrainComponent.HeightMap->GetHeight());

	FE_GL_ERROR(glBindVertexArray(PlaneMesh->GetVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, PlaneMesh->GetVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	BrushVisualShader->Stop();
	TerrainComponent.BrushVisualFB->UnBind();

	bBrushVisualFBCleared = false;
}
// **************************** TERRAIN EDITOR TOOLS END ****************************

void FETerrainSystem::UpdateCPUHeightInfo(FEEntity* TerrainEntity)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::SnapInstancedEntity TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	/*FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, heightMap->getTextureID()));

	size_t rawDataLenght = heightMap->getWidth() * heightMap->getHeight() * 2;
	unsigned char* rawData = new unsigned char[rawDataLenght];
	glPixelStorei(GL_PACK_ALIGNMENT, 2);
	FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_SHORT, rawData));
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	heightMap->unBind();*/

	size_t RawDataLenght;
	unsigned char* RawData = TerrainComponent.HeightMap->GetRawData(&RawDataLenght);

	float Max = FLT_MIN;
	float Min = FLT_MAX;
	int Iterator = 0;
	for (size_t i = 0; i < RawDataLenght; i += 2)
	{
		const unsigned short Temp = *(unsigned short*)(&RawData[i]);
		TerrainComponent.HeightMapArray[Iterator] = Temp / static_cast<float>(0xFFFF);

		if (Max < TerrainComponent.HeightMapArray[Iterator])
			Max = TerrainComponent.HeightMapArray[Iterator];

		if (Min > TerrainComponent.HeightMapArray[Iterator])
			Min = TerrainComponent.HeightMapArray[Iterator];

		Iterator++;
	}

	const glm::vec3 MinPoint = glm::vec3(-1.0f, Min, -1.0f);
	const glm::vec3 MaxPoint = glm::vec3(1.0f, Max, 1.0f);
	TerrainComponent.AABB = FEAABB(MinPoint, MaxPoint);
	FETransformComponent& Transform = TerrainEntity->GetComponent<FETransformComponent>();
	Transform.SetDirtyFlag(true);

	delete[] RawData;
}

void FETerrainSystem::ConnectInstancedEntityToLayer(FEEntity* TerrainEntity, FEEntity* EntityWithTerrainComponent, const int LayerIndex)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::SnapInstancedEntity TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainSystem::ConnectInstancedEntityToLayer with out of bound \"layerIndex\"", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	if (TerrainComponent.Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrainSystem::ConnectInstancedEntityToLayer on indicated layer slot layer is nullptr", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	if (!EntityWithTerrainComponent->HasComponent<FEInstancedComponent>())
	{
		LOG.Add("FETerrainSystem::ConnectInstancedEntityToLayer EntityWithTerrainComponent does not have FEInstancedComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	for (size_t i = 0; i < TerrainComponent.SnapedInstancedEntities.size(); i++)
	{
		if (TerrainComponent.SnapedInstancedEntities[i]->GetObjectID() == EntityWithTerrainComponent->GetObjectID())
		{
			//EntityWithTerrainComponent->GetComponent<FEInstancedComponent>().ConnectToTerrainLayer(this, LayerIndex, &FETerrainComponent::GetLayerIntensityAt);
			EntityWithTerrainComponent->GetComponent<FEInstancedComponent>().ConnectToTerrainLayer(TerrainEntity, LayerIndex);
			break;
		}
	}
}

void FETerrainSystem::UnConnectInstancedEntityFromLayer(FEEntity* Entity)
{
	Entity->GetComponent<FEInstancedComponent>().UnConnectFromTerrainLayer();
}

void FETerrainSystem::LoadHeightMap(std::string FileName, FEEntity* TerrainEntity)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::InitTerrainEditTools TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	FETexture* NewTexture = nullptr;
	std::string FileExtension = FILE_SYSTEM.GetFileExtension(FileName);
	if (FileExtension == ".texture")
	{
		NewTexture = RESOURCE_MANAGER.LoadFETexture(FileName.c_str());
	}
	else if (FileExtension == ".png")
	{
		NewTexture = RESOURCE_MANAGER.LoadPNGTexture(FileName.c_str());
	}
	else
	{
		LOG.Add("FETerrainSystem::LoadHeightMap FileExtension is not supported", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	SetHeightMap(NewTexture, TerrainEntity);
}

void FETerrainSystem::SetHeightMap(FETexture* HightMap, FEEntity* TerrainEntity)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::InitTerrainEditTools TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (HightMap == nullptr || HightMap->GetInternalFormat() != GL_R16)
		return;

	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, HightMap->GetTextureID()));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	FE_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	size_t TextureDataSize = 0;
	unsigned short* TextureData = reinterpret_cast<unsigned short*>(HightMap->GetRawData(&TextureDataSize));
	RESOURCE_MANAGER.SetTag(HightMap, TERRAIN_SYSTEM_RESOURCE_TAG);
	if (TextureData == nullptr || TextureDataSize == 0)
		return;

	size_t ElemntCount = TextureDataSize / sizeof(unsigned short);
	TerrainComponent.HeightMapArray.resize(ElemntCount);

	float Max = FLT_MIN;
	float Min = FLT_MAX;
	for (size_t i = 0; i < ElemntCount; i++)
	{
		const unsigned short TemporaryValue = TextureData[i];
		TerrainComponent.HeightMapArray[i] = TemporaryValue / static_cast<float>(0xFFFF);

		if (Max < TerrainComponent.HeightMapArray[i])
			Max = TerrainComponent.HeightMapArray[i];

		if (Min > TerrainComponent.HeightMapArray[i])
			Min = TerrainComponent.HeightMapArray[i];
	}

	const glm::vec3 MinPoint = glm::vec3(-1.0f, Min, -1.0f);
	const glm::vec3 MaxPoint = glm::vec3(1.0f, Max, 1.0f);
	TerrainComponent.AABB = FEAABB(MinPoint, MaxPoint);

	TerrainComponent.HeightMap = HightMap;
	InitTerrainEditTools(TerrainEntity);
	UpdateCPUHeightInfo(TerrainEntity);
}

void FETerrainSystem::InitTerrainEditTools(FEEntity* TerrainEntity)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::InitTerrainEditTools TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	TerrainComponent.BrushOutputFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, 32, 32);
	delete TerrainComponent.BrushOutputFB->GetColorAttachment();
	TerrainComponent.BrushOutputFB->SetColorAttachment(TerrainComponent.HeightMap);

	TerrainComponent.BrushVisualFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT, TerrainComponent.HeightMap->GetWidth(), TerrainComponent.HeightMap->GetHeight());
	TerrainComponent.ProjectedMap = TerrainComponent.BrushVisualFB->GetColorAttachment();
}

bool FETerrainSystem::UpdateLayerMapsRawData(FEEntity* TerrainEntity)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::UpdateLayerMapsRawData TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return false;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (TIME.EndTimeStamp(TerrainEntity->GetObjectID()) != -1.0)
	{
		if (TIME.EndTimeStamp(TerrainEntity->GetObjectID()) < 2000)
			return false;
	}

	TIME.BeginTimeStamp(TerrainEntity->GetObjectID());

	for (size_t i = 0; i < TerrainComponent.LayerMaps.size(); i++)
	{
		if (TerrainComponent.LayerMapsRawData[i] != nullptr)
		{
			delete TerrainComponent.LayerMapsRawData[i];
			TerrainComponent.LayerMapsRawData[i] = nullptr;
		}

		if (TerrainComponent.LayerMaps[i] == nullptr)
		{
			TerrainComponent.LayerMapsRawData[i] = nullptr;
		}
		else
		{
			TerrainComponent.LayerMapsRawData[i] = TerrainComponent.LayerMaps[i]->GetRawData();
		}
	}

	return true;
}

float FETerrainSystem::GetLayerIntensityAt(FEEntity* TerrainEntity, glm::vec2 XZWorldPosition, const int LayerIndex)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::GetLayerIntensityAt TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return 0.0f;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainSystem::GetLayerIntensityAt with out of bound \"layerIndex\"", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return 0.0f;
	}

	if (TerrainComponent.Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrainSystem::GetLayerIntensityAt on indicated layer slot layer is nullptr", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return 0.0f;
	}

	UpdateLayerMapsRawData(TerrainEntity);

	float LocalX = XZWorldPosition[0];
	float LocalZ = XZWorldPosition[1];

	LocalX -= TerrainComponent.FinalAABB.GetMin()[0];
	LocalZ -= TerrainComponent.FinalAABB.GetMin()[2];

	if (TerrainComponent.XSize == 0 || TerrainComponent.ZSize == 0)
		GetAABB(TerrainEntity);

	LocalX = LocalX / TerrainComponent.XSize;
	LocalZ = LocalZ / TerrainComponent.ZSize;

	if (LocalX > 0 && LocalZ > 0 && LocalX < 1.0 && LocalZ < 1.0)
	{
		const int TextureIndex = LayerIndex / FE_TERRAIN_LAYER_PER_TEXTURE;
		FETexture* Texture = TerrainComponent.LayerMaps[TextureIndex];
		LocalX = static_cast<float>(static_cast<int>(LocalX * Texture->GetWidth()));
		LocalZ = static_cast<float>(static_cast<int>(LocalZ * Texture->GetHeight()));

		const int Index = static_cast<int>(LocalZ * Texture->GetWidth() + LocalX) * 4 + LayerIndex % FE_TERRAIN_LAYER_PER_TEXTURE;

		if (TerrainComponent.LayerMapsRawData[TextureIndex] != nullptr)
			return TerrainComponent.LayerMapsRawData[TextureIndex][Index] / 255.0f;

	}

	return 0.0f;
}

void FETerrainSystem::FillTerrainLayerMaskWithRawData(FEEntity* TerrainEntity, const unsigned char* RawData, const size_t LayerIndex)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::FillTerrainLayerMaskWithRawData TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (RawData == nullptr)
	{
		LOG.Add("FETerrainSystem::fillTerrainLayerMaskWithRawData with nullptr rawData", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainSystem::fillTerrainLayerMaskWithRawData with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	int Index = 0;
	const size_t TextureWidht = TerrainComponent.LayerMaps[0]->GetWidth();
	const size_t TextureHeight = TerrainComponent.LayerMaps[0]->GetHeight();

	std::vector<unsigned char*> LayersPerTextureData;
	LayersPerTextureData.resize(2);
	LayersPerTextureData[0] = TerrainComponent.LayerMaps[0]->GetRawData();
	LayersPerTextureData[1] = TerrainComponent.LayerMaps[1]->GetRawData();

	std::vector<unsigned char*> LayersPerChannelData;
	LayersPerChannelData.resize(FE_TERRAIN_MAX_LAYERS);
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		LayersPerChannelData[i] = new unsigned char[TextureWidht * TextureHeight];
	}

	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		Index = 0;
		if (LayerIndex == i)
		{
			for (size_t j = 0; j < TextureWidht * TextureHeight; j++)
			{
				LayersPerChannelData[i][Index++] = RawData[j];
			}
		}
		else
		{
			for (size_t j = i % FE_TERRAIN_LAYER_PER_TEXTURE; j < TextureWidht * TextureHeight * 4; j += 4)
			{
				LayersPerChannelData[i][Index++] = LayersPerTextureData[i / FE_TERRAIN_LAYER_PER_TEXTURE][j];
			}
		}
	}

	std::vector<unsigned char*> FinalTextureChannels;
	FinalTextureChannels.resize(2);
	FinalTextureChannels[0] = new unsigned char[TextureWidht * TextureHeight * 4];
	FinalTextureChannels[1] = new unsigned char[TextureWidht * TextureHeight * 4];

	Index = 0;

	int* AllChannelsPixels = new int[8];

	for (size_t i = 0; i < TextureWidht * TextureHeight * 4; i += 4)
	{
		float sum = 0.0f;
		for (size_t j = 0; j < 8; j++)
		{
			AllChannelsPixels[j] = LayersPerChannelData[j][Index];
		}

		FinalTextureChannels[0][i] = static_cast<unsigned char>(AllChannelsPixels[0]);
		FinalTextureChannels[0][i + 1] = static_cast<unsigned char>(AllChannelsPixels[1]);
		FinalTextureChannels[0][i + 2] = static_cast<unsigned char>(AllChannelsPixels[2]);
		FinalTextureChannels[0][i + 3] = static_cast<unsigned char>(AllChannelsPixels[3]);

		FinalTextureChannels[1][i] = static_cast<unsigned char>(AllChannelsPixels[4]);
		FinalTextureChannels[1][i + 1] = static_cast<unsigned char>(AllChannelsPixels[5]);
		FinalTextureChannels[1][i + 2] = static_cast<unsigned char>(AllChannelsPixels[6]);
		FinalTextureChannels[1][i + 3] = static_cast<unsigned char>(AllChannelsPixels[7]);

		Index++;
	}

	const int MaxDimention = std::max(static_cast<int>(TextureWidht), static_cast<int>(TextureHeight));
	const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);

	TerrainComponent.LayerMaps[0]->UpdateRawData(FinalTextureChannels[0], MipCount);
	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	TerrainComponent.LayerMaps[1]->UpdateRawData(FinalTextureChannels[1], MipCount);
	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	delete[] LayersPerTextureData[0];
	delete[] LayersPerTextureData[1];
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		delete[]LayersPerChannelData[i];
	}

	delete[] FinalTextureChannels[0];
	delete[] FinalTextureChannels[1];
	delete[] AllChannelsPixels;
}

void FETerrainSystem::FillTerrainLayerMask(FEEntity* TerrainEntity, const size_t LayerIndex)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::FillTerrainLayerMask TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainSystem::fillTerrainLayerMask with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (TerrainComponent.Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrainSystem::fillTerrainLayerMask on indicated layer slot layer is nullptr", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	FETexture* CorrectLayer = TerrainComponent.LayerMaps[LayerIndex / FE_TERRAIN_LAYER_PER_TEXTURE];
	const size_t TextureWidht = CorrectLayer->GetWidth();
	const size_t TextureHeight = CorrectLayer->GetHeight();
	unsigned char* FilledChannel = new unsigned char[TextureWidht * TextureHeight];
	for (size_t i = 0; i < TextureWidht * TextureHeight; i++)
	{
		FilledChannel[i] = 255;
	}

	FillTerrainLayerMaskWithRawData(TerrainEntity, FilledChannel, LayerIndex);
	delete[] FilledChannel;
}

void FETerrainSystem::ActivateVacantLayerSlot(FEEntity* TerrainEntity, FEMaterial* Material)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::ActivateVacantLayerSlot TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	// If this terrain does not have LayerMaps we would create them.
	if (TerrainComponent.LayerMaps[0] == nullptr)
	{
		/*FETexture* NewTexture = CreateTexture();*/
		int TextureWidth = FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION;
		int TextureHeight = FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION;
		//NewTexture->InternalFormat = GL_RGBA;

		std::vector<unsigned char> RawData;
		const size_t DataLenght = TextureWidth * TextureHeight * 4;
		RawData.resize(DataLenght);
		for (size_t i = 0; i < DataLenght; i++)
		{
			RawData[i] = 0;
		}

		//FETexture* NewTexture = RESOURCE_MANAGER.RawDataToFETexture(RawData.data(), TextureWidth, TextureHeight, -1, GL_RGBA);

		/*FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));
		FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGBA, NewTexture->Width, NewTexture->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, RawData.data());

		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));*/

		TerrainComponent.LayerMaps[0] = RESOURCE_MANAGER.RawDataToFETexture(RawData.data(), TextureWidth, TextureHeight, -1, GL_RGBA);
		TerrainComponent.LayerMaps[0]->SetName("Terrain_LayerMap0");
		RESOURCE_MANAGER.SetTag(TerrainComponent.LayerMaps[0], TERRAIN_SYSTEM_RESOURCE_TAG);

		/*NewTexture = CreateTexture();
		NewTexture->Width = FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION;
		NewTexture->Height = FE_TERRAIN_STANDARD_LAYER_MAP_RESOLUTION;
		NewTexture->InternalFormat = GL_RGBA;

		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, NewTexture->TextureID));
		FETexture::GPUAllocateTeture(GL_TEXTURE_2D, 0, GL_RGBA, NewTexture->Width, NewTexture->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, RawData.data());

		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));*/

		TerrainComponent.LayerMaps[1] = RESOURCE_MANAGER.RawDataToFETexture(RawData.data(), TextureWidth, TextureHeight, -1, GL_RGBA);
		TerrainComponent.LayerMaps[1]->SetName("Terrain_LayerMap1");
		RESOURCE_MANAGER.SetTag(TerrainComponent.LayerMaps[1], TERRAIN_SYSTEM_RESOURCE_TAG);

		TerrainComponent.ActivateVacantLayerSlot(Material);
		FillTerrainLayerMask(TerrainEntity, 0);
		return;
	}

	TerrainComponent.ActivateVacantLayerSlot(Material);
}

void FETerrainSystem::ClearTerrainLayerMask(FEEntity* TerrainEntity, size_t LayerIndex)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::ClearTerrainLayerMask TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FEResourceManager::ClearTerrainLayerMask with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (TerrainComponent.Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FEResourceManager::ClearTerrainLayerMask on indicated layer slot layer is nullptr", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	FETexture* CorrectLayer = TerrainComponent.LayerMaps[LayerIndex / FE_TERRAIN_LAYER_PER_TEXTURE];
	const size_t TextureWidht = CorrectLayer->GetWidth();
	const size_t TextureHeight = CorrectLayer->GetHeight();
	unsigned char* FilledChannel = new unsigned char[TextureWidht * TextureHeight];
	for (size_t i = 0; i < TextureWidht * TextureHeight; i++)
	{
		FilledChannel[i] = 0;
	}

	FillTerrainLayerMaskWithRawData(TerrainEntity, FilledChannel, LayerIndex);
	delete[] FilledChannel;
}

void FETerrainSystem::DeleteTerrainLayerMask(FEEntity* TerrainEntity, size_t LayerIndex)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::DeleteTerrainLayerMask TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainSystem::DeleteTerrainLayerMask with out of bound \"layerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (TerrainComponent.Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrainSystem::DeleteTerrainLayerMask on indicated layer slot layer is nullptr", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	ClearTerrainLayerMask(TerrainEntity, LayerIndex);

	std::vector<unsigned char*> LayersPerTextureData;
	LayersPerTextureData.resize(2);
	size_t RawDataSize = 0;
	LayersPerTextureData[0] = TerrainComponent.LayerMaps[0]->GetRawData(&RawDataSize);
	LayersPerTextureData[1] = TerrainComponent.LayerMaps[1]->GetRawData();

	std::vector<unsigned char*> AllLayers;
	AllLayers.resize(8);
	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		AllLayers[i] = new unsigned char[RawDataSize / 4];
	}

	// Gathering channels from 2 textures.
	int ChannelIndex = 0;
	for (size_t i = 0; i < RawDataSize; i += 4)
	{
		AllLayers[0][ChannelIndex] = LayersPerTextureData[0][i];
		AllLayers[1][ChannelIndex] = LayersPerTextureData[0][i + 1];
		AllLayers[2][ChannelIndex] = LayersPerTextureData[0][i + 2];
		AllLayers[3][ChannelIndex] = LayersPerTextureData[0][i + 3];

		AllLayers[4][ChannelIndex] = LayersPerTextureData[1][i];
		AllLayers[5][ChannelIndex] = LayersPerTextureData[1][i + 1];
		AllLayers[6][ChannelIndex] = LayersPerTextureData[1][i + 2];
		AllLayers[7][ChannelIndex] = LayersPerTextureData[1][i + 3];

		ChannelIndex++;
	}

	// Shifting existing layers masks to place where was deleted mask.
	for (size_t i = LayerIndex; i < FE_TERRAIN_MAX_LAYERS - 1; i++)
	{
		for (size_t j = 0; j < RawDataSize / 4; j++)
		{
			AllLayers[i][j] = AllLayers[i + 1][j];
		}
	}

	unsigned char* FirstTextureData = new unsigned char[RawDataSize];
	unsigned char* SecondTextureData = new unsigned char[RawDataSize];

	// Putting individual channels back to 2 distinct textures.
	ChannelIndex = 0;
	for (size_t i = 0; i < RawDataSize; i += 4)
	{
		FirstTextureData[i] = AllLayers[0][ChannelIndex];
		FirstTextureData[i + 1] = AllLayers[1][ChannelIndex];
		FirstTextureData[i + 2] = AllLayers[2][ChannelIndex];
		FirstTextureData[i + 3] = AllLayers[3][ChannelIndex];

		SecondTextureData[i] = AllLayers[4][ChannelIndex];
		SecondTextureData[i + 1] = AllLayers[5][ChannelIndex];
		SecondTextureData[i + 2] = AllLayers[6][ChannelIndex];
		SecondTextureData[i + 3] = AllLayers[7][ChannelIndex];

		ChannelIndex++;
	}

	const int MaxDimention = std::max(TerrainComponent.LayerMaps[0]->GetWidth(), TerrainComponent.LayerMaps[0]->GetHeight());
	const size_t MipCount = static_cast<size_t>(floor(log2(MaxDimention)) + 1);

	TerrainComponent.LayerMaps[0]->UpdateRawData(FirstTextureData, MipCount);
	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	TerrainComponent.LayerMaps[1]->UpdateRawData(SecondTextureData, MipCount);
	FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
	FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));

	for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		delete[] AllLayers[i];
	}

	delete[] FirstTextureData;
	delete[] SecondTextureData;

	TerrainComponent.DeleteLayerInSlot(LayerIndex);
}

// This function is not working properly if mask needs to be resized.
void FETerrainSystem::LoadTerrainLayerMask(FEEntity* TerrainEntity, std::string FileName, const size_t LayerIndex)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::LoadTerrainLayerMask TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_LOADING", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainSystem::LoadTerrainLayerMask with out of bound \"layerIndex\"", "FE_LOG_LOADING", FE_LOG_WARNING);
		return;
	}

	if (TerrainComponent.Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrainSystem::LoadTerrainLayerMask on indicated layer slot layer is nullptr", "FE_LOG_LOADING", FE_LOG_WARNING);
		return;
	}

	FETexture* LoadedTexture = RESOURCE_MANAGER.LoadPNGTexture(FileName.c_str());
	size_t RawDataSize = 0;
	unsigned char* RawData = LoadedTexture->GetRawData(&RawDataSize);
	// It should be just ordinary png not gray scale.
	if (RawDataSize != LoadedTexture->GetWidth() * LoadedTexture->GetHeight() * 4)
	{
		LOG.Add(std::string("Can't use file: ") + FileName + " in function FETerrainSystem::LoadTerrainLayerMask as a mask.", "FE_LOG_LOADING", FE_LOG_ERROR);
		return;
	}

	// If new texture have different resolution.
	FETexture* FirstLayerMap = TerrainComponent.LayerMaps[0];
	if (LoadedTexture->GetWidth() != FirstLayerMap->GetWidth() || LoadedTexture->GetHeight() != FirstLayerMap->GetHeight())
	{
		bool bNeedToResizeMaskTexture = false;
		// Firstly we check if current masks has any data.
		std::vector<unsigned char*> LayersPerTextureData;
		LayersPerTextureData.resize(2);
		LayersPerTextureData[0] = TerrainComponent.LayerMaps[0]->GetRawData();
		LayersPerTextureData[1] = TerrainComponent.LayerMaps[1]->GetRawData();

		// We fill first layer by default so we should check TextureIterator differently
		const unsigned char FirstValue = LayersPerTextureData[0][0];
		for (size_t i = 0; i < static_cast<size_t>(TerrainComponent.LayerMaps[0]->GetWidth() * TerrainComponent.LayerMaps[0]->GetHeight()); i += 4)
		{
			if (LayersPerTextureData[0][i] != FirstValue || LayersPerTextureData[0][i + 1] != 0 ||
				LayersPerTextureData[0][i + 2] != 0 || LayersPerTextureData[0][i + 3] != 0 ||
				LayersPerTextureData[1][i] != 0 || LayersPerTextureData[1][i + 1] != 0 ||
				LayersPerTextureData[1][i + 2] != 0 || LayersPerTextureData[1][i + 3] != 0)
			{
				bNeedToResizeMaskTexture = true;
				break;
			}
		}

		if (bNeedToResizeMaskTexture)
		{
			LOG.Add("FETerrainSystem::LoadTerrainLayerMask resizing loaded mask to match currently used one.", "FE_LOG_LOADING", FE_LOG_WARNING);
			unsigned char* NewRawData = RESOURCE_MANAGER.ResizeTextureRawData(RawData, LoadedTexture->GetWidth(), LoadedTexture->GetHeight(), FirstLayerMap->GetWidth(), FirstLayerMap->GetHeight(), GL_RGBA, 1);
			if (NewRawData == nullptr)
			{
				LOG.Add("FETerrainSystem::LoadTerrainLayerMask resizing loaded mask failed.", "FE_LOG_LOADING", FE_LOG_ERROR);
				return;
			}

			delete[] RawData;
			RawData = NewRawData;
		}
		else
		{
			LOG.Add("FETerrainSystem::LoadTerrainLayerMask resizing TerrainLayerMap to match currently loaded one.", "FE_LOG_LOADING", FE_LOG_WARNING);

			// Fix problem, both LayerMaps would be cleared.
			std::vector<unsigned char> RawData;
			const size_t DataLenght = LoadedTexture->GetWidth() * LoadedTexture->GetHeight() * 4;
			RawData.resize(DataLenght);
			for (size_t i = 0; i < DataLenght; i++)
			{
				RawData[i] = 0;
			}

			FETexture* NewTexture = RESOURCE_MANAGER.RawDataToFETexture(RawData.data(), LoadedTexture->GetWidth(), LoadedTexture->GetHeight(), GL_RGBA, GL_RGBA);

			RESOURCE_MANAGER.DeleteFETexture(TerrainComponent.LayerMaps[0]);
			TerrainComponent.LayerMaps[0] = NewTexture;
			FirstLayerMap = TerrainComponent.LayerMaps[0];

			NewTexture = RESOURCE_MANAGER.RawDataToFETexture(RawData.data(), LoadedTexture->GetWidth(), LoadedTexture->GetHeight(), GL_RGBA, GL_RGBA);

			RESOURCE_MANAGER.DeleteFETexture(TerrainComponent.LayerMaps[1]);
			TerrainComponent.LayerMaps[1] = NewTexture;
		}
	}

	unsigned char* FilledChannel = new unsigned char[FirstLayerMap->GetWidth() * FirstLayerMap->GetHeight()];
	int Index = 0;
	for (size_t i = 0; i < static_cast<size_t>(FirstLayerMap->GetWidth() * FirstLayerMap->GetHeight() * 4); i += 4)
	{
		FilledChannel[Index++] = RawData[i];
	}

	FillTerrainLayerMaskWithRawData(TerrainEntity, FilledChannel, LayerIndex);
}

void FETerrainSystem::SaveTerrainLayerMask(FEEntity* TerrainEntity, std::string FileName, const size_t LayerIndex)
{
	if (TerrainEntity == nullptr || !TerrainEntity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::SaveTerrainLayerMask TerrainEntity is nullptr or does not have FETerrainComponent", "FE_LOG_LOADING", FE_LOG_WARNING);
		return;
	}
	FETerrainComponent& TerrainComponent = TerrainEntity->GetComponent<FETerrainComponent>();

	if (LayerIndex < 0 || LayerIndex >= FE_TERRAIN_MAX_LAYERS)
	{
		LOG.Add("FETerrainSystem::SaveTerrainLayerMask with out of bound \"LayerIndex\"", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	if (TerrainComponent.Layers[LayerIndex] == nullptr)
	{
		LOG.Add("FETerrainSystem::SaveTerrainLayerMask \"TerrainComponent.Layers[LayerIndex]\" is nullptr", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return;
	}

	// Reading data from current layer map texture.
	size_t ResultingTextureDataLenght = 0;
	FETexture* CorrectLayer = TerrainComponent.LayerMaps[LayerIndex / FE_TERRAIN_LAYER_PER_TEXTURE];
	const unsigned char* RawData = CorrectLayer->GetRawData(&ResultingTextureDataLenght);
	unsigned char* ResultingData = new unsigned char[ResultingTextureDataLenght];

	for (size_t i = 0; i < ResultingTextureDataLenght; i += 4)
	{
		const size_t index = i + LayerIndex % FE_TERRAIN_LAYER_PER_TEXTURE;
		ResultingData[i] = RawData[index];
		ResultingData[i + 1] = RawData[index];
		ResultingData[i + 2] = RawData[index];
		ResultingData[i + 3] = 255;
	}

	RESOURCE_MANAGER.ExportRawDataToPNG(FileName.c_str(), ResultingData, CorrectLayer->GetWidth(), CorrectLayer->GetHeight(), GL_RGBA);
}

Json::Value FETerrainSystem::TerrainComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	if (Entity == nullptr || !Entity->HasComponent<FETerrainComponent>())
	{
		LOG.Add("FETerrainSystem::TerrainComponentToJson Entity is nullptr or does not have FETerrainComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return Root;
	}
	FETerrainComponent& TerrainComponent = Entity->GetComponent<FETerrainComponent>();

	Root["Height map"]["ID"] = TerrainComponent.HeightMap->GetObjectID();
	Root["Height map"]["Name"] = TerrainComponent.HeightMap->GetName();
	Root["Height map"]["FileName"] = TerrainComponent.HeightMap->GetObjectID() + ".texture";

	Root["Hight scale"] = TerrainComponent.GetHightScale();
	Root["Displacement scale"] = TerrainComponent.GetDisplacementScale();
	Root["Tile multiplicator"]["X"] = TerrainComponent.GetTileMult().x;
	Root["Tile multiplicator"]["Y"] = TerrainComponent.GetTileMult().y;
	Root["LODlevel"] = TerrainComponent.GetLODLevel();
	Root["Chunks per side"] = TerrainComponent.GetChunkPerSide();

	// Saving terrains layers.
	for (int i = 0; i < TerrainComponent.LayerMaps.size(); i++)
	{
		if (TerrainComponent.LayerMaps[i] != nullptr)
		{
			Root["LayerMaps"][i]["ID"] = TerrainComponent.LayerMaps[i]->GetObjectID();
			Root["LayerMaps"][i]["Name"] = TerrainComponent.LayerMaps[i]->GetName();
			Root["LayerMaps"][i]["FileName"] = TerrainComponent.LayerMaps[i]->GetObjectID() + ".texture";
		}
	}

	for (int i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		FETerrainLayer* CurrentLayer = TerrainComponent.GetLayerInSlot(i);
		if (CurrentLayer == nullptr)
		{
			Root["Layers"][i]["isAcive"] = false;
			break;
		}

		Root["Layers"][i]["isAcive"] = true;
		Root["Layers"][i]["Name"] = CurrentLayer->GetMaterial()->GetName();
		Root["Layers"][i]["Material ID"] = CurrentLayer->GetMaterial()->GetObjectID();
	}

	return Root;
}

void FETerrainSystem::TerrainComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	if (Entity == nullptr)
	{
		LOG.Add("FETerrainSystem::TerrainComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	Entity->AddComponent<FETerrainComponent>();
	FETerrainComponent& TerrainComponent = Entity->GetComponent<FETerrainComponent>();

	FETexture* HightMapTexture = RESOURCE_MANAGER.GetTexture(Root["Height map"]["ID"].asString());
	TERRAIN_SYSTEM.SetHeightMap(HightMapTexture, Entity);

	TerrainComponent.SetHightScale(Root["Hight scale"].asFloat());
	TerrainComponent.SetDisplacementScale(Root["Displacement scale"].asFloat());
	TerrainComponent.SetTileMult(glm::vec2(Root["Tile multiplicator"]["X"].asFloat(), Root["Tile multiplicator"]["Y"].asFloat()));
	TerrainComponent.SetLODLevel(Root["LODlevel"].asFloat());
	TerrainComponent.SetChunkPerSide(Root["Chunks per side"].asFloat());

	for (int i = 0; i < FE_TERRAIN_MAX_LAYERS / FE_TERRAIN_LAYER_PER_TEXTURE; i++)
	{
		if (Root.isMember("LayerMaps"))
		{
			FETexture* LayerTexture = RESOURCE_MANAGER.GetTexture(Root["LayerMaps"][i]["ID"].asString());
			TerrainComponent.LayerMaps[i] = LayerTexture;
		}
	}

	for (int i = 0; i < FE_TERRAIN_MAX_LAYERS; i++)
	{
		if (Root["Layers"][i]["isAcive"].asBool())
		{
			TERRAIN_SYSTEM.ActivateVacantLayerSlot(Entity, RESOURCE_MANAGER.GetMaterial(Root["Layers"][i]["Material ID"].asCString()));
			TerrainComponent.GetLayerInSlot(i)->SetName(Root["Layers"][i]["Name"].asCString());
		}
	}
}