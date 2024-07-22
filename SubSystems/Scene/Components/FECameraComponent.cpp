#include "FECameraComponent.h"
#include "../ResourceManager/FEResourceManager.h"
using namespace FocalEngine;

FECameraComponent::FECameraComponent()
{
	Frustum.resize(6);
	for (size_t i = 0; i < Frustum.size(); i++)
		Frustum[i].resize(4);
}

FECameraComponent::FECameraComponent(const FECameraComponent& Other)
{
	
}

void FECameraComponent::operator=(const FECameraComponent& Other)
{
	
}

bool FECameraComponent::IsMainCamera() const
{
	return bIsMainCamera;
}

glm::mat4 FECameraComponent::GetViewMatrix() const
{
	return ViewMatrix;
}

glm::mat4 FECameraComponent::GetProjectionMatrix() const
{
	return ProjectionMatrix;
}

glm::vec3 FECameraComponent::GetUp() const
{
	return glm::normalize(glm::vec3(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * ViewMatrix));
}

glm::vec3 FECameraComponent::GetForward() const
{
	return glm::normalize(glm::vec3(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) * ViewMatrix));
}

glm::vec3 FECameraComponent::GetRight() const
{
	return glm::normalize(glm::vec3(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * ViewMatrix));
}

bool FECameraComponent::IsActive() const
{
	return bIsActive;
}

void FECameraComponent::SetActive(const bool Active)
{
	bIsActive = Active;
}

bool FECameraComponent::IsInputActive() const
{
	return bIsInputActive;
}

//void FECameraComponent::SetIsInputActive(const bool Active)
//{
//	bIsInputActive = Active;
//}

float FECameraComponent::GetFOV() const
{
	return FOV;
}

void FECameraComponent::SetFOV(const float FOV)
{
	this->FOV = FOV;
}

float FECameraComponent::GetNearPlane() const
{
	return NearPlane;
}

void FECameraComponent::SetNearPlane(const float NearPlane)
{
	this->NearPlane = NearPlane;
}

float FECameraComponent::GetFarPlane() const
{
	return FarPlane;
}

void FECameraComponent::SetFarPlane(const float FarPlane)
{
	this->FarPlane = FarPlane;
}

float FECameraComponent::GetAspectRatio() const
{
	return AspectRatio;
}

void FECameraComponent::SetAspectRatio(const float AspectRatio)
{
	this->AspectRatio = AspectRatio;
}

float FECameraComponent::GetGamma() const
{
	return Gamma;
}

void FECameraComponent::SetGamma(const float Gamma)
{
	this->Gamma = Gamma;
}

float FECameraComponent::GetExposure() const
{
	return Exposure;
}

void FECameraComponent::SetExposure(const float Exposure)
{
	this->Exposure = Exposure;
}

// FIX ME! Should not be here.
float FECameraComponent::GetMovementSpeed() const
{
	return MovementSpeed;
}

void FECameraComponent::SetMovementSpeed(const float MovementSpeed)
{
	this->MovementSpeed = MovementSpeed;
}

void FECameraComponent::UpdateFrustumPlanes()
{
	float Clip[16];

	glm::mat4 Cliping = GetProjectionMatrix() * GetViewMatrix();
	for (int i = 0; i < 4; i++)
	{
		Clip[i * 4] = Cliping[i][0];
		Clip[i * 4 + 1] = Cliping[i][1];
		Clip[i * 4 + 2] = Cliping[i][2];
		Clip[i * 4 + 3] = Cliping[i][3];
	}

	/* Extract the numbers for the RIGHT plane */
	Frustum[0][0] = Clip[3] - Clip[0];
	Frustum[0][1] = Clip[7] - Clip[4];
	Frustum[0][2] = Clip[11] - Clip[8];
	Frustum[0][3] = Clip[15] - Clip[12];

	/* Normalize the result */
	float T = sqrt(Frustum[0][0] * Frustum[0][0] + Frustum[0][1] * Frustum[0][1] + Frustum[0][2] * Frustum[0][2]);
	Frustum[0][0] /= T;
	Frustum[0][1] /= T;
	Frustum[0][2] /= T;
	Frustum[0][3] /= T;

	/* Extract the numbers for the LEFT plane */
	Frustum[1][0] = Clip[3] + Clip[0];
	Frustum[1][1] = Clip[7] + Clip[4];
	Frustum[1][2] = Clip[11] + Clip[8];
	Frustum[1][3] = Clip[15] + Clip[12];

	/* Normalize the result */
	T = sqrt(Frustum[1][0] * Frustum[1][0] + Frustum[1][1] * Frustum[1][1] + Frustum[1][2] * Frustum[1][2]);
	Frustum[1][0] /= T;
	Frustum[1][1] /= T;
	Frustum[1][2] /= T;
	Frustum[1][3] /= T;

	/* Extract the BOTTOM plane */
	Frustum[2][0] = Clip[3] + Clip[1];
	Frustum[2][1] = Clip[7] + Clip[5];
	Frustum[2][2] = Clip[11] + Clip[9];
	Frustum[2][3] = Clip[15] + Clip[13];

	/* Normalize the result */
	T = sqrt(Frustum[2][0] * Frustum[2][0] + Frustum[2][1] * Frustum[2][1] + Frustum[2][2] * Frustum[2][2]);
	Frustum[2][0] /= T;
	Frustum[2][1] /= T;
	Frustum[2][2] /= T;
	Frustum[2][3] /= T;

	/* Extract the TOP plane */
	Frustum[3][0] = Clip[3] - Clip[1];
	Frustum[3][1] = Clip[7] - Clip[5];
	Frustum[3][2] = Clip[11] - Clip[9];
	Frustum[3][3] = Clip[15] - Clip[13];

	/* Normalize the result */
	T = sqrt(Frustum[3][0] * Frustum[3][0] + Frustum[3][1] * Frustum[3][1] + Frustum[3][2] * Frustum[3][2]);
	Frustum[3][0] /= T;
	Frustum[3][1] /= T;
	Frustum[3][2] /= T;
	Frustum[3][3] /= T;

	/* Extract the FAR plane */
	Frustum[4][0] = Clip[3] - Clip[2];
	Frustum[4][1] = Clip[7] - Clip[6];
	Frustum[4][2] = Clip[11] - Clip[10];
	Frustum[4][3] = Clip[15] - Clip[14];

	/* Normalize the result */
	T = sqrt(Frustum[4][0] * Frustum[4][0] + Frustum[4][1] * Frustum[4][1] + Frustum[4][2] * Frustum[4][2]);
	Frustum[4][0] /= T;
	Frustum[4][1] /= T;
	Frustum[4][2] /= T;
	Frustum[4][3] /= T;

	/* Extract the NEAR plane */
	Frustum[5][0] = Clip[3] + Clip[2];
	Frustum[5][1] = Clip[7] + Clip[6];
	Frustum[5][2] = Clip[11] + Clip[10];
	Frustum[5][3] = Clip[15] + Clip[14];

	/* Normalize the result */
	T = sqrt(Frustum[5][0] * Frustum[5][0] + Frustum[5][1] * Frustum[5][1] + Frustum[5][2] * Frustum[5][2]);
	Frustum[5][0] /= T;
	Frustum[5][1] /= T;
	Frustum[5][2] /= T;
	Frustum[5][3] /= T;
}

std::vector<std::vector<float>> FECameraComponent::GetFrustumPlanes()
{
	return Frustum;
}

int FECameraComponent::GetRenderTargetWidth() const
{
	return RenderTargetWidth;
}

int FECameraComponent::GetRenderTargetHeight() const
{
	return RenderTargetHeight;
}

void FECameraComponent::SetRenderTargetSize(const int Width, const int Height)
{
	if (bUseDefaultRenderTargetSize)
		return;

	SetRenderTargetSizeInternal(Width, Height);
}

bool FECameraComponent::IsUsingDefaultRenderTargetSize() const
{
	return bUseDefaultRenderTargetSize;
}

void FECameraComponent::SetIsUsingDefaultRenderTargetSize(const bool NewValue)
{
	bUseDefaultRenderTargetSize = NewValue;
}

void FECameraComponent::SetRenderTargetSizeInternal(const int Width, const int Height)
{
	if (Width < 1 || Height < 1)
		return;

	RenderTargetWidth = Width;
	RenderTargetHeight = Height;

	AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);

	//if (CameraFramebuffer != nullptr)
	//	delete CameraFramebuffer;

	//CameraFramebuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, Width, Height);
}
//
//FEFramebuffer* FECameraComponent::GetCameraFramebuffer() const
//{
//	return CameraFramebuffer;
//}