#include "FEBasicCamera.h"
using namespace FocalEngine;

FEBasicCamera::FEBasicCamera(const std::string Name) : FEObject(FE_CAMERA, Name)
{
	Yaw = 0.0f;
	UpdateAll();

	Frustum = new float*[6];
	for (size_t i = 0; i < 6; i++)
	{
		Frustum[i] = new float[4];
	}
}

FEBasicCamera::~FEBasicCamera()
{
	for (size_t i = 0; i < 6; i++)
	{
		delete[] Frustum[i];
	}
	delete[] Frustum;
}

float FEBasicCamera::GetYaw()
{
	return Yaw;
}

void FEBasicCamera::SetYaw(const float NewYaw)
{
	Yaw = NewYaw;
	UpdateViewMatrix();
}

float FEBasicCamera::GetPitch()
{
	return Pitch;
}

void FEBasicCamera::SetPitch(const float NewPitch)
{
	Pitch = NewPitch;
	UpdateViewMatrix();
}

float FEBasicCamera::GetRoll()
{
	return Roll;
}

void FEBasicCamera::SetRoll(const float NewRoll)
{
	Roll = NewRoll;
	UpdateViewMatrix();
}

glm::vec3 FEBasicCamera::GetPosition() const
{
	return Position;
}

void FEBasicCamera::SetPosition(const glm::vec3 NewPosition)
{
	Position = NewPosition;
	UpdateViewMatrix();
}

float FEBasicCamera::GetAspectRatio() const
{
	return AspectRatio;
}

void FEBasicCamera::SetAspectRatio(const float NewAspectRatio)
{
	AspectRatio = NewAspectRatio;
	UpdateProjectionMatrix();
}

void FEBasicCamera::UpdateViewMatrix()
{
	ViewMatrix = glm::mat4(1.0f);

	ViewMatrix = glm::rotate(ViewMatrix, GetPitch() * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
	ViewMatrix = glm::rotate(ViewMatrix, GetYaw() * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
	ViewMatrix = glm::rotate(ViewMatrix, GetRoll() * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));

	const glm::vec3 CameraPosition = GetPosition();
	const glm::vec3 NegativeCameraPosition = -CameraPosition;

	ViewMatrix = glm::translate(ViewMatrix, NegativeCameraPosition);
}

glm::mat4 FEBasicCamera::GetViewMatrix() const
{
	return ViewMatrix;
}

glm::mat4 FEBasicCamera::GetProjectionMatrix() const
{
	return ProjectionMatrix;
}

void FEBasicCamera::UpdateProjectionMatrix()
{
	ProjectionMatrix = glm::perspective(Fov, AspectRatio, NearPlane, FarPlane);
}

void FEBasicCamera::UpdateAll()
{
	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

float FEBasicCamera::GetFov() const
{
	return Fov;
}

void FEBasicCamera::SetFov(const float NewFov)
{
	Fov = NewFov;
	UpdateProjectionMatrix();
}

float FEBasicCamera::GetNearPlane() const
{
	return NearPlane;
}

void FEBasicCamera::SetNearPlane(const float NewNearPlane)
{
	NearPlane = NewNearPlane;
	UpdateProjectionMatrix();
}

float FEBasicCamera::GetFarPlane() const
{
	return FarPlane;
}

void FEBasicCamera::SetFarPlane(const float NewFarPlane)
{
	FarPlane = NewFarPlane;
	UpdateProjectionMatrix();
}

float FEBasicCamera::GetGamma() const
{
	return Gamma;
}

void FEBasicCamera::SetGamma(const float NewGamma)
{
	Gamma = NewGamma;
}

float FEBasicCamera::GetExposure() const
{
	return Exposure;
}

void FEBasicCamera::SetExposure(const float NewExposure)
{
	Exposure = NewExposure;
}

bool FEBasicCamera::GetIsInputActive()
{
	return bIsInputActive;
}

void FEBasicCamera::SetIsInputActive(const bool bIsActive)
{
	bIsInputActive = bIsActive;
}

void FocalEngine::FEBasicCamera::Reset()
{
	Fov = 70.0f;
	NearPlane = 0.1f;
	FarPlane = 5000.0f;
	AspectRatio = 1.0f;

	Yaw = 0.0f;
	Pitch = 0.0f;
	Roll = 0.0f;

	Gamma = 2.2f;
	Exposure = 1.0f;

	Position = glm::vec3(0.0f);

	UpdateAll();
}

void FEBasicCamera::SetLookAt(const glm::vec3 LookAt)
{
	ViewMatrix = glm::lookAt(GetPosition(), LookAt, glm::vec3(0.0, 1.0, 0.0));
}

glm::vec3 FEBasicCamera::GetForward()
{
	return glm::normalize(glm::vec3(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) * ViewMatrix));
}

void FEBasicCamera::SetOnUpdate(void(*Func)(FEBasicCamera*))
{
	ClientOnUpdateImpl = Func;
}

glm::vec3 FEBasicCamera::GetRight()
{
	return glm::normalize(glm::vec3(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * ViewMatrix));
}

glm::vec3 FEBasicCamera::GetUp()
{
	return glm::normalize(glm::vec3(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * ViewMatrix));
}

void FEBasicCamera::UpdateFrustumPlanes()
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

float** FEBasicCamera::GetFrustumPlanes()
{
	return Frustum;
}

float FEBasicCamera::GetMovementSpeed()
{
	return MovementSpeed;
}

void FEBasicCamera::SetMovementSpeed(const float NewValue)
{
	MovementSpeed = NewValue;
}