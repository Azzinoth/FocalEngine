#include "FECameraSystem.h"
#include "../FEngine.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetCameraSystem()
{
	return FECameraSystem::GetInstancePointer();
}
#endif

FECameraSystem::FECameraSystem()
{
	RegisterOnComponentCallbacks();
	ENGINE.AddOnViewportResizeCallback(OnViewportResize);

	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FECameraComponent>(CameraComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FECameraComponent>(CameraComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FECameraComponent>(DuplicateCameraComponent);
}

void FECameraSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FECameraComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FECameraComponent>(OnMyComponentDestroy);
}

void FECameraSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
		return;

	FECameraComponent& CameraComponent = Entity->GetComponent<FECameraComponent>();
}

void FECameraSystem::DuplicateCameraComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr || !SourceEntity->HasComponent<FECameraComponent>())
		return;

	FECameraComponent& OriginalCameraComponent = SourceEntity->GetComponent<FECameraComponent>();
	TargetEntity->AddComponent<FECameraComponent>();

	FECameraComponent& NewCameraComponent = TargetEntity->GetComponent<FECameraComponent>();
	NewCameraComponent = OriginalCameraComponent;
	// By default, duplicated camera component should not be a main camera.
	NewCameraComponent.bIsMainCamera = false;
	// But if component is duplicated as part of scene duplication, it could be a main camera.
	// If no other camera is set as main camera in parent scene, it will be set as main camera.
	if (CAMERA_SYSTEM.GetMainCameraEntity(TargetEntity->GetParentScene()) == nullptr)
		NewCameraComponent.bIsMainCamera = true;
	
	// TO DO: Maybe rethink what to do with viewport.
	// Currently, we will set it to nullptr.
	NewCameraComponent.Viewport = nullptr;
}

void FECameraSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
		return;

	FECameraComponent& CameraComponent = Entity->GetComponent<FECameraComponent>();

	auto DataIterator = RENDERER.CameraRenderingDataMap.begin();
	while (DataIterator != RENDERER.CameraRenderingDataMap.end())
	{
		if (DataIterator->second->CameraEntity == Entity)
		{
			delete DataIterator->second;
			RENDERER.CameraRenderingDataMap.erase(DataIterator);
			break;
		}
		
		DataIterator++;
	}

	auto ViewPortIterator = CAMERA_SYSTEM.ViewPortToCameraEntities.begin();
	while (ViewPortIterator != CAMERA_SYSTEM.ViewPortToCameraEntities.end())
	{
		for (size_t i = 0; i < ViewPortIterator->second.size(); i++)
		{
			if (ViewPortIterator->second[i] == Entity)
			{
				ViewPortIterator->second.erase(ViewPortIterator->second.begin() + i);
				break;
			}
		}

		ViewPortIterator++;
	}
}

FECameraSystem::~FECameraSystem() {};

void FECameraSystem::SetMainCamera(FEEntity* CameraEntity)
{
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
	{
		LOG.Add("FECameraSystem::SetMainCamera CameraEntity is nullptr or does not have a camera component.", "FE_LOG_ECS", FE_LOG_ERROR);
		return;
	}

	if (CameraEntity->GetParentScene() == nullptr)
	{
		LOG.Add("FECameraSystem::SetMainCamera CameraEntity does not have a parent scene.", "FE_LOG_ECS", FE_LOG_ERROR);
		return;
	}

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

	std::vector<std::string> EntitiesWithCameraComponent = CameraEntity->GetParentScene()->GetEntityIDListWithComponent<FECameraComponent>();
	// Loop through all camera components and set them to not be the main camera.
	for (const std::string& EntityID : EntitiesWithCameraComponent)
	{
		FEEntity* Entity = CameraEntity->GetParentScene()->GetEntity(EntityID);
		if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
			continue;

		FECameraComponent& OtherCameraComponent = Entity->GetComponent<FECameraComponent>();
		OtherCameraComponent.bIsMainCamera = false;
	}

	CameraComponent.bIsMainCamera = true;
}

FEEntity* FECameraSystem::GetMainCameraEntity(FEScene* Scene) const
{
	if (Scene == nullptr)
	{
		LOG.Add("FECameraSystem::GetMainCameraEntity Scene is nullptr.", "FE_LOG_ECS", FE_LOG_ERROR);
		return nullptr;
	}

	std::vector<std::string> EntitiesWithCameraComponent = Scene->GetEntityIDListWithComponent<FECameraComponent>();
	for (const std::string& EntityID : EntitiesWithCameraComponent)
	{
		FEEntity* Entity = Scene->GetEntity(EntityID);
		if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
			continue;

		FECameraComponent& CameraComponent = Entity->GetComponent<FECameraComponent>();
		if (CameraComponent.bIsMainCamera)
			return Entity;
	}

	return nullptr;
}

void FECameraSystem::OnViewportResize(std::string ViewportID)
{
	if (ViewportID.empty())
	{
		LOG.Add("FECameraSystem::OnViewportResize ViewportID is empty.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return;
	}

	// Check if we are managing that viewport.
	if (CAMERA_SYSTEM.ViewPortToCameraEntities.find(ViewportID) == CAMERA_SYSTEM.ViewPortToCameraEntities.end())
		return;

	std::vector<FEEntity*> CameraEntities = CAMERA_SYSTEM.ViewPortToCameraEntities[ViewportID];
	for (FEEntity* CameraEntity : CameraEntities)
	{
		if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
			continue;

		FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
		CameraComponent.SetRenderTargetSizeInternal(CameraComponent.Viewport->Width, CameraComponent.Viewport->Height);
		RENDERER.OnResizeCameraRenderingDataUpdate(CameraEntity);
	}
}

void FECameraSystem::Update(const double DeltaTime)
{
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active | FESceneFlag::Renderable);
	for (FEScene* Scene : ActiveScenes)
	{
		std::vector<std::string> EntitiesWithCameraComponent = Scene->GetEntityIDListWithComponent<FECameraComponent>();
		for (const std::string& EntityID : EntitiesWithCameraComponent)
		{
			FEEntity* Entity = Scene->GetEntity(EntityID);
			if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
				continue;

			IndividualUpdate(Entity, DeltaTime);
		}
	}
}

void FECameraSystem::IndividualUpdate(FEEntity* CameraEntity, const double DeltaTime)
{
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
	{
		LOG.Add("FECameraSystem::IndividuallyUpdate CameraEntity is nullptr or does not have a camera component.", "FE_LOG_ECS", FE_LOG_ERROR);
		return;
	}

	FETransformComponent& TransformComponent = CameraEntity->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();

	if (CameraComponent.Viewport != nullptr)
	{
		// Check for stale data.
		if (CameraComponent.RenderTargetWidth != CameraComponent.Viewport->GetWidth() || CameraComponent.RenderTargetHeight != CameraComponent.Viewport->GetHeight())
		{
			if (CameraComponent.Viewport->GetWidth() != 0 && CameraComponent.Viewport->GetHeight() != 0)
			{
				CameraComponent.SetRenderTargetSizeInternal(CameraComponent.Viewport->Width, CameraComponent.Viewport->Height);
				RENDERER.OnResizeCameraRenderingDataUpdate(CameraEntity);
			}
		}
	}

	// If no script is attached, we will use default camera view matrix update.
	if (!CameraEntity->HasComponent<FENativeScriptComponent>())
		CameraComponent.ViewMatrix = glm::inverse(TransformComponent.GetWorldMatrix());
	
	CameraComponent.ProjectionMatrix = glm::perspective(glm::radians(CameraComponent.FOV), CameraComponent.AspectRatio, CameraComponent.NearPlane, CameraComponent.FarPlane);
}

bool FECameraSystem::SetCameraViewport(FEEntity* CameraEntity, std::string ViewportID)
{
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
	{
		LOG.Add("FECameraSystem::SetCameraViewport CameraEntity is nullptr or does not have a camera component.", "FE_LOG_ECS", FE_LOG_ERROR);
		return false;
	}

	if (ViewportID.empty())
	{
		LOG.Add("FECameraSystem::SetCameraViewport ViewportID is empty.", "FE_LOG_ECS", FE_LOG_ERROR);
		return false;
	}

	FEViewport* Viewport = ENGINE.GetViewport(ViewportID);

	if (Viewport == nullptr)
	{
		LOG.Add("FECameraSystem::SetCameraViewport Viewport was not found with such ViewportID.", "FE_LOG_ECS", FE_LOG_ERROR);
		return false;
	}

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	CameraComponent.Viewport = Viewport;

	ViewPortToCameraEntities[Viewport->ID].push_back(CameraEntity);

	IndividualUpdate(CameraEntity, 0.0);
	return true;
}

FEViewport* FECameraSystem::GetMainCameraViewport(FEScene* Scene) const
{
	if (Scene == nullptr)
	{
		LOG.Add("FECameraSystem::GetMainCameraViewport Scene is nullptr.", "FE_LOG_ECS", FE_LOG_ERROR);
		return nullptr;
	}

	std::vector<std::string> EntitiesWithCameraComponent = Scene->GetEntityIDListWithComponent<FECameraComponent>();
	for (const std::string& EntityID : EntitiesWithCameraComponent)
	{
		FEEntity* Entity = Scene->GetEntity(EntityID);
		if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
			continue;

		FECameraComponent& CameraComponent = Entity->GetComponent<FECameraComponent>();
		if (CameraComponent.bIsMainCamera)
			return CameraComponent.Viewport;
	}

	return nullptr;
}


Json::Value FECameraSystem::CameraComponentToJson(FEEntity* Entity)
{
	Json::Value Root;
	if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
	{
		LOG.Add("FECameraSystem::CameraComponentToJson Entity is nullptr or does not have FECameraComponent", "FE_LOG_ECS", FE_LOG_WARNING);
		return Root;
	}
	FECameraComponent& CameraComponent = Entity->GetComponent<FECameraComponent>();

	Root["bIsMainCamera"] = CameraComponent.bIsMainCamera;

	Root["ClearColor"]["R"] = CameraComponent.ClearColor.x;
	Root["ClearColor"]["G"] = CameraComponent.ClearColor.y;
	Root["ClearColor"]["B"] = CameraComponent.ClearColor.z;
	Root["ClearColor"]["A"] = CameraComponent.ClearColor.w;

	Root["FOV"] = CameraComponent.FOV;
	Root["NearPlane"] = CameraComponent.NearPlane;
	Root["FarPlane"] = CameraComponent.FarPlane;
	Root["AspectRatio"] = CameraComponent.AspectRatio;

	// *********** Gamma Correction & Exposure ***********
	Root["Gamma Correction & Exposure"]["Gamma"] = CameraComponent.GetGamma();
	Root["Gamma Correction & Exposure"]["Exposure"] = CameraComponent.GetExposure();
	// *********** Anti-Aliasing(FXAA) ***********
	Root["Anti-Aliasing(FXAA)"]["FXAASpanMax"] = CameraComponent.GetFXAASpanMax();
	Root["Anti-Aliasing(FXAA)"]["FXAAReduceMin"] = CameraComponent.GetFXAAReduceMin();
	Root["Anti-Aliasing(FXAA)"]["FXAAReduceMul"] = CameraComponent.GetFXAAReduceMul();
	// *********** Bloom ***********
	Root["Bloom"]["Threshold Brightness"] = CameraComponent.GetBloomThreshold();
	Root["Bloom"]["Bloom Size"] = CameraComponent.GetBloomSize();
	// *********** Depth of Field ***********
	Root["Depth of Field"]["Near distance"] = CameraComponent.GetDOFNearDistance();
	Root["Depth of Field"]["Far distance"] = CameraComponent.GetDOFFarDistance();
	Root["Depth of Field"]["Strength"] = CameraComponent.GetDOFStrength();
	Root["Depth of Field"]["Distance dependent strength"] = CameraComponent.GetDOFDistanceDependentStrength();
	// *********** Distance fog ***********
	Root["Distance fog"]["isDistanceFog Enabled"] = CameraComponent.IsDistanceFogEnabled();
	Root["Distance fog"]["Density"] = CameraComponent.GetDistanceFogDensity();
	Root["Distance fog"]["Gradient"] = CameraComponent.GetDistanceFogGradient();
	// *********** Chromatic Aberration ***********
	Root["Chromatic Aberration"]["Shift strength"] = CameraComponent.GetChromaticAberrationIntensity();
	// *********** SSAO ***********
	Root["SSAO"]["isSSAO Active"] = CameraComponent.IsSSAOEnabled();
	Root["SSAO"]["Sample Count"] = CameraComponent.GetSSAOSampleCount();

	Root["SSAO"]["Small Details"] = CameraComponent.IsSSAOSmallDetailsEnabled();
	Root["SSAO"]["Blured"] = CameraComponent.IsSSAOResultBlured();

	Root["SSAO"]["Bias"] = CameraComponent.GetSSAOBias();
	Root["SSAO"]["Radius"] = CameraComponent.GetSSAORadius();
	Root["SSAO"]["Radius Small Details"] = CameraComponent.GetSSAORadiusSmallDetails();
	Root["SSAO"]["Small Details Weight"] = CameraComponent.GetSSAOSmallDetailsWeight();

	// Should that be saved ?
	//FEViewport* Viewport = nullptr;

	return Root;
}

void FECameraSystem::CameraComponentFromJson(FEEntity* Entity, Json::Value Root)
{
	if (Entity == nullptr)
	{
		LOG.Add("FECameraSystem::CameraComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
		return;
	}

	Entity->AddComponent<FECameraComponent>();
	FECameraComponent& CameraComponent = Entity->GetComponent<FECameraComponent>();

	CameraComponent.bIsMainCamera = Root["bIsMainCamera"].asBool();

	CameraComponent.ClearColor.x = Root["ClearColor"]["R"].asFloat();
	CameraComponent.ClearColor.y = Root["ClearColor"]["G"].asFloat();
	CameraComponent.ClearColor.z = Root["ClearColor"]["B"].asFloat();
	CameraComponent.ClearColor.w = Root["ClearColor"]["A"].asFloat();

	CameraComponent.FOV = Root["FOV"].asFloat();
	CameraComponent.NearPlane = Root["NearPlane"].asFloat();
	CameraComponent.FarPlane = Root["FarPlane"].asFloat();
	CameraComponent.AspectRatio = Root["AspectRatio"].asFloat();

	// *********** Gamma Correction & Exposure ***********
	CameraComponent.SetGamma(Root["Gamma Correction & Exposure"]["Gamma"].asFloat());
	CameraComponent.SetExposure(Root["Gamma Correction & Exposure"]["Exposure"].asFloat());
	// *********** Anti-Aliasing(FXAA) ***********
	CameraComponent.SetFXAASpanMax(Root["Anti-Aliasing(FXAA)"]["FXAASpanMax"].asFloat());
	CameraComponent.SetFXAAReduceMin(Root["Anti-Aliasing(FXAA)"]["FXAAReduceMin"].asFloat());
	CameraComponent.SetFXAAReduceMul(Root["Anti-Aliasing(FXAA)"]["FXAAReduceMul"].asFloat());
	// *********** Bloom ***********
	CameraComponent.SetBloomThreshold(Root["Bloom"]["Threshold Brightness"].asFloat());
	CameraComponent.SetBloomSize(Root["Bloom"]["Bloom Size"].asFloat());
	// *********** Depth of Field ***********
	CameraComponent.SetDOFNearDistance(Root["Depth of Field"]["Near distance"].asFloat());
	CameraComponent.SetDOFFarDistance(Root["Depth of Field"]["Far distance"].asFloat());
	CameraComponent.SetDOFStrength(Root["Depth of Field"]["Strength"].asFloat());
	CameraComponent.SetDOFDistanceDependentStrength(Root["Depth of Field"]["Distance dependent strength"].asFloat());
	// *********** Distance fog ***********
	CameraComponent.SetDistanceFogEnabled(Root["Distance fog"]["isDistanceFog Enabled"].asBool());
	CameraComponent.SetDistanceFogDensity(Root["Distance fog"]["Density"].asFloat());
	CameraComponent.SetDistanceFogGradient(Root["Distance fog"]["Gradient"].asFloat());
	// *********** Chromatic Aberration ***********
	CameraComponent.SetChromaticAberrationIntensity(Root["Chromatic Aberration"]["Shift strength"].asFloat());
	// *********** SSAO ***********
	CameraComponent.SetSSAOEnabled(Root["SSAO"]["isSSAO Active"].asBool());
	CameraComponent.SetSSAOSampleCount(Root["SSAO"]["Sample Count"].asInt());

	CameraComponent.SetSSAOSmallDetailsEnabled(Root["SSAO"]["Small Details"].asBool());
	CameraComponent.SetSSAOResultBlured(Root["SSAO"]["Blured"].asBool());

	CameraComponent.SetSSAOBias(Root["SSAO"]["Bias"].asFloat());
	CameraComponent.SetSSAORadius(Root["SSAO"]["Radius"].asFloat());

	CameraComponent.SetSSAORadiusSmallDetails(Root["SSAO"]["Radius Small Details"].asFloat());
	CameraComponent.SetSSAOSmallDetailsWeight(Root["SSAO"]["Small Details Weight"].asFloat());
}