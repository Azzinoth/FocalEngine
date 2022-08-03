#include "FELight.h"
using namespace FocalEngine;

FELight::FELight(const FE_OBJECT_TYPE LightType) : FEObject(LightType, "")
{
}

FELight::~FELight()
{
}

glm::vec3 FELight::GetColor()
{
	return Color;
}

void FELight::SetColor(const glm::vec3 NewValue)
{
	Color = NewValue;
}

bool FELight::IsLightEnabled()
{
	return bEnabled;
}

void FELight::SetLightEnabled(const bool NewValue)
{
	bEnabled = NewValue;
}

float FELight::GetShadowBias()
{
	return ShadowBias;
}

void FELight::SetShadowBias(const float NewValue)
{
	ShadowBias = NewValue;
}

float FELight::GetIntensity()
{
	return Intensity;
}

void FELight::SetIntensity(const float NewValue)
{
	Intensity = NewValue;
}

float FELight::GetShadowBlurFactor()
{
	return ShadowBlurFactor;
}

void FELight::SetShadowBlurFactor(float NewValue)
{
	if (NewValue < 0.0f)
		NewValue = 0.0f;

	if (NewValue > 16.0f)
		NewValue = 16.0f;

	ShadowBlurFactor = NewValue;
}

bool FELight::IsCastShadows()
{
	return bCastShadows;
}

void FELight::SetCastShadows(const bool NewValue)
{
	bCastShadows = NewValue;
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

// old
//void FEDirectionalLight::updateCascades(float cameraFov, float aspectRatio, float nearPlane, float farPlane, glm::mat4 viewMatrix, glm::vec3 cameraForward, glm::vec3 cameraRight, glm::vec3 cameraUp)
//{
//	static glm::vec4 basisX = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
//	static glm::vec4 basisY = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
//	static glm::vec4 basisZ = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
//
//	glm::vec4 fbasisX = glm::normalize(glm::toMat4(transform.getQuaternion()) * basisX);
//	glm::vec4 fbasisY = glm::normalize(glm::toMat4(transform.getQuaternion()) * basisY);
//	glm::vec4 fbasisZ = glm::normalize(glm::toMat4(transform.getQuaternion()) * basisZ);
//
//	glm::mat4 cascadeView = glm::mat4(1.0f);
//
//	cascadeView[0][0] = fbasisX.x;
//	cascadeView[1][0] = fbasisX.y;
//	cascadeView[2][0] = fbasisX.z;
//	cascadeView[0][1] = fbasisY.x;
//	cascadeView[1][1] = fbasisY.y;
//	cascadeView[2][1] = fbasisY.z;
//	cascadeView[0][2] = fbasisZ.x;
//	cascadeView[1][2] = fbasisZ.y;
//	cascadeView[2][2] = fbasisZ.z;
//
//	farPlane = nearPlane;
//	glm::mat4 inverseVM = glm::inverse(viewMatrix);
//	static std::vector<glm::vec4> frustumEdges;
//	frustumEdges.resize(8);
//
//	float TEST_CASCADE_DISTANCE_SCALE_FACTOR = 1.8f;
//
//	for (size_t i = 0; i < 4; i++)
//	{
//		cascadeData[i].viewMat = cascadeView;
//
//		nearPlane = farPlane;
//		farPlane = shadowCoverage * (0.0447f * float(pow(2.1867f, (i + 1))));
//		cascadeData[i].size = float(int(farPlane) - 1) * TEST_CASCADE_DISTANCE_SCALE_FACTOR;
//		if (cascadeData[i].size <= 0.01f)
//			cascadeData[i].size = 1.0;
//
//		float firstCascadeY1 = nearPlane * tan(glm::radians(cameraFov / 2.0f));
//		float firstCascadeY2 = farPlane * tan(glm::radians(cameraFov / 2.0f));
//
//		float firstCascadeX1 = nearPlane * tan((aspectRatio) / 2.0f);
//		float firstCascadeX2 = farPlane * tan((aspectRatio) / 2.0f);
//
//		frustumEdges[0] = glm::vec4(firstCascadeX1, -firstCascadeY1, -nearPlane, 1.0f);
//		frustumEdges[1] = glm::vec4(firstCascadeX1, firstCascadeY1, -nearPlane, 1.0f);
//		frustumEdges[2] = glm::vec4(-firstCascadeX1, firstCascadeY1, -nearPlane, 1.0f);
//		frustumEdges[3] = glm::vec4(-firstCascadeX1, -firstCascadeY1, -nearPlane, 1.0f);
//
//		frustumEdges[4] = glm::vec4(firstCascadeX2, -firstCascadeY2, -farPlane, 1.0f);
//		frustumEdges[5] = glm::vec4(firstCascadeX2, firstCascadeY2, -farPlane, 1.0f);
//		frustumEdges[6] = glm::vec4(-firstCascadeX2, firstCascadeY2, -farPlane, 1.0f);
//		frustumEdges[7] = glm::vec4(-firstCascadeX2, -firstCascadeY2, -farPlane, 1.0f);
//
//		for (size_t j = 0; j < frustumEdges.size(); j++)
//			frustumEdges[j] = cascadeData[0].viewMat * inverseVM * frustumEdges[j];
//
//		for (size_t j = 0; j < frustumEdges.size(); j++)
//			frustumEdges[j].z = -frustumEdges[j].z;
//
//		float minX = FLT_MAX;
//		float maxX = FLT_MIN;
//		float minY = FLT_MAX;
//		float maxY = FLT_MIN;
//		float minZ = FLT_MAX;
//		float maxZ = FLT_MIN;
//
//		for (size_t j = 0; j < frustumEdges.size(); j++)
//		{
//			minX = std::min(minX, frustumEdges[j].x);
//			minY = std::min(minY, frustumEdges[j].y);
//			minZ = std::min(minZ, frustumEdges[j].z);
//
//			maxX = std::max(maxX, frustumEdges[j].x);
//			maxY = std::max(maxY, frustumEdges[j].y);
//			maxZ = std::max(maxZ, frustumEdges[j].z);
//		}
//
//		cascadeData[i].projectionMat = glm::ortho(minX - farPlane * (CSMXYDepth / 4.0f), maxX + farPlane * (CSMXYDepth / 4.0f),
//			minY - farPlane * (CSMXYDepth / 4.0f), maxY + farPlane * (CSMXYDepth / 4.0f),
//			minZ - farPlane * CSMZDepth, maxZ + farPlane * CSMZDepth);
//	}
//}


void FEDirectionalLight::UpdateCascades(float CameraFov, float AspectRatio, float NearPlane, float FarPlane, glm::mat4 ViewMatrix, glm::vec3 CameraForward, glm::vec3 CameraRight, glm::vec3 CameraUp)
{
	static glm::vec4 BasisX = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	static glm::vec4 BasisY = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	static glm::vec4 BasisZ = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	glm::vec4 FbasisX = glm::normalize(glm::toMat4(Transform.GetQuaternion()) * BasisX);
	glm::vec4 FbasisY = glm::normalize(glm::toMat4(Transform.GetQuaternion()) * BasisY);
	glm::vec4 FbasisZ = glm::normalize(glm::toMat4(Transform.GetQuaternion()) * BasisZ);

	glm::mat4 CascadeView = glm::mat4(1.0f);
	CascadeView[0][0] = FbasisX.x;
	CascadeView[1][0] = FbasisX.y;
	CascadeView[2][0] = FbasisX.z;
	CascadeView[0][1] = FbasisY.x;
	CascadeView[1][1] = FbasisY.y;
	CascadeView[2][1] = FbasisY.z;
	CascadeView[0][2] = FbasisZ.x;
	CascadeView[1][2] = FbasisZ.y;
	CascadeView[2][2] = FbasisZ.z;

	FarPlane = NearPlane;
	glm::mat4 InverseVm = glm::inverse(ViewMatrix);
	static std::vector<glm::vec4> FrustumEdges;
	FrustumEdges.resize(8);

	float TestCascadeDistanceScaleFactor = 4.0f;
	float TestCSM0 = 0.050f;
	float TestCSM1 = 0.150f;
	float TestCSM2 = 0.550f;
	float TestCSM3 = 1.1f;
	float TestCSMScale = 1.75f;

	float TestCSMType = 1;

	// old and incorrect
	if (TestCSMType == 0)
	{
		for (size_t i = 0; i < 4; i++)
		{
			CascadeData[i].ViewMat = CascadeView;

			NearPlane = FarPlane;
			FarPlane = ShadowCoverage * (0.0447f * static_cast<float>(pow(2.1867f, (i + 1))));
			CascadeData[i].Size = static_cast<float>(static_cast<int>(FarPlane) - 1) * TestCascadeDistanceScaleFactor;
			if (CascadeData[i].Size <= 0.01f)
				CascadeData[i].Size = 1.0;

			float FirstCascadeY1 = NearPlane * tan(glm::radians(CameraFov / 2.0f));
			float FirstCascadeY2 = FarPlane * tan(glm::radians(CameraFov / 2.0f));

			float FirstCascadeX1 = NearPlane * tan((AspectRatio) / 2.0f);
			float FirstCascadeX2 = FarPlane * tan((AspectRatio) / 2.0f);

			FrustumEdges[0] = glm::vec4(FirstCascadeX1, -FirstCascadeY1, -NearPlane, 1.0f);
			FrustumEdges[1] = glm::vec4(FirstCascadeX1, FirstCascadeY1, -NearPlane, 1.0f);
			FrustumEdges[2] = glm::vec4(-FirstCascadeX1, FirstCascadeY1, -NearPlane, 1.0f);
			FrustumEdges[3] = glm::vec4(-FirstCascadeX1, -FirstCascadeY1, -NearPlane, 1.0f);

			FrustumEdges[4] = glm::vec4(FirstCascadeX2, -FirstCascadeY2, -FarPlane, 1.0f);
			FrustumEdges[5] = glm::vec4(FirstCascadeX2, FirstCascadeY2, -FarPlane, 1.0f);
			FrustumEdges[6] = glm::vec4(-FirstCascadeX2, FirstCascadeY2, -FarPlane, 1.0f);
			FrustumEdges[7] = glm::vec4(-FirstCascadeX2, -FirstCascadeY2, -FarPlane, 1.0f);

			for (size_t j = 0; j < FrustumEdges.size(); j++)
				FrustumEdges[j] = CascadeData[0].ViewMat * InverseVm * FrustumEdges[j];

			for (size_t j = 0; j < FrustumEdges.size(); j++)
				FrustumEdges[j].z = -FrustumEdges[j].z;

			float MinX = FLT_MAX;
			float MaxX = FLT_MIN;
			float MinY = FLT_MAX;
			float MaxY = FLT_MIN;
			float MinZ = FLT_MAX;
			float MaxZ = FLT_MIN;

			for (size_t j = 0; j < FrustumEdges.size(); j++)
			{
				MinX = std::min(MinX, FrustumEdges[j].x);
				MinY = std::min(MinY, FrustumEdges[j].y);
				MinZ = std::min(MinZ, FrustumEdges[j].z);

				MaxX = std::max(MaxX, FrustumEdges[j].x);
				MaxY = std::max(MaxY, FrustumEdges[j].y);
				MaxZ = std::max(MaxZ, FrustumEdges[j].z);
			}

			CascadeData[i].ProjectionMat = glm::ortho(MinX - FarPlane * (CSMXYDepth / 4.0f), MaxX + FarPlane * (CSMXYDepth / 4.0f),
				MinY - FarPlane * (CSMXYDepth / 4.0f), MaxY + FarPlane * (CSMXYDepth / 4.0f),
				MinZ - FarPlane * CSMZDepth, MaxZ + FarPlane * CSMZDepth);
		}
	}
	else if (TestCSMType == 1)
	{
		for (size_t i = 0; i < 4; i++)
		{
			CascadeData[i].ViewMat = CascadeView;

			NearPlane = FarPlane;
			if (i == 0)
			{
				FarPlane = (ShadowCoverage / 4.0f) * TestCSM0;
			}
			else if (i == 1)
			{
				FarPlane = (ShadowCoverage / 4.0f) * TestCSM1;
			}
			else if (i == 2)
			{
				FarPlane = (ShadowCoverage / 4.0f) * TestCSM2;
			}
			else if (i == 3)
			{
				FarPlane = (ShadowCoverage / 4.0f) * TestCSM3;
			}
			CascadeData[i].Size = FarPlane * TestCSMScale;

			float FirstCascadeY1 = NearPlane * tan(glm::radians(CameraFov / 2.0f));
			float FirstCascadeY2 = FarPlane * tan(glm::radians(CameraFov / 2.0f));

			float FirstCascadeX1 = NearPlane * tan((AspectRatio) / 2.0f);
			float FirstCascadeX2 = FarPlane * tan((AspectRatio) / 2.0f);

			FrustumEdges[0] = glm::vec4(FirstCascadeX1, -FirstCascadeY1, -NearPlane, 1.0f);
			FrustumEdges[1] = glm::vec4(FirstCascadeX1, FirstCascadeY1, -NearPlane, 1.0f);
			FrustumEdges[2] = glm::vec4(-FirstCascadeX1, FirstCascadeY1, -NearPlane, 1.0f);
			FrustumEdges[3] = glm::vec4(-FirstCascadeX1, -FirstCascadeY1, -NearPlane, 1.0f);

			FrustumEdges[4] = glm::vec4(FirstCascadeX2, -FirstCascadeY2, -FarPlane, 1.0f);
			FrustumEdges[5] = glm::vec4(FirstCascadeX2, FirstCascadeY2, -FarPlane, 1.0f);
			FrustumEdges[6] = glm::vec4(-FirstCascadeX2, FirstCascadeY2, -FarPlane, 1.0f);
			FrustumEdges[7] = glm::vec4(-FirstCascadeX2, -FirstCascadeY2, -FarPlane, 1.0f);

			for (size_t j = 0; j < FrustumEdges.size(); j++)
				FrustumEdges[j] = CascadeData[0].ViewMat * InverseVm * FrustumEdges[j];

			for (size_t j = 0; j < FrustumEdges.size(); j++)
				FrustumEdges[j].z = -FrustumEdges[j].z;

			float MinX = FLT_MAX;
			float MaxX = -FLT_MAX;
			float MinY = FLT_MAX;
			float MaxY = -FLT_MAX;
			float MinZ = FLT_MAX;
			float MaxZ = -FLT_MAX;

			for (size_t j = 0; j < FrustumEdges.size(); j++)
			{
				MinX = std::min(MinX, FrustumEdges[j].x);
				MinY = std::min(MinY, FrustumEdges[j].y);
				MinZ = std::min(MinZ, FrustumEdges[j].z);

				MaxX = std::max(MaxX, FrustumEdges[j].x);
				MaxY = std::max(MaxY, FrustumEdges[j].y);
				MaxZ = std::max(MaxZ, FrustumEdges[j].z);
			}

			float left = MinX - FarPlane * (CSMXYDepth / 4.0f);
			float right = MaxX + FarPlane * (CSMXYDepth / 4.0f);

			float bottom = MinY - FarPlane * (CSMXYDepth / 4.0f);
			float top = MaxY + FarPlane * (CSMXYDepth / 4.0f);

			float ZNear = MinZ - FarPlane * CSMZDepth;
			float ZFar = MaxZ + FarPlane * CSMZDepth;

			CascadeData[i].ProjectionMat = glm::ortho(left, right,
													  bottom, top,
													  ZNear, ZFar);

			float clip[16];
			float t;

			glm::mat4 cliping = CascadeData[i].ProjectionMat * CascadeView;
			for (int j = 0; j < 4; j++)
			{
				clip[j * 4] = cliping[j][0];
				clip[j * 4 + 1] = cliping[j][1];
				clip[j * 4 + 2] = cliping[j][2];
				clip[j * 4 + 3] = cliping[j][3];
			}

			/* Extract the numbers for the RIGHT plane */
			CascadeData[i].Frustum[0][0] = clip[3] - clip[0];
			CascadeData[i].Frustum[0][1] = clip[7] - clip[4];
			CascadeData[i].Frustum[0][2] = clip[11] - clip[8];
			CascadeData[i].Frustum[0][3] = clip[15] - clip[12];

			/* Normalize the result */
			t = sqrt(CascadeData[i].Frustum[0][0] * CascadeData[i].Frustum[0][0] + CascadeData[i].Frustum[0][1] * CascadeData[i].Frustum[0][1] + CascadeData[i].Frustum[0][2] * CascadeData[i].Frustum[0][2]);
			CascadeData[i].Frustum[0][0] /= t;
			CascadeData[i].Frustum[0][1] /= t;
			CascadeData[i].Frustum[0][2] /= t;
			CascadeData[i].Frustum[0][3] /= t;

			/* Extract the numbers for the LEFT plane */
			CascadeData[i].Frustum[1][0] = clip[3] + clip[0];
			CascadeData[i].Frustum[1][1] = clip[7] + clip[4];
			CascadeData[i].Frustum[1][2] = clip[11] + clip[8];
			CascadeData[i].Frustum[1][3] = clip[15] + clip[12];

			/* Normalize the result */
			t = sqrt(CascadeData[i].Frustum[1][0] * CascadeData[i].Frustum[1][0] + CascadeData[i].Frustum[1][1] * CascadeData[i].Frustum[1][1] + CascadeData[i].Frustum[1][2] * CascadeData[i].Frustum[1][2]);
			CascadeData[i].Frustum[1][0] /= t;
			CascadeData[i].Frustum[1][1] /= t;
			CascadeData[i].Frustum[1][2] /= t;
			CascadeData[i].Frustum[1][3] /= t;

			/* Extract the BOTTOM plane */
			CascadeData[i].Frustum[2][0] = clip[3] + clip[1];
			CascadeData[i].Frustum[2][1] = clip[7] + clip[5];
			CascadeData[i].Frustum[2][2] = clip[11] + clip[9];
			CascadeData[i].Frustum[2][3] = clip[15] + clip[13];

			/* Normalize the result */
			t = sqrt(CascadeData[i].Frustum[2][0] * CascadeData[i].Frustum[2][0] + CascadeData[i].Frustum[2][1] * CascadeData[i].Frustum[2][1] + CascadeData[i].Frustum[2][2] * CascadeData[i].Frustum[2][2]);
			CascadeData[i].Frustum[2][0] /= t;
			CascadeData[i].Frustum[2][1] /= t;
			CascadeData[i].Frustum[2][2] /= t;
			CascadeData[i].Frustum[2][3] /= t;

			/* Extract the TOP plane */
			CascadeData[i].Frustum[3][0] = clip[3] - clip[1];
			CascadeData[i].Frustum[3][1] = clip[7] - clip[5];
			CascadeData[i].Frustum[3][2] = clip[11] - clip[9];
			CascadeData[i].Frustum[3][3] = clip[15] - clip[13];

			/* Normalize the result */
			t = sqrt(CascadeData[i].Frustum[3][0] * CascadeData[i].Frustum[3][0] + CascadeData[i].Frustum[3][1] * CascadeData[i].Frustum[3][1] + CascadeData[i].Frustum[3][2] * CascadeData[i].Frustum[3][2]);
			CascadeData[i].Frustum[3][0] /= t;
			CascadeData[i].Frustum[3][1] /= t;
			CascadeData[i].Frustum[3][2] /= t;
			CascadeData[i].Frustum[3][3] /= t;

			/* Extract the FAR plane */
			CascadeData[i].Frustum[4][0] = clip[3] - clip[2];
			CascadeData[i].Frustum[4][1] = clip[7] - clip[6];
			CascadeData[i].Frustum[4][2] = clip[11] - clip[10];
			CascadeData[i].Frustum[4][3] = clip[15] - clip[14];

			/* Normalize the result */
			t = sqrt(CascadeData[i].Frustum[4][0] * CascadeData[i].Frustum[4][0] + CascadeData[i].Frustum[4][1] * CascadeData[i].Frustum[4][1] + CascadeData[i].Frustum[4][2] * CascadeData[i].Frustum[4][2]);
			CascadeData[i].Frustum[4][0] /= t;
			CascadeData[i].Frustum[4][1] /= t;
			CascadeData[i].Frustum[4][2] /= t;
			CascadeData[i].Frustum[4][3] /= t;

			/* Extract the NEAR plane */
			CascadeData[i].Frustum[5][0] = clip[3] + clip[2];
			CascadeData[i].Frustum[5][1] = clip[7] + clip[6];
			CascadeData[i].Frustum[5][2] = clip[11] + clip[10];
			CascadeData[i].Frustum[5][3] = clip[15] + clip[14];

			/* Normalize the result */
			t = sqrt(CascadeData[i].Frustum[5][0] * CascadeData[i].Frustum[5][0] + CascadeData[i].Frustum[5][1] * CascadeData[i].Frustum[5][1] + CascadeData[i].Frustum[5][2] * CascadeData[i].Frustum[5][2]);
			CascadeData[i].Frustum[5][0] /= t;
			CascadeData[i].Frustum[5][1] /= t;
			CascadeData[i].Frustum[5][2] /= t;
			CascadeData[i].Frustum[5][3] /= t;

			int ty = 0;
			ty++;
		}
	}
	// my way of frustum reconstruction
	else if (TestCSMType == 2)
	{
		float NearPlaneXLength = NearPlane * glm::tan(AspectRatio / 2.0f) * 2.0f;
		float NearPlaneYLength = NearPlaneXLength / AspectRatio;

		glm::vec3 CameraPos = glm::vec3(ViewMatrix[3][0], ViewMatrix[3][1], ViewMatrix[3][2]);

		glm::vec3 NearTopLeft = CameraPos;
		NearTopLeft += -CameraRight * (NearPlaneXLength / 2.0f);
		NearTopLeft += CameraUp * (NearPlaneYLength / 2.0f);
		NearTopLeft += CameraForward * NearPlane;

		glm::vec3 NearTopRight = CameraPos;
		NearTopRight += CameraRight * (NearPlaneXLength / 2.0f);
		NearTopRight += CameraUp * (NearPlaneYLength / 2.0f);
		NearTopRight += CameraForward * NearPlane;

		glm::vec3 NearBottomLeft = CameraPos;
		NearBottomLeft += -CameraRight * (NearPlaneXLength / 2.0f);
		NearBottomLeft += -CameraUp * (NearPlaneYLength / 2.0f);
		NearBottomLeft += CameraForward * NearPlane;

		glm::vec3 NearBottomRight = CameraPos;
		NearBottomRight += CameraRight * (NearPlaneXLength / 2.0f);
		NearBottomRight += -CameraUp * (NearPlaneYLength / 2.0f);
		NearBottomRight += CameraForward * NearPlane;

		glm::vec3 CameraToTopLeft = glm::normalize(NearTopLeft - CameraPos);
		glm::vec3 CameraToTopRight = glm::normalize(NearTopRight - CameraPos);
		glm::vec3 CameraToBottomLeft = glm::normalize(NearBottomLeft - CameraPos);
		glm::vec3 CameraToBottomRight = glm::normalize(NearBottomRight - CameraPos);

		for (size_t i = 0; i < 4; i++)
		{
			CascadeData[i].ViewMat = CascadeView;

			NearPlane = FarPlane;
			if (i == 0)
			{
				FarPlane = (ShadowCoverage / 4.0f) * TestCSM0;
			}
			else if (i == 1)
			{
				FarPlane = (ShadowCoverage / 4.0f) * TestCSM1;
			}
			else if (i == 2)
			{
				FarPlane = (ShadowCoverage / 4.0f) * TestCSM2;
			}
			else if (i == 3)
			{
				FarPlane = (ShadowCoverage / 4.0f) * TestCSM3;
			}
			CascadeData[i].Size = FarPlane * TestCSMScale;

			CameraPos = glm::vec3(0.0f);

			FrustumEdges[0] = glm::vec4(CameraPos + CameraToTopLeft * NearPlane, 1.0f);
			FrustumEdges[1] = glm::vec4(CameraPos + CameraToTopRight * NearPlane, 1.0f);
			FrustumEdges[2] = glm::vec4(CameraPos + CameraToBottomLeft * NearPlane, 1.0f);
			FrustumEdges[3] = glm::vec4(CameraPos + CameraToBottomRight * NearPlane, 1.0f);

			FrustumEdges[4] = glm::vec4(CameraPos + CameraToTopLeft * FarPlane, 1.0f);
			FrustumEdges[5] = glm::vec4(CameraPos + CameraToTopRight * FarPlane, 1.0f);
			FrustumEdges[6] = glm::vec4(CameraPos + CameraToBottomLeft * FarPlane, 1.0f);
			FrustumEdges[7] = glm::vec4(CameraPos + CameraToBottomRight * FarPlane, 1.0f);

			for (size_t j = 0; j < FrustumEdges.size(); j++)
				FrustumEdges[j] = CascadeData[0].ViewMat * InverseVm * FrustumEdges[j];

			for (size_t j = 0; j < FrustumEdges.size(); j++)
				FrustumEdges[j].z = -FrustumEdges[j].z;

			float MinX = FLT_MAX;
			float MaxX = -FLT_MAX;
			float MinY = FLT_MAX;
			float MaxY = -FLT_MAX;
			float MinZ = FLT_MAX;
			float MaxZ = -FLT_MAX;

			for (size_t j = 0; j < FrustumEdges.size(); j++)
			{
				MinX = std::min(MinX, FrustumEdges[j].x);
				MinY = std::min(MinY, FrustumEdges[j].y);
				MinZ = std::min(MinZ, FrustumEdges[j].z);

				MaxX = std::max(MaxX, FrustumEdges[j].x);
				MaxY = std::max(MaxY, FrustumEdges[j].y);
				MaxZ = std::max(MaxZ, FrustumEdges[j].z);
			}

			CascadeData[i].ProjectionMat = glm::ortho(MinX - FarPlane * (CSMXYDepth / 4.0f), MaxX + FarPlane * (CSMXYDepth / 4.0f),
				MinY - FarPlane * (CSMXYDepth / 4.0f), MaxY + FarPlane * (CSMXYDepth / 4.0f),
				MinZ - FarPlane * CSMZDepth, MaxZ + FarPlane * CSMZDepth);
		}
	}
}

FEDirectionalLight::FEDirectionalLight() : FELight(FE_DIRECTIONAL_LIGHT)
{
}

glm::vec3 FEDirectionalLight::GetDirection()
{
	Direction = glm::normalize(Transform.GetTransformMatrix() * glm::vec4(DefaultDirection, 0.0f));
	return Direction;
}

void FEDirectionalLight::SetDirection(const glm::vec3 NewValue)
{
	Direction = NewValue;
}

FEDirectionalLight::~FEDirectionalLight()
{
	delete CascadeData[0].FrameBuffer;
	delete CascadeData[1].FrameBuffer;
	delete CascadeData[2].FrameBuffer;
	delete CascadeData[3].FrameBuffer;
}

int FEDirectionalLight::GetActiveCascades()
{
	return ActiveCascades;
}

void FEDirectionalLight::SetActiveCascades(int NewValue)
{
	if (NewValue < 1 || NewValue > 4)
		NewValue = 1;

	ActiveCascades = NewValue;
}

float FEDirectionalLight::GetShadowCoverage()
{
	return ShadowCoverage;
}

void FEDirectionalLight::SetShadowCoverage(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.1f;

	ShadowCoverage = NewValue;
	//updateProjectionMat();
}

float FEDirectionalLight::GetCSMZDepth()
{
	return CSMZDepth;
}

void FEDirectionalLight::SetCSMZDepth(float NewValue)
{
	if (NewValue <= 0.5f)
		NewValue = 0.5f;

	CSMZDepth = NewValue;
}

float FEDirectionalLight::GetCSMXYDepth()
{
	return CSMXYDepth;
}

void FEDirectionalLight::SetCSMXYDepth(float NewValue)
{
	if (NewValue <= 0.5f)
		NewValue = 0.5f;

	CSMXYDepth = NewValue;
}

void FEDirectionalLight::SetCastShadows(const bool NewValue)
{
	FELight::SetCastShadows(NewValue);
	if (!NewValue)
	{
		for (size_t i = 0; i < static_cast<size_t>(ActiveCascades); i++)
		{
			CascadeData[i].FrameBuffer->Bind();
			FE_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));
			CascadeData[i].FrameBuffer->UnBind();
		}
	}
}

FESpotLight::FESpotLight() : FELight(FE_SPOT_LIGHT)
{
}

FESpotLight::~FESpotLight()
{
}

float FESpotLight::GetSpotAngle()
{
	return SpotAngle;
}

void FESpotLight::SetSpotAngle(const float NewValue)
{
	SpotAngle = NewValue;
}

float FESpotLight::GetSpotAngleOuter()
{
	return SpotAngleOuter;
}

void FESpotLight::SetSpotAngleOuter(const float NewValue)
{
	SpotAngleOuter = NewValue;
}

float FESpotLight::GetRange()
{
	return Range;
}

void FESpotLight::SetRange(const float NewValue)
{
	Range = NewValue;
}

glm::vec3 FESpotLight::GetDirection()
{
	Direction = glm::normalize(Transform.GetTransformMatrix() * glm::vec4(DefaultDirection, 0.0f));
	return Direction;
}

void FESpotLight::SetDirection(const glm::vec3 NewValue)
{
	Direction = NewValue;
}

FEPointLight::FEPointLight() : FELight(FE_POINT_LIGHT)
{
}

FEPointLight::~FEPointLight()
{
}

float FEPointLight::GetRange()
{
	return Range;
}

void FEPointLight::SetRange(const float NewValue)
{
	Range = NewValue;
}

bool FELight::IsStaticShadowBias()
{
	return bStaticShadowBias;
}

void FELight::SetIsStaticShadowBias(const bool NewValue)
{
	bStaticShadowBias = NewValue;
}

float FELight::GetShadowBiasVariableIntensity()
{
	return ShadowBiasVariableIntensity;
}

void FELight::SetShadowBiasVariableIntensity(float NewValue)
{
	if (NewValue <= 0.0f)
		NewValue = 0.01f;

	ShadowBiasVariableIntensity = NewValue;
}