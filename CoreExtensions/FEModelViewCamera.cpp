#include "FEModelViewCamera.h"
using namespace FocalEngine;

FEModelViewCamera::FEModelViewCamera(const std::string Name) : FEBasicCamera(Name)
{
	Type = 2;
}

FEModelViewCamera::~FEModelViewCamera()
{

}

void FEModelViewCamera::Move(float DeltaTime)
{
	UpdateViewMatrix();

	if (ClientOnUpdateImpl)
		ClientOnUpdateImpl(this);
}

void FEModelViewCamera::Reset()
{
	CurrentPolarAngle = 90.0;
	CurrentAzimutAngle = 90.0;
	DistanceToModel = 10.0;

	FEBasicCamera::Reset();
}

void FEModelViewCamera::MouseMoveInput(const double Xpos, const double Ypos)
{
	const int MouseX = static_cast<int>(Xpos);
	const int MouseY = static_cast<int>(Ypos);

	if (!bIsInputActive)
	{
		LastMouseX = MouseX;
		LastMouseY = MouseY;
		return;
	}

	if (LastMouseX != MouseX)
	{
		CurrentAzimutAngle += (MouseX - LastMouseX) * 0.1f;
	}

	if (LastMouseY != MouseY)
	{
		SetPolarAngle(CurrentPolarAngle - (MouseY - LastMouseY) * 0.1f);
	}

	LastMouseX = MouseX;
	LastMouseY = MouseY;
}

void FEModelViewCamera::KeyboardInput(int Key, int Scancode, int Action, int Mods)
{
	if (!bIsInputActive)
		return;
}

glm::dvec3 FEModelViewCamera::PolarToCartesian(double PolarAngle, double AzimutAngle, const double R)
{
	PolarAngle *= glm::pi<double>() / 180.0;
	AzimutAngle *= glm::pi<double>() / 180.0;

	const double X = R * sin(PolarAngle) * cos(AzimutAngle);
	const double Y = R * sin(PolarAngle) * sin(AzimutAngle);
	const double Z = R * cos(PolarAngle);

	return glm::dvec3(X, Z, Y);
}

void FEModelViewCamera::UpdateViewMatrix()
{
	ViewMatrix = glm::mat4(1.0f);

	Position = PolarToCartesian(CurrentPolarAngle, CurrentAzimutAngle, DistanceToModel);
	ViewMatrix = glm::lookAt(Position, glm::vec3(0.0f), glm::vec3(0, 1, 0));

	ViewMatrix = glm::translate(ViewMatrix, -TrackingObjectPosition);
	Position += TrackingObjectPosition;
}

double FEModelViewCamera::GetDistanceToModel()
{
	return DistanceToModel;
}

void FEModelViewCamera::SetDistanceToModel(double NewValue)
{
	if (NewValue < 0.0)
		NewValue = 0.1;

	DistanceToModel = NewValue;
	UpdateViewMatrix();
}

void FEModelViewCamera::MouseScrollInput(const double Xoffset, const double Yoffset)
{
	if (!bIsInputActive)
		return;

	SetDistanceToModel(DistanceToModel + Yoffset * 2.0);
}

double FEModelViewCamera::GetPolarAngle()
{
	return CurrentPolarAngle;
}

void FEModelViewCamera::SetPolarAngle(double NewValue)
{
	if (NewValue < 0.01)
		NewValue = 0.011;

	if (NewValue > 179.98)
		NewValue = 179.98;

	CurrentPolarAngle = NewValue;
}

double FEModelViewCamera::GetAzimutAngle()
{
	return CurrentAzimutAngle;
}

void FEModelViewCamera::SetAzimutAngle(const double NewValue)
{
	CurrentAzimutAngle = NewValue;
}

glm::vec3 FEModelViewCamera::GetTrackingObjectPosition()
{
	return TrackingObjectPosition;
}

void FEModelViewCamera::SetTrackingObjectPosition(const glm::vec3 NewValue)
{
	TrackingObjectPosition = NewValue;
}