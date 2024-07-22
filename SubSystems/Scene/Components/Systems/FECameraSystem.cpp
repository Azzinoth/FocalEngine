#include "FECameraSystem.h"
#include "../FEngine.h"
using namespace FocalEngine;

FECameraSystem* FECameraSystem::Instance = nullptr;
FECameraSystem::FECameraSystem()
{
	RegisterOnComponentCallbacks();
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
	if (CameraComponent.bUseDefaultRenderTargetSize)
		CameraComponent.SetRenderTargetSizeInternal(ENGINE.GetRenderTargetWidth(), ENGINE.GetRenderTargetHeight());
}

void FECameraSystem::DuplicateCameraComponent(FEEntity* EntityWithCameraComponent, FEEntity* NewEntity)
{
	/*if (EntityWithCameraComponent == nullptr || NewEntity == nullptr || !EntityWithCameraComponent->HasComponent<FECameraComponent>())
		return;

	FECameraComponent& CameraComponent = EntityWithCameraComponent->GetComponent<FECameraComponent>();*/
}

void FECameraSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	/*if (Entity == nullptr || !Entity->HasComponent<FECameraComponent>())
		return;

	FECameraComponent& CameraComponent = Entity->GetComponent<FECameraComponent>();*/
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
	
	CameraComponent.bIsInputActive = Active;
	if (Active)
		SetCursorToCenter(CameraComponent);
}

void FECameraSystem::SetCursorToCenter(FECameraComponent& Camera)
{
	if (APPLICATION.GetMainWindow()->IsInFocus())
	{
		Camera.LastMouseX = RenderTargetCenterX;
		Camera.LastMouseY = RenderTargetCenterY;

		ENGINE.SetMousePosition(Camera.LastMouseX, Camera.LastMouseY);
		//SetCursorPos(Camera.LastMouseX, Camera.LastMouseY);

		Camera.LastMouseX = static_cast<int>(ENGINE.MouseX);
		Camera.LastMouseY = static_cast<int>(ENGINE.MouseY);
		//Camera.LastMouseX = Camera.LastMouseX - RenderTargetShiftX;
		//Camera.LastMouseY = Camera.LastMouseY - RenderTargetShiftY;
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
	const int MouseX = static_cast<int>(ENGINE.MouseX);
	const int MouseY = static_cast<int>(ENGINE.MouseY);

	if (CameraComponent.bIsInputActive && DeltaTime > 0.0)
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
				CameraComponent.CurrentMouseXAngle -= (MouseX - CameraComponent.LastMouseX) * 0.15f;
				SetCursorToCenter(CameraComponent);
			}

			if (CameraComponent.LastMouseY < MouseY || abs(CameraComponent.LastMouseY - MouseY) > CameraComponent.CorrectionToSensitivity)
			{
				CameraComponent.CurrentMouseYAngle -= (MouseY - CameraComponent.LastMouseY) * 0.15f;
				SetCursorToCenter(CameraComponent);
			}

			if (CameraComponent.CurrentMouseYAngle > 89.0f)
				CameraComponent.CurrentMouseYAngle = -89.0f;
			if (CameraComponent.CurrentMouseYAngle < -89.0f)
				CameraComponent.CurrentMouseYAngle = 89.0f;

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

	if (CameraComponent.Type == 1 && !CameraComponent.bIsInputActive)
	{
		CameraComponent.LastMouseX = MouseX;
		CameraComponent.LastMouseY = MouseY;
		return;
	}

	if (CameraComponent.Type == 0)
	{
		//TransformComponent.Update();
		//TransformComponent.ForceSetWorldMatrix(TransformComponent.GetLocalMatrix());
		CameraComponent.ViewMatrix = glm::inverse(TransformComponent.GetWorldMatrix());
	}
	else if (CameraComponent.Type == 1)
	{
		// FIX ME! It should be controlled through the FETransform component with a script.
		CameraComponent.ViewMatrix = glm::mat4(1.0f);

		glm::vec3 NewPosition = PolarToCartesian(CameraComponent.CurrentPolarAngle, CameraComponent.CurrentAzimutAngle, CameraComponent.DistanceToModel);
		CameraComponent.ViewMatrix = glm::lookAt(NewPosition, glm::vec3(0.0f), glm::vec3(0, 1, 0));

		CameraComponent.ViewMatrix = glm::translate(CameraComponent.ViewMatrix, -CameraComponent.TrackingObjectPosition);
		NewPosition += CameraComponent.TrackingObjectPosition;
	}
	
	CameraComponent.ProjectionMatrix = glm::perspective(glm::radians(CameraComponent.FOV), CameraComponent.AspectRatio, CameraComponent.NearPlane, CameraComponent.FarPlane);
}

void FECameraSystem::RenderTargetResize(int Width, int Height)
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

			FECameraComponent& CameraComponent = Entity->GetComponent<FECameraComponent>();

			if (CameraComponent.bUseDefaultRenderTargetSize)
				CameraComponent.SetRenderTargetSizeInternal(Width, Height);
			
			IndividualUpdate(Entity, 0.0);
		}
	}
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