#include "FEFreeCamera.h"

#include <utility>
using namespace FocalEngine;

FEFreeCamera::FEFreeCamera(std::string Name) : FEBasicCamera (std::move(Name))
{
	Type = 1;
}

FEFreeCamera::~FEFreeCamera() = default;

void FEFreeCamera::Move(const float DeltaTime)
{
	glm::vec4 Forward = { 0.0f, 0.0f, -(MovementSpeed * 2) * (DeltaTime / 1000), 0.0f };
	glm::vec4 Right = { (MovementSpeed * 2) * (DeltaTime / 1000), 0.0f, 0.0f, 0.0f };

	Right = Right * ViewMatrix;
	Forward = Forward * ViewMatrix;

	glm::normalize(Right);
	glm::normalize(Forward);

	if (bLeftKeyPressed)
	{
		Position.x -= Right.x;
		Position.y -= Right.y;
		Position.z -= Right.z;
	}

	if (bUpKeyPressed)
	{
		Position.x += Forward.x;
		Position.y += Forward.y;
		Position.z += Forward.z;
	}

	if (bRightKeyPressed)
	{
		Position.x += Right.x;
		Position.y += Right.y;
		Position.z += Right.z;
	}

	if (bDownKeyPressed)
	{
		Position.x -= Forward.x;
		Position.y -= Forward.y;
		Position.z -= Forward.z;
	}

	UpdateViewMatrix();

	if (ClientOnUpdateImpl)
		ClientOnUpdateImpl(this);
}

void FEFreeCamera::SetIsInputActive(const bool bIsActive)
{
	if (bIsActive)
	{
		SetCursorToCenter();
	}
	else
	{
		bLeftKeyPressed = false;
		bUpKeyPressed = false;
		bDownKeyPressed = false;
		bRightKeyPressed = false;
	}

	FEBasicCamera::SetIsInputActive(bIsActive);
}

void FEFreeCamera::Reset()
{
	CurrentMouseXAngle = 0;
	CurrentMouseYAngle = 0;

	FEBasicCamera::Reset();
}

void FEFreeCamera::SetCursorToCenter()
{
	if (APPLICATION.GetMainWindow()->IsInFocus())
	{
		LastMouseX = RenderTargetCenterX;
		LastMouseY = RenderTargetCenterY;

		SetCursorPos(LastMouseX, LastMouseY);

		LastMouseX = LastMouseX - RenderTargetShiftX;
		LastMouseY = LastMouseY - RenderTargetShiftY;
	}
}

void FEFreeCamera::MouseMoveInput(const double Xpos, const double Ypos)
{
	if (!bIsInputActive)
		return;

	const int MouseX = static_cast<int>(Xpos);
	const int MouseY = static_cast<int>(Ypos);

	if (LastMouseX == 0) LastMouseX = MouseX;
	if (LastMouseY == 0) LastMouseY = MouseY;

	if (LastMouseX < MouseX || abs(LastMouseX - MouseX) > CorrectionToSensitivity)
	{
		CurrentMouseXAngle += (MouseX - LastMouseX) * 0.15f;
		SetCursorToCenter();
	}

	if (LastMouseY < MouseY || abs(LastMouseY - MouseY) > CorrectionToSensitivity)
	{
		CurrentMouseYAngle += (MouseY - LastMouseY) * 0.15f;
		SetCursorToCenter();
	}

	SetYaw(CurrentMouseXAngle);
	if (CurrentMouseYAngle > 89.0f)
		CurrentMouseYAngle = 89.0f;
	if (CurrentMouseYAngle < -89.0f)
		CurrentMouseYAngle = -89.0f;
	SetPitch(CurrentMouseYAngle);
}

void FEFreeCamera::KeyboardInput(const int Key, int Scancode, const int Action, int Mods)
{
	if (!bIsInputActive)
		return;

	if (Key == GLFW_KEY_A && Action == GLFW_PRESS)
	{
		bLeftKeyPressed = true;
	}
	else if (Key == GLFW_KEY_A && Action == GLFW_RELEASE)
	{
		bLeftKeyPressed = false;
	}

	if (Key == GLFW_KEY_W && Action == GLFW_PRESS)
	{
		bUpKeyPressed = true;
	}
	else if (Key == GLFW_KEY_W && Action == GLFW_RELEASE)
	{
		bUpKeyPressed = false;
	}

	if (Key == GLFW_KEY_S && Action == GLFW_PRESS)
	{
		bDownKeyPressed = true;
	}
	else if (Key == GLFW_KEY_S && Action == GLFW_RELEASE)
	{
		bDownKeyPressed = false;
	}

	if (Key == GLFW_KEY_D && Action == GLFW_PRESS)
	{
		bRightKeyPressed = true;
	}
	else if (Key == GLFW_KEY_D && Action == GLFW_RELEASE)
	{
		bRightKeyPressed = false;
	}
}

void FEFreeCamera::SetYaw(const float NewYaw)
{
	FEBasicCamera::SetYaw(NewYaw);
	CurrentMouseXAngle = NewYaw;
}

void FEFreeCamera::SetPitch(const float NewPitch)
{
	FEBasicCamera::SetPitch(NewPitch);
	CurrentMouseYAngle = NewPitch;
}

void FEFreeCamera::SetRenderTargetCenterX(const int NewRenderTargetCenterX)
{
	RenderTargetCenterX = NewRenderTargetCenterX;
}

void FEFreeCamera::SetRenderTargetCenterY(const int NewRenderTargetCenterY)
{
	RenderTargetCenterY = NewRenderTargetCenterY;
}

void FEFreeCamera::SetRenderTargetShiftX(const int NewRenderTargetShiftX)
{
	RenderTargetShiftX = NewRenderTargetShiftX;
}

void FEFreeCamera::SetRenderTargetShiftY(const int NewRenderTargetShiftY)
{
	RenderTargetShiftY = NewRenderTargetShiftY;
}