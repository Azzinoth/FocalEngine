#include "FECameraComponent.h"
#include "../ResourceManager/FEResourceManager.h"
#include "Systems/FECameraSystem.h"
#include "../../../FEngine.h"
using namespace FocalEngine;

FECameraComponent::FECameraComponent()
{
	Frustum.resize(6);
	for (size_t i = 0; i < Frustum.size(); i++)
		Frustum[i].resize(4);

	Viewport = new FEViewport();
}

bool FECameraComponent::IsMainCamera() const
{
	return bIsMainCamera;
}

glm::mat4 FECameraComponent::GetViewMatrix() const
{
	return ViewMatrix;
}

void FECameraComponent::SetViewMatrix(const glm::mat4 NewViewMatrix)
{
	ViewMatrix = NewViewMatrix;
}

glm::mat4 FECameraComponent::GetProjectionMatrix() const
{
	return ProjectionMatrix;
}

void FECameraComponent::SetProjectionMatrix(const glm::mat4 NewProjectionMatrix)
{
	ProjectionMatrix = NewProjectionMatrix;
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

bool FECameraComponent::IsClearColorEnabled() const
{
	return bClearColorEnabled;
}

void FECameraComponent::SetIsClearColorEnabled(const bool bEnabled)
{
	bClearColorEnabled = bEnabled;
}

glm::vec4 FECameraComponent::GetClearColor() const
{
	return ClearColor;
}

void FECameraComponent::SetClearColor(glm::vec4 NewClearColor)
{
	ClearColor = NewClearColor;
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

float FECameraComponent::GetRenderScale()
{
	return RenderScale;
}

int FECameraComponent::GetRenderTargetWidth() const
{
	if (Viewport == nullptr)
	{
		LOG.Add("FECameraComponent::GetRenderTargetWidth Viewport is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return RenderTargetWidth;
	}

	return static_cast<int>(Viewport->GetWidth() * RenderScale);
}

int FECameraComponent::GetRenderTargetHeight() const
{
	if (Viewport == nullptr)
	{
		LOG.Add("FECameraComponent::GetRenderTargetHeight Viewport is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return RenderTargetHeight;
	}

	return static_cast<int>(Viewport->GetHeight() * RenderScale);
}

bool FECameraComponent::TryToSetViewportSize(const int Width, const int Height)
{
	return TryToSetViewportSizeInternal(Width, Height);
}

bool FECameraComponent::TryToSetViewportSizeInternal(const int Width, const int Height)
{
	if (Width < 1 || Height < 1)
		return false;

	if (Viewport == nullptr)
	{
		LOG.Add("FECameraComponent::TryToSetViewportSizeInternal Viewport is nullptr.", "FE_LOG_RENDERING", FE_LOG_ERROR);
		return false;
	}

	if (Viewport->GetType() == FE_VIEWPORT_VIRTUAL)
	{
		Viewport->SetWidth(Width);
		Viewport->SetHeight(Height);
		RenderScale = 1.0f;
		AspectRatio = static_cast<float>(Viewport->GetWidth()) / static_cast<float>(Viewport->GetHeight());

		return true;
	}
	else
	{
		// If the viewport is not virtual this function should fail.
		LOG.Add("FECameraComponent::TryToSetViewportSizeInternal Viewport is not virtual.", "FE_LOG_RENDERING", FE_LOG_WARNING);
		return false;
	}

	return false;
}

float FECameraComponent::GetBloomThreshold()
{
	return BloomThreshold;
}

void FECameraComponent::SetBloomThreshold(float NewValue)
{
	BloomThreshold = NewValue;
}

float FECameraComponent::GetBloomSize()
{
	return BloomSize;
}

void FECameraComponent::SetBloomSize(float NewValue)
{
	BloomSize = NewValue;
}

bool FECameraComponent::IsTemporalJitterEnabled()
{
	return bTemporalJitterEnabled;
}

void FECameraComponent::SetTemporalJitterEnabled(const bool NewValue)
{
	bTemporalJitterEnabled = NewValue;
}

size_t FECameraComponent::GetTemporalJitterSequenceLength()
{
	return TemporalJitterSequenceLength;
}

void FECameraComponent::SetTemporalJitterSequenceLength(size_t NewValue)
{
	TemporalJitterSequenceLength = NewValue;
}

glm::vec2 FECameraComponent::GetTemporalJitterOffset()
{
	if (!bTemporalJitterEnabled)
		return glm::vec2(0.0f);

	return CurrentTemporalJitterOffset;
}

// Halton jitter
void FECameraComponent::UpdateTemporalJitterOffset()
{
	if (LastTemporalFrameIndexUpdateEngineFrame == ENGINE.GetCurrentFrameIndex())
		return;

	LastTemporalFrameIndexUpdateEngineFrame = ENGINE.GetCurrentFrameIndex();

	CurrentTemporalJitterOffset = glm::vec2(0.0f, 0.0f);
	TemporalFrameIndex = (TemporalFrameIndex + 1) % TemporalJitterSequenceLength;

	constexpr int BaseX = 2;
	int Index = TemporalFrameIndex + 1;
	float InvBase = 1.0f / BaseX;
	float Fraction = InvBase;
	while (Index > 0)
	{
		CurrentTemporalJitterOffset.x += (Index % BaseX) * Fraction;
		Index /= BaseX;
		Fraction *= InvBase;
	}

	constexpr int BaseY = 3;
	Index = TemporalFrameIndex + 1;
	InvBase = 1.0f / BaseY;
	Fraction = InvBase;
	while (Index > 0)
	{
		CurrentTemporalJitterOffset.y += (Index % BaseY) * Fraction;
		Index /= BaseY;
		Fraction *= InvBase;
	}

	CurrentTemporalJitterOffset.x -= 0.5f;
	CurrentTemporalJitterOffset.y -= 0.5f;
}

float FECameraComponent::GetFXAASpanMax()
{
	return FXAASpanMax;
}

void FECameraComponent::SetFXAASpanMax(float NewValue)
{
	FXAASpanMax = NewValue;
}

float FECameraComponent::GetFXAAReduceMin()
{
	return FXAAReduceMin;
}

void FECameraComponent::SetFXAAReduceMin(float NewValue)
{
	FXAAReduceMin = NewValue;
}

float FECameraComponent::GetFXAAReduceMul()
{
	return FXAAReduceMul;
}

void FECameraComponent::SetFXAAReduceMul(float NewValue)
{
	FXAAReduceMul = NewValue;
}

float FECameraComponent::GetDOFNearDistance()
{
	return DOFNearDistance;
}

void FECameraComponent::SetDOFNearDistance(float NewValue)
{
	DOFNearDistance = NewValue;
}

float FECameraComponent::GetDOFFarDistance()
{
	return DOFFarDistance;
}

void FECameraComponent::SetDOFFarDistance(float NewValue)
{
	DOFFarDistance = NewValue;
}

float FECameraComponent::GetDOFStrength()
{
	return DOFStrength;
}

void FECameraComponent::SetDOFStrength(float NewValue)
{
	DOFStrength = NewValue;
}

float FECameraComponent::GetDOFDistanceDependentStrength()
{
	return DOFDistanceDependentStrength;
}

void FECameraComponent::SetDOFDistanceDependentStrength(float NewValue)
{
	DOFDistanceDependentStrength = NewValue;
}

float FECameraComponent::GetChromaticAberrationIntensity()
{
	return ChromaticAberrationIntensity;
}

void FECameraComponent::SetChromaticAberrationIntensity(float NewValue)
{
	ChromaticAberrationIntensity = NewValue;
}

bool FECameraComponent::IsSSAOEnabled()
{
	return bSSAOActive;
}

void FECameraComponent::SetSSAOEnabled(const bool NewValue)
{
	bSSAOActive = NewValue;
}

int FECameraComponent::GetSSAOSampleCount()
{
	return SSAOSampleCount;
}

void FECameraComponent::SetSSAOSampleCount(int NewValue)
{
	if (NewValue < 1)
		NewValue = 1;

	if (NewValue > 64)
		NewValue = 64;

	SSAOSampleCount = NewValue;
}

bool FECameraComponent::IsSSAOSmallDetailsEnabled()
{
	return bSSAOSmallDetails;
}

void FECameraComponent::SetSSAOSmallDetailsEnabled(const bool NewValue)
{
	bSSAOSmallDetails = NewValue;
}

bool FECameraComponent::IsSSAOResultBlured()
{
	return bSSAOBlured;
}

void FECameraComponent::SetSSAOResultBlured(const bool NewValue)
{
	bSSAOBlured = NewValue;
}

float FECameraComponent::GetSSAOBias()
{
	return SSAOBias;
}

void FECameraComponent::SetSSAOBias(const float NewValue)
{
	SSAOBias = NewValue;
}

float FECameraComponent::GetSSAORadius()
{
	return SSAORadius;
}

void FECameraComponent::SetSSAORadius(const float NewValue)
{
	SSAORadius = NewValue;
}

float FECameraComponent::GetSSAORadiusSmallDetails()
{
	return SSAORadiusSmallDetails;
}

void FECameraComponent::SetSSAORadiusSmallDetails(const float NewValue)
{
	SSAORadiusSmallDetails = NewValue;
}

float FECameraComponent::GetSSAOSmallDetailsWeight()
{
	return SSAOSmallDetailsWeight;
}

void FECameraComponent::SetSSAOSmallDetailsWeight(const float NewValue)
{
	SSAOSmallDetailsWeight = NewValue;
}

bool FECameraComponent::IsDistanceFogEnabled()
{
	return bDistanceFogEnabled;
}

void FECameraComponent::SetDistanceFogEnabled(const bool NewValue)
{
	if (bDistanceFogEnabled == false && NewValue == true)
	{
		if (DistanceFogDensity <= 0.0f)
			DistanceFogDensity = 0.007f;
		if (DistanceFogGradient <= 0.0f)
			DistanceFogGradient = 2.5f;
	}
	bDistanceFogEnabled = NewValue;
}

float FECameraComponent::GetDistanceFogDensity()
{
	return DistanceFogDensity;
}

void FECameraComponent::SetDistanceFogDensity(const float NewValue)
{
	DistanceFogDensity = NewValue;
}

float FECameraComponent::GetDistanceFogGradient()
{
	return DistanceFogGradient;
}

void FECameraComponent::SetDistanceFogGradient(const float NewValue)
{
	DistanceFogGradient = NewValue;
}

const FEViewport* FECameraComponent::GetViewport()
{
	return Viewport;
}

FERenderingPipeline FECameraComponent::GetRenderingPipeline() const
{
	return RenderingPipeline;
}