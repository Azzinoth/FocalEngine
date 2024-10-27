#include "FirstScript.h"

void FreeCameraController::Awake()
{

}

void FreeCameraController::SetCursorToCenter()
{
	if (APPLICATION.GetMainWindow()->IsInFocus())
	{
		FECameraComponent& CameraComponent = ParentEntity->GetComponent<FECameraComponent>();
		const FEViewport* Viewport = CameraComponent.GetViewport();
		if (Viewport != nullptr)
		{
			int MainWindowXPosition, MainWindowYPosition;
			APPLICATION.GetMainWindow()->GetPosition(&MainWindowXPosition, &MainWindowYPosition);
			int CenterX = MainWindowXPosition + Viewport->GetX() + (Viewport->GetWidth() / 2);
			int CenterY = MainWindowYPosition + Viewport->GetY() + (Viewport->GetHeight() / 2);

			RenderTargetCenterX = CenterX;
			RenderTargetCenterY = CenterY;
		}

		LastMouseX = RenderTargetCenterX;
		LastMouseY = RenderTargetCenterY;

		INPUT.SetMousePosition(LastMouseX, LastMouseY);
		FEInputMouseState MouseState = INPUT.GetMouseState();

		LastMouseX = static_cast<int>(MouseState.X);
		LastMouseY = static_cast<int>(MouseState.Y);
	}
}

void FreeCameraController::UpdateViewMatrix()
{
	if (ParentEntity == nullptr || !ParentEntity->HasComponent<FECameraComponent>())
		return;

	FETransformComponent& TransformComponent = ParentEntity->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = ParentEntity->GetComponent<FECameraComponent>();

	CameraComponent.SetViewMatrix(glm::inverse(TransformComponent.GetWorldMatrix()));
}

void FreeCameraController::OnUpdate(double DeltaTime)
{
	if (ParentEntity == nullptr || !ParentEntity->HasComponent<FECameraComponent>())
		return;

	FETransformComponent& TransformComponent = ParentEntity->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = ParentEntity->GetComponent<FECameraComponent>();

	if (!bLastFrameWasActive && CameraComponent.IsActive())
		SetCursorToCenter();

	bLastFrameWasActive = CameraComponent.IsActive();

	FEInputMouseState MouseState = INPUT.GetMouseState();

	const int MouseX = static_cast<int>(MouseState.X);
	const int MouseY = static_cast<int>(MouseState.Y);

	if (CameraComponent.IsActive() && DeltaTime > 0.0)
	{
		glm::vec4 Forward = { 0.0f, 0.0f, -(MovementSpeed * 2) * (DeltaTime / 1000), 0.0f };
		glm::vec4 Right = { (MovementSpeed * 2) * (DeltaTime / 1000), 0.0f, 0.0f, 0.0f };

		Right = Right * CameraComponent.GetViewMatrix();
		Forward = Forward * CameraComponent.GetViewMatrix();

		glm::normalize(Right);
		glm::normalize(Forward);

		glm::vec3 CurrentPosition = TransformComponent.GetPosition();

		if (INPUT.GetKeyInfo(FEInputKey::FE_KEY_A).State != FE_RELEASED)
		{
			CurrentPosition.x -= Right.x;
			CurrentPosition.y -= Right.y;
			CurrentPosition.z -= Right.z;
		}

		if (INPUT.GetKeyInfo(FEInputKey::FE_KEY_W).State != FE_RELEASED)
		{
			CurrentPosition.x += Forward.x;
			CurrentPosition.y += Forward.y;
			CurrentPosition.z += Forward.z;
		}

		if (INPUT.GetKeyInfo(FEInputKey::FE_KEY_D).State != FE_RELEASED)
		{
			CurrentPosition.x += Right.x;
			CurrentPosition.y += Right.y;
			CurrentPosition.z += Right.z;
		}

		if (INPUT.GetKeyInfo(FEInputKey::FE_KEY_S).State != FE_RELEASED)
		{
			CurrentPosition.x -= Forward.x;
			CurrentPosition.y -= Forward.y;
			CurrentPosition.z -= Forward.z;
		}

		TransformComponent.SetPosition(CurrentPosition);

		// Rotation part.
		if (LastMouseX == 0) LastMouseX = MouseX;
		if (LastMouseY == 0) LastMouseY = MouseY;

		if (bFirstFrameActive)
		{
			bFirstFrameActive = false;
			CurrentMouseYAngle = TransformComponent.GetRotation().x;
			CurrentMouseXAngle = TransformComponent.GetRotation().y;

			return;
		}

		if (LastMouseX < MouseX || abs(LastMouseX - MouseX) > CorrectionToSensitivity)
		{
			CurrentMouseXAngle -= (MouseX - LastMouseX) * 0.15f * static_cast<float>((DeltaTime / 5.0));
			SetCursorToCenter();
		}

		if (LastMouseY < MouseY || abs(LastMouseY - MouseY) > CorrectionToSensitivity)
		{
			CurrentMouseYAngle -= (MouseY - LastMouseY) * 0.15f * static_cast<float>((DeltaTime / 5.0));
			SetCursorToCenter();
		}

		TransformComponent.SetRotation(glm::vec3(CurrentMouseYAngle, CurrentMouseXAngle, 0.0f));
	}

	UpdateViewMatrix();
}

void ModelViewCameraController::Awake()
{

}

glm::dvec3 ModelViewCameraController::PolarToCartesian(double PolarAngle, double AzimutAngle, const double R)
{
	PolarAngle *= glm::pi<double>() / 180.0;
	AzimutAngle *= glm::pi<double>() / 180.0;

	const double X = R * sin(PolarAngle) * cos(AzimutAngle);
	const double Y = R * sin(PolarAngle) * sin(AzimutAngle);
	const double Z = R * cos(PolarAngle);

	return glm::dvec3(X, Z, Y);
}

void ModelViewCameraController::UpdateViewMatrix()
{
	if (ParentEntity == nullptr || !ParentEntity->HasComponent<FECameraComponent>())
		return;

	FETransformComponent& TransformComponent = ParentEntity->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = ParentEntity->GetComponent<FECameraComponent>();

	glm::mat4 NewViewMatrix = glm::mat4(1.0f);

	glm::dvec3 NewPosition = PolarToCartesian(CurrentPolarAngle, CurrentAzimutAngle, DistanceToModel);
	NewViewMatrix = glm::lookAt(NewPosition, glm::dvec3(0.0f), glm::dvec3(0, 1, 0));

	NewViewMatrix = glm::translate(NewViewMatrix, -TargetPosition);
	NewPosition += TargetPosition;

	glm::dvec3 Position;
	glm::dquat Rotation;
	glm::dvec3 Scale;
	GEOMETRY.DecomposeMatrixToTranslationRotationScale(NewViewMatrix, Position, Rotation, Scale);

	// We need to udpate TransformComponent position and rotation.
	TransformComponent.SetPosition(NewPosition);
	TransformComponent.SetQuaternion(Rotation);
	TransformComponent.ForceSetWorldMatrix(TransformComponent.GetLocalMatrix());

	CameraComponent.SetViewMatrix(NewViewMatrix);
}

void ModelViewCameraController::OnUpdate(double DeltaTime)
{
	if (ParentEntity == nullptr || !ParentEntity->HasComponent<FECameraComponent>())
		return;

	FETransformComponent& TransformComponent = ParentEntity->GetComponent<FETransformComponent>();
	FECameraComponent& CameraComponent = ParentEntity->GetComponent<FECameraComponent>();

	FEInputMouseState MouseState = INPUT.GetMouseState();
	const int MouseX = static_cast<int>(MouseState.X);
	const int MouseY = static_cast<int>(MouseState.Y);

	if (CameraComponent.IsActive() && DeltaTime > 0.0)
	{
		float NewDistanceToModelValue = DistanceToModel + static_cast<float>(MouseState.ScrollYOffset) * 2.0f * MouseWheelSensitivity;
		if (NewDistanceToModelValue < 0.0f)
			NewDistanceToModelValue = 0.1f;

		DistanceToModel = NewDistanceToModelValue;

		if (LastMouseX != MouseX)
		{
			CurrentAzimutAngle += (MouseX - LastMouseX) * 0.1f;
		}

		if (LastMouseY != MouseY)
		{
			double NewValue = CurrentPolarAngle - (MouseY - LastMouseY) * 0.1f;
			if (NewValue < 0.01)
				NewValue = 0.011;

			if (NewValue > 179.98)
				NewValue = 179.98;

			CurrentPolarAngle = NewValue;
		}
	}

	LastMouseX = MouseX;
	LastMouseY = MouseY;

	UpdateViewMatrix();
}