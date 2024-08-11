#include "FECameraSystem.h"
#include "../FEngine.h"
using namespace FocalEngine;

FECameraSystem* FECameraSystem::Instance = nullptr;
FECameraSystem::FECameraSystem()
{
	RegisterOnComponentCallbacks();
	ENGINE.AddOnViewportResizeCallback(OnViewportResize);

	COMPONENTS_TOOL.RegisterComponentToJsonFunction<FECameraComponent>(CameraComponentToJson);
	COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FECameraComponent>(CameraComponentFromJson);
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

	/*if (CameraComponent.Viewport != nullptr)
	{
		ENGINE.ViewportCheckForModificationIndividual(CameraComponent.Viewport);
		CameraComponent.SetRenderTargetSizeInternal(CameraComponent.Viewport->Width, CameraComponent.Viewport->Height);
	}*/
}

void FECameraSystem::DuplicateCameraComponent(FEEntity* EntityWithCameraComponent, FEEntity* NewEntity)
{
	if (EntityWithCameraComponent == nullptr || NewEntity == nullptr || !EntityWithCameraComponent->HasComponent<FECameraComponent>())
		return;

	FECameraComponent& OriginalCameraComponent = EntityWithCameraComponent->GetComponent<FECameraComponent>();
	NewEntity->AddComponent<FECameraComponent>();

	FECameraComponent& NewCameraComponent = NewEntity->GetComponent<FECameraComponent>();
	NewCameraComponent = OriginalCameraComponent;
	NewCameraComponent.bIsMainCamera = false;
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

	std::vector<std::string> EntitiesWithCameraComponent = CameraEntity->GetParentScene()->GetEntityIDListWith<FECameraComponent>();
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

	std::vector<std::string> EntitiesWithCameraComponent = Scene->GetEntityIDListWith<FECameraComponent>();
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
	std::vector<FEScene*> ActiveScenes = SCENE_MANAGER.GetActiveScenes();
	for (FEScene* Scene : ActiveScenes)
	{
		std::vector<std::string> EntitiesWithCameraComponent = Scene->GetEntityIDListWith<FECameraComponent>();
		for (const std::string& EntityID : EntitiesWithCameraComponent)
		{
			FEEntity* Entity = Scene->GetEntity(EntityID);
			if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
				continue;

			IndividualUpdate(Entity, DeltaTime);
		}
	}
}

void FECameraSystem::SetIsIndividualInputActive(FEEntity* CameraEntity, const bool Active)
{
	if (CameraEntity == nullptr || !CameraEntity->HasComponent<FECameraComponent>())
	{
		LOG.Add("FECameraSystem::SetIsIndividualInputActive CameraEntity is nullptr or does not have a camera component.", "FE_LOG_ECS", FE_LOG_ERROR);
		return;
	}

	FECameraComponent& CameraComponent = CameraEntity->GetComponent<FECameraComponent>();
	
	CameraComponent.bIsInputGrabingActive = Active;
	if (Active && CameraComponent.Type == 0)
		SetCursorToCenter(CameraComponent);
}

void FECameraSystem::SetCursorToCenter(FECameraComponent& Camera)
{
	if (APPLICATION.GetMainWindow()->IsInFocus())
	{
		Camera.LastMouseX = Camera.RenderTargetCenterX;
		Camera.LastMouseY = Camera.RenderTargetCenterY;

		ENGINE.SetMousePosition(Camera.LastMouseX, Camera.LastMouseY);

		Camera.LastMouseX = static_cast<int>(ENGINE.MouseX);
		Camera.LastMouseY = static_cast<int>(ENGINE.MouseY);
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

	// FIX ME! This is temporary
	if (CameraComponent.Viewport != nullptr)
	{
		int MainWindowXPosition, MainWindowYPosition;
		APPLICATION.GetMainWindow()->GetPosition(&MainWindowXPosition, &MainWindowYPosition);

		int CenterX = MainWindowXPosition + CameraComponent.Viewport->X + (CameraComponent.Viewport->Width / 2);
		int CenterY = MainWindowYPosition + CameraComponent.Viewport->Y + (CameraComponent.Viewport->Height / 2);

		int ShiftX = MainWindowXPosition + CameraComponent.Viewport->X;
		int ShiftY = MainWindowYPosition + CameraComponent.Viewport->Y;

		CameraComponent.RenderTargetCenterX = CenterX;
		CameraComponent.RenderTargetCenterY = CenterY;

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

	// FIX ME! This is temporary
	const int MouseX = static_cast<int>(ENGINE.MouseX);
	const int MouseY = static_cast<int>(ENGINE.MouseY);

	if (CameraComponent.bIsInputGrabingActive && DeltaTime > 0.0)
	{
		// FIX ME! This is temporary
		if (CameraComponent.Type == 0)
		{
			glm::vec4 Forward = { 0.0f, 0.0f, -(CameraComponent.MovementSpeed * 2) * (DeltaTime / 1000), 0.0f };
			glm::vec4 Right = { (CameraComponent.MovementSpeed * 2) * (DeltaTime / 1000), 0.0f, 0.0f, 0.0f };

			Right = Right * CameraComponent.ViewMatrix;
			Forward = Forward * CameraComponent.ViewMatrix;

			glm::normalize(Right);
			glm::normalize(Forward);

			glm::vec3 CurrentPosition = TransformComponent.GetPosition();

			if (ENGINE.bAKeyPressed)
			{
				CurrentPosition.x -= Right.x;
				CurrentPosition.y -= Right.y;
				CurrentPosition.z -= Right.z;
			}

			if (ENGINE.bWKeyPressed)
			{
				CurrentPosition.x += Forward.x;
				CurrentPosition.y += Forward.y;
				CurrentPosition.z += Forward.z;
			}

			if (ENGINE.bDKeyPressed)
			{
				CurrentPosition.x += Right.x;
				CurrentPosition.y += Right.y;
				CurrentPosition.z += Right.z;
			}

			if (ENGINE.bSKeyPressed)
			{
				CurrentPosition.x -= Forward.x;
				CurrentPosition.y -= Forward.y;
				CurrentPosition.z -= Forward.z;
			}

			TransformComponent.SetPosition(CurrentPosition);

			// Rotation part.
			if (CameraComponent.LastMouseX == 0) CameraComponent.LastMouseX = MouseX;
			if (CameraComponent.LastMouseY == 0) CameraComponent.LastMouseY = MouseY;

			if (CameraComponent.LastMouseX < MouseX || abs(CameraComponent.LastMouseX - MouseX) > CameraComponent.CorrectionToSensitivity)
			{
				CameraComponent.CurrentMouseXAngle -= (MouseX - CameraComponent.LastMouseX) * 0.15f * static_cast<float>((DeltaTime / 5.0));
				SetCursorToCenter(CameraComponent);
			}

			if (CameraComponent.LastMouseY < MouseY || abs(CameraComponent.LastMouseY - MouseY) > CameraComponent.CorrectionToSensitivity)
			{
				CameraComponent.CurrentMouseYAngle -= (MouseY - CameraComponent.LastMouseY) * 0.15f * static_cast<float>((DeltaTime / 5.0));
				SetCursorToCenter(CameraComponent);
			}

			TransformComponent.SetRotation(glm::vec3(CameraComponent.CurrentMouseYAngle, CameraComponent.CurrentMouseXAngle, 0.0f));
		}
		else if (CameraComponent.Type == 1)
		{
			double NewDistanceToModelValue = CameraComponent.DistanceToModel + ENGINE.MouseScrollYOffset * 2.0;
			if (NewDistanceToModelValue < 0.0)
				NewDistanceToModelValue = 0.1;

			CameraComponent.DistanceToModel = NewDistanceToModelValue;

			if (CameraComponent.LastMouseX != MouseX)
			{
				CameraComponent.CurrentAzimutAngle += (MouseX - CameraComponent.LastMouseX) * 0.1f;
			}

			if (CameraComponent.LastMouseY != MouseY)
			{
				double NewValue = CameraComponent.CurrentPolarAngle - (MouseY - CameraComponent.LastMouseY) * 0.1f;
				if (NewValue < 0.01)
					NewValue = 0.011;

				if (NewValue > 179.98)
					NewValue = 179.98;

				CameraComponent.CurrentPolarAngle = NewValue;
			}

			CameraComponent.LastMouseX = MouseX;
			CameraComponent.LastMouseY = MouseY;
		}
	}

	if (CameraComponent.Type == 1 && !CameraComponent.bIsInputGrabingActive)
	{
		CameraComponent.LastMouseX = MouseX;
		CameraComponent.LastMouseY = MouseY;
	}

	if (CameraComponent.Type == 0)
	{
		CameraComponent.ViewMatrix = glm::inverse(TransformComponent.GetWorldMatrix());
	}
	else if (CameraComponent.Type == 1)
	{
		CameraComponent.ViewMatrix = glm::mat4(1.0f);

		glm::dvec3 NewPosition = PolarToCartesian(CameraComponent.CurrentPolarAngle, CameraComponent.CurrentAzimutAngle, CameraComponent.DistanceToModel);
		CameraComponent.ViewMatrix = glm::lookAt(NewPosition, glm::dvec3(0.0f), glm::dvec3(0, 1, 0));

		CameraComponent.ViewMatrix = glm::translate(CameraComponent.ViewMatrix, -CameraComponent.TrackingObjectPosition);
		NewPosition += CameraComponent.TrackingObjectPosition;

		glm::dvec3 Position;
		glm::dquat Rotation;
		glm::dvec3 Scale;
		GEOMETRY.DecomposeMatrixToTranslationRotationScale(CameraComponent.ViewMatrix, Position, Rotation, Scale);

		// We need to udpate TransformComponent position and rotation.
		TransformComponent.SetPosition(NewPosition);
		TransformComponent.SetQuaternion(Rotation);
		TransformComponent.ForceSetWorldMatrix(TransformComponent.GetLocalMatrix());
	}
	
	CameraComponent.ProjectionMatrix = glm::perspective(glm::radians(CameraComponent.FOV), CameraComponent.AspectRatio, CameraComponent.NearPlane, CameraComponent.FarPlane);
}

glm::dvec3 FECameraSystem::PolarToCartesian(double PolarAngle, double AzimutAngle, const double R)
{
	PolarAngle *= glm::pi<double>() / 180.0;
	AzimutAngle *= glm::pi<double>() / 180.0;

	const double X = R * sin(PolarAngle) * cos(AzimutAngle);
	const double Y = R * sin(PolarAngle) * sin(AzimutAngle);
	const double Z = R * cos(PolarAngle);

	return glm::dvec3(X, Z, Y);
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

	std::vector<std::string> EntitiesWithCameraComponent = Scene->GetEntityIDListWith<FECameraComponent>();
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
	// It should not be here.
	Root["Type"] = CameraComponent.Type;
	Root["DistanceToModel"] = CameraComponent.DistanceToModel;
	Root["CurrentPolarAngle"] = CameraComponent.CurrentPolarAngle;
	Root["CurrentAzimutAngle"] = CameraComponent.CurrentAzimutAngle;
	Root["MovementSpeed"] = CameraComponent.MovementSpeed;
	
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
	// It should not be here.
	CameraComponent.Type = Root["Type"].asInt();
	CameraComponent.DistanceToModel = Root["DistanceToModel"].asDouble();
	CameraComponent.CurrentPolarAngle = Root["CurrentPolarAngle"].asDouble();
	CameraComponent.CurrentAzimutAngle = Root["CurrentAzimutAngle"].asDouble();
	CameraComponent.MovementSpeed = Root["MovementSpeed"].asFloat();

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