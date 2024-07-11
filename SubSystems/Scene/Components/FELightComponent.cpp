#include "FELightComponent.h"
using namespace FocalEngine;

FELightComponent::FELightComponent()
{
	Type = FE_NULL_LIGHT;
}

FELightComponent::FELightComponent(FE_LIGHT_TYPE Type)
{
	this->Type = Type;
}

FELightComponent::~FELightComponent() {}

FE_LIGHT_TYPE FELightComponent::GetType()
{
	return Type;
}

glm::vec3 FELightComponent::GetColor()
{
	return Color;
}

void FELightComponent::SetColor(const glm::vec3 NewValue)
{
	Color = NewValue;
}

bool FELightComponent::IsLightEnabled()
{
	return bEnabled;
}

void FELightComponent::SetLightEnabled(const bool NewValue)
{
	bEnabled = NewValue;
}

float FELightComponent::GetShadowBias()
{
	return ShadowBias;
}

void FELightComponent::SetShadowBias(const float NewValue)
{
	ShadowBias = NewValue;
}

float FELightComponent::GetIntensity()
{
	return Intensity;
}

void FELightComponent::SetIntensity(const float NewValue)
{
	Intensity = NewValue;
}

float FELightComponent::GetRange()
{
	return Range;
}

void FELightComponent::SetRange(const float NewValue)
{
	Range = NewValue;
}

float FELightComponent::GetShadowBlurFactor()
{
	return ShadowBlurFactor;
}

void FELightComponent::SetShadowBlurFactor(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 16.0f)
		NewValue = 16.0f;

	ShadowBlurFactor = NewValue;
}

bool FELightComponent::IsCastShadows()
{
	return bCastShadows;
}

void FELightComponent::SetCastShadows(const bool NewValue)
{
	bCastShadows = NewValue;
	if (!NewValue && Type == FE_DIRECTIONAL_LIGHT)
	{
		for (size_t i = 0; i < static_cast<size_t>(ActiveCascades); i++)
		{
			CascadeData[i].FrameBuffer->Bind();
			FE_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));
			CascadeData[i].FrameBuffer->UnBind();
		}
	}
}

bool FELightComponent::IsStaticShadowBias()
{
	return bStaticShadowBias;
}

void FELightComponent::SetIsStaticShadowBias(const bool NewValue)
{
	bStaticShadowBias = NewValue;
}

float FELightComponent::GetShadowBiasVariableIntensity()
{
	return ShadowBiasVariableIntensity;
}

void FELightComponent::SetShadowBiasVariableIntensity(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.01f;

	ShadowBiasVariableIntensity = NewValue;
}

float FELightComponent::GetSpotAngle()
{
	return SpotAngle;
}

void FELightComponent::SetSpotAngle(const float NewValue)
{
	SpotAngle = NewValue;
}

float FELightComponent::GetSpotAngleOuter()
{
	return SpotAngleOuter;
}

void FELightComponent::SetSpotAngleOuter(const float NewValue)
{
	SpotAngleOuter = NewValue;
}

int FELightComponent::GetActiveCascades()
{
	return ActiveCascades;
}

void FELightComponent::SetActiveCascades(int NewValue)
{
	if (NewValue < 1 || NewValue > 4)
		NewValue = 1;

	ActiveCascades = NewValue;
}

float FELightComponent::GetShadowCoverage()
{
	return ShadowCoverage;
}

void FELightComponent::SetShadowCoverage(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.1f;

	ShadowCoverage = NewValue;
}

float FELightComponent::GetCSMZDepth()
{
	return CSMZDepth;
}

void FELightComponent::SetCSMZDepth(float NewValue)
{
	if (NewValue <= 0.5f)
		NewValue = 0.5f;

	CSMZDepth = NewValue;
}

float FELightComponent::GetCSMXYDepth()
{
	return CSMXYDepth;
}

void FELightComponent::SetCSMXYDepth(float NewValue)
{
	if (NewValue <= 0.5f)
		NewValue = 0.5f;

	CSMXYDepth = NewValue;
}

FECascadeData::FECascadeData()
{
	Frustum = new float* [6];
	for (size_t i = 0; i < 6; i++)
	{
		Frustum[i] = new float[4];
	}
}

FECascadeData::~FECascadeData()
{
	for (size_t i = 0; i < 6; i++)
	{
		delete[] Frustum[i];
	}
	delete[] Frustum;
}