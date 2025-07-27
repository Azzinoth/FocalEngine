#include "FECameraComponent.h"
#include "../ResourceManager/FEResourceManager.h"
#include "Systems/FECameraSystem.h"
#include "../../../FEngine.h"
using namespace FocalEngine;

FECameraComponent::FECameraComponent()
{
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

bool FECameraComponent::IsRenderingEnabled() const
{
	return bRenderingEnabled;
}

void FECameraComponent::SetRenderingEnabled(const bool bEnabled)
{
	bRenderingEnabled = bEnabled;
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

void FECameraComponent::UpdateFrustum()
{
	glm::mat4 ViewProjectionMatrix = GetProjectionMatrix() * GetViewMatrix();

	// Left plane
	Frustum.LeftPlane.SetFromGeneralFormCoefficients(ViewProjectionMatrix[0][3] + ViewProjectionMatrix[0][0],
													 ViewProjectionMatrix[1][3] + ViewProjectionMatrix[1][0],
													 ViewProjectionMatrix[2][3] + ViewProjectionMatrix[2][0],
													 ViewProjectionMatrix[3][3] + ViewProjectionMatrix[3][0]);

	// Right plane
	Frustum.RightPlane.SetFromGeneralFormCoefficients(ViewProjectionMatrix[0][3] - ViewProjectionMatrix[0][0],
													  ViewProjectionMatrix[1][3] - ViewProjectionMatrix[1][0],
													  ViewProjectionMatrix[2][3] - ViewProjectionMatrix[2][0],
													  ViewProjectionMatrix[3][3] - ViewProjectionMatrix[3][0]);

	// Bottom plane
	Frustum.BottomPlane.SetFromGeneralFormCoefficients(ViewProjectionMatrix[0][3] + ViewProjectionMatrix[0][1],
													   ViewProjectionMatrix[1][3] + ViewProjectionMatrix[1][1],
													   ViewProjectionMatrix[2][3] + ViewProjectionMatrix[2][1],
													   ViewProjectionMatrix[3][3] + ViewProjectionMatrix[3][1]);

	// Top plane
	Frustum.TopPlane.SetFromGeneralFormCoefficients(ViewProjectionMatrix[0][3] - ViewProjectionMatrix[0][1],
													ViewProjectionMatrix[1][3] - ViewProjectionMatrix[1][1],
													ViewProjectionMatrix[2][3] - ViewProjectionMatrix[2][1],
													ViewProjectionMatrix[3][3] - ViewProjectionMatrix[3][1]);

	// Near plane
	Frustum.NearPlane.SetFromGeneralFormCoefficients(ViewProjectionMatrix[0][3] + ViewProjectionMatrix[0][2],
													 ViewProjectionMatrix[1][3] + ViewProjectionMatrix[1][2],
													 ViewProjectionMatrix[2][3] + ViewProjectionMatrix[2][2],
													 ViewProjectionMatrix[3][3] + ViewProjectionMatrix[3][2]);

	// Far plane
	Frustum.FarPlane.SetFromGeneralFormCoefficients(ViewProjectionMatrix[0][3] - ViewProjectionMatrix[0][2],
													ViewProjectionMatrix[1][3] - ViewProjectionMatrix[1][2],
													ViewProjectionMatrix[2][3] - ViewProjectionMatrix[2][2],
													ViewProjectionMatrix[3][3] - ViewProjectionMatrix[3][2]);
}

FEFrustum FECameraComponent::GetFrustum()
{
	return Frustum;
}

std::vector<float> FEFrustum::GetAllPlanesCoefficients()
{
	std::vector<float> Coefficients;

	glm::vec4 RightPlaneCoefficients = RightPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(RightPlaneCoefficients.x);
	Coefficients.push_back(RightPlaneCoefficients.y);
	Coefficients.push_back(RightPlaneCoefficients.z);
	Coefficients.push_back(RightPlaneCoefficients.w);

	glm::vec4 LeftPlaneCoefficients = LeftPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(LeftPlaneCoefficients.x);
	Coefficients.push_back(LeftPlaneCoefficients.y);
	Coefficients.push_back(LeftPlaneCoefficients.z);
	Coefficients.push_back(LeftPlaneCoefficients.w);

	glm::vec4 BottomPlaneCoefficients = BottomPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(BottomPlaneCoefficients.x);
	Coefficients.push_back(BottomPlaneCoefficients.y);
	Coefficients.push_back(BottomPlaneCoefficients.z);
	Coefficients.push_back(BottomPlaneCoefficients.w);

	glm::vec4 TopPlaneCoefficients = TopPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(TopPlaneCoefficients.x);
	Coefficients.push_back(TopPlaneCoefficients.y);
	Coefficients.push_back(TopPlaneCoefficients.z);
	Coefficients.push_back(TopPlaneCoefficients.w);

	glm::vec4 FarPlaneCoefficients = FarPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(FarPlaneCoefficients.x);
	Coefficients.push_back(FarPlaneCoefficients.y);
	Coefficients.push_back(FarPlaneCoefficients.z);
	Coefficients.push_back(FarPlaneCoefficients.w);

	glm::vec4 NearPlaneCoefficients = NearPlane.GetGeneralFormCoefficients();
	Coefficients.push_back(NearPlaneCoefficients.x);
	Coefficients.push_back(NearPlaneCoefficients.y);
	Coefficients.push_back(NearPlaneCoefficients.z);
	Coefficients.push_back(NearPlaneCoefficients.w);

	return Coefficients;
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

bool FECameraComponent::IsSSAOResultBlurred()
{
	return bSSAOBlurred;
}

void FECameraComponent::SetSSAOResultBlurred(const bool NewValue)
{
	bSSAOBlurred = NewValue;
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