#include "FELightSystem.h"
using namespace FocalEngine;

FELightSystem* FELightSystem::Instance = nullptr;
FELightSystem::FELightSystem()
{
	RegisterOnComponentCallbacks();
	SCENE.AddOnSceneClearCallback(std::bind(&FELightSystem::OnSceneClear, this));
}

void FELightSystem::RegisterOnComponentCallbacks()
{
	SCENE.RegisterOnComponentConstructCallback<FELightComponent>(OnMyComponentAdded);
	SCENE.RegisterOnComponentDestroyCallback<FELightComponent>(OnMyComponentDestroy);
}

void FELightSystem::OnSceneClear()
{
	RegisterOnComponentCallbacks();
}

void FELightSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FELightComponent>())
		return;

	FELightComponent& LightComponent = Entity->GetComponent<FELightComponent>();
	if (LightComponent.GetType() == FE_DIRECTIONAL_LIGHT)
	{
		LightComponent.CascadeData[0].FrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		LightComponent.CascadeData[1].FrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		LightComponent.CascadeData[2].FrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);
		LightComponent.CascadeData[3].FrameBuffer = RESOURCE_MANAGER.CreateFramebuffer(FE_DEPTH_ATTACHMENT, 1024 * 2, 1024 * 2);

		// To clear cascades framebuffers.
		LightComponent.SetCastShadows(false);
		LightComponent.SetCastShadows(true);
	}
}

void FELightSystem::OnMyComponentDestroy(FEEntity* Entity)
{
	if (Entity == nullptr || !Entity->HasComponent<FELightComponent>())
		return;

	FELightComponent& LightComponent = Entity->GetComponent<FELightComponent>();
	if (LightComponent.GetType() == FE_DIRECTIONAL_LIGHT)
	{
		delete LightComponent.CascadeData[0].FrameBuffer;
		delete LightComponent.CascadeData[1].FrameBuffer;
		delete LightComponent.CascadeData[2].FrameBuffer;
		delete LightComponent.CascadeData[3].FrameBuffer;
	}
}

FELightSystem::~FELightSystem() {};

glm::vec3 FELightSystem::GetDirection(FEEntity* LightEntity)
{
	if (LightEntity == nullptr || !LightEntity->HasComponent<FELightComponent>())
	{
		LOG.Add("FELightSystem::GetDirection LightEntity is nullptr or does not have FELightComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return glm::vec3(0.0f);
	}
	FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();

	FETransformComponent& Transform = LightEntity->GetComponent<FETransformComponent>();
	LightComponent.Direction = glm::normalize(Transform.GetWorldMatrix() * glm::vec4(LightComponent.DefaultDirection, 0.0f));

	return LightComponent.Direction;
}

void FELightSystem::UpdateCascades(FEEntity* LightEntity, float CameraFov, float AspectRatio, float NearPlane, float FarPlane, glm::mat4 ViewMatrix, glm::vec3 CameraForward, glm::vec3 CameraRight, glm::vec3 CameraUp)
{
	if (LightEntity == nullptr || !LightEntity->HasComponent<FELightComponent>())
	{
		LOG.Add("FELightSystem::GetDirection LightEntity is nullptr or does not have FELightComponent", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}
	FELightComponent& LightComponent = LightEntity->GetComponent<FELightComponent>();

	if (LightComponent.GetType() != FE_DIRECTIONAL_LIGHT)
	{
		LOG.Add("FELightSystem::UpdateCascades LightComponent is not a directional light", "FE_LOG_GENERAL", FE_LOG_WARNING);
		return;
	}

	FETransformComponent& TransformComponent = LightEntity->GetComponent<FETransformComponent>();

	static glm::vec4 BasisX = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	static glm::vec4 BasisY = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	static glm::vec4 BasisZ = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	glm::vec4 FbasisX = glm::normalize(glm::toMat4(TransformComponent.GetQuaternion()) * BasisX);
	glm::vec4 FbasisY = glm::normalize(glm::toMat4(TransformComponent.GetQuaternion()) * BasisY);
	glm::vec4 FbasisZ = glm::normalize(glm::toMat4(TransformComponent.GetQuaternion()) * BasisZ);

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
			LightComponent.CascadeData[i].ViewMat = CascadeView;

			NearPlane = FarPlane;
			FarPlane = LightComponent.ShadowCoverage * (0.0447f * static_cast<float>(pow(2.1867f, (i + 1))));
			LightComponent.CascadeData[i].Size = static_cast<float>(static_cast<int>(FarPlane) - 1) * TestCascadeDistanceScaleFactor;
			if (LightComponent.CascadeData[i].Size <= 0.01f)
				LightComponent.CascadeData[i].Size = 1.0;

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
				FrustumEdges[j] = LightComponent.CascadeData[0].ViewMat * InverseVm * FrustumEdges[j];

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

			LightComponent.CascadeData[i].ProjectionMat = glm::ortho(MinX - FarPlane * (LightComponent.CSMXYDepth / 4.0f), MaxX + FarPlane * (LightComponent.CSMXYDepth / 4.0f),
																	 MinY - FarPlane * (LightComponent.CSMXYDepth / 4.0f), MaxY + FarPlane * (LightComponent.CSMXYDepth / 4.0f),
																	 MinZ - FarPlane * LightComponent.CSMZDepth, MaxZ + FarPlane * LightComponent.CSMZDepth);
		}
	}
	else if (TestCSMType == 1)
	{
		for (size_t i = 0; i < 4; i++)
		{
			LightComponent.CascadeData[i].ViewMat = CascadeView;

			NearPlane = FarPlane;
			if (i == 0)
			{
				FarPlane = (LightComponent.ShadowCoverage / 4.0f) * TestCSM0;
			}
			else if (i == 1)
			{
				FarPlane = (LightComponent.ShadowCoverage / 4.0f) * TestCSM1;
			}
			else if (i == 2)
			{
				FarPlane = (LightComponent.ShadowCoverage / 4.0f) * TestCSM2;
			}
			else if (i == 3)
			{
				FarPlane = (LightComponent.ShadowCoverage / 4.0f) * TestCSM3;
			}
			LightComponent.CascadeData[i].Size = FarPlane * TestCSMScale;

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
				FrustumEdges[j] = LightComponent.CascadeData[0].ViewMat * InverseVm * FrustumEdges[j];

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

			float Left = MinX - FarPlane * (LightComponent.CSMXYDepth / 4.0f);
			float Right = MaxX + FarPlane * (LightComponent.CSMXYDepth / 4.0f);

			float Bottom = MinY - FarPlane * (LightComponent.CSMXYDepth / 4.0f);
			float Top = MaxY + FarPlane * (LightComponent.CSMXYDepth / 4.0f);

			float ZNear = MinZ - FarPlane * LightComponent.CSMZDepth;
			float ZFar = MaxZ + FarPlane * LightComponent.CSMZDepth;

			LightComponent.CascadeData[i].ProjectionMat = glm::ortho(Left, Right,
																	 Bottom, Top,
																	 ZNear, ZFar);

			float Clip[16];
			float T;

			glm::mat4 cliping = LightComponent.CascadeData[i].ProjectionMat * CascadeView;
			for (int j = 0; j < 4; j++)
			{
				Clip[j * 4] = cliping[j][0];
				Clip[j * 4 + 1] = cliping[j][1];
				Clip[j * 4 + 2] = cliping[j][2];
				Clip[j * 4 + 3] = cliping[j][3];
			}

			/* Extract the numbers for the RIGHT plane */
			LightComponent.CascadeData[i].Frustum[0][0] = Clip[3] - Clip[0];
			LightComponent.CascadeData[i].Frustum[0][1] = Clip[7] - Clip[4];
			LightComponent.CascadeData[i].Frustum[0][2] = Clip[11] - Clip[8];
			LightComponent.CascadeData[i].Frustum[0][3] = Clip[15] - Clip[12];

			/* Normalize the result */
			T = sqrt(LightComponent.CascadeData[i].Frustum[0][0] * LightComponent.CascadeData[i].Frustum[0][0] + LightComponent.CascadeData[i].Frustum[0][1] * LightComponent.CascadeData[i].Frustum[0][1] + LightComponent.CascadeData[i].Frustum[0][2] * LightComponent.CascadeData[i].Frustum[0][2]);
			LightComponent.CascadeData[i].Frustum[0][0] /= T;
			LightComponent.CascadeData[i].Frustum[0][1] /= T;
			LightComponent.CascadeData[i].Frustum[0][2] /= T;
			LightComponent.CascadeData[i].Frustum[0][3] /= T;

			/* Extract the numbers for the LEFT plane */
			LightComponent.CascadeData[i].Frustum[1][0] = Clip[3] + Clip[0];
			LightComponent.CascadeData[i].Frustum[1][1] = Clip[7] + Clip[4];
			LightComponent.CascadeData[i].Frustum[1][2] = Clip[11] + Clip[8];
			LightComponent.CascadeData[i].Frustum[1][3] = Clip[15] + Clip[12];

			/* Normalize the result */
			T = sqrt(LightComponent.CascadeData[i].Frustum[1][0] * LightComponent.CascadeData[i].Frustum[1][0] + LightComponent.CascadeData[i].Frustum[1][1] * LightComponent.CascadeData[i].Frustum[1][1] + LightComponent.CascadeData[i].Frustum[1][2] * LightComponent.CascadeData[i].Frustum[1][2]);
			LightComponent.CascadeData[i].Frustum[1][0] /= T;
			LightComponent.CascadeData[i].Frustum[1][1] /= T;
			LightComponent.CascadeData[i].Frustum[1][2] /= T;
			LightComponent.CascadeData[i].Frustum[1][3] /= T;

			/* Extract the BOTTOM plane */
			LightComponent.CascadeData[i].Frustum[2][0] = Clip[3] + Clip[1];
			LightComponent.CascadeData[i].Frustum[2][1] = Clip[7] + Clip[5];
			LightComponent.CascadeData[i].Frustum[2][2] = Clip[11] + Clip[9];
			LightComponent.CascadeData[i].Frustum[2][3] = Clip[15] + Clip[13];

			/* Normalize the result */
			T = sqrt(LightComponent.CascadeData[i].Frustum[2][0] * LightComponent.CascadeData[i].Frustum[2][0] + LightComponent.CascadeData[i].Frustum[2][1] * LightComponent.CascadeData[i].Frustum[2][1] + LightComponent.CascadeData[i].Frustum[2][2] * LightComponent.CascadeData[i].Frustum[2][2]);
			LightComponent.CascadeData[i].Frustum[2][0] /= T;
			LightComponent.CascadeData[i].Frustum[2][1] /= T;
			LightComponent.CascadeData[i].Frustum[2][2] /= T;
			LightComponent.CascadeData[i].Frustum[2][3] /= T;

			/* Extract the TOP plane */
			LightComponent.CascadeData[i].Frustum[3][0] = Clip[3] - Clip[1];
			LightComponent.CascadeData[i].Frustum[3][1] = Clip[7] - Clip[5];
			LightComponent.CascadeData[i].Frustum[3][2] = Clip[11] - Clip[9];
			LightComponent.CascadeData[i].Frustum[3][3] = Clip[15] - Clip[13];

			/* Normalize the result */
			T = sqrt(LightComponent.CascadeData[i].Frustum[3][0] * LightComponent.CascadeData[i].Frustum[3][0] + LightComponent.CascadeData[i].Frustum[3][1] * LightComponent.CascadeData[i].Frustum[3][1] + LightComponent.CascadeData[i].Frustum[3][2] * LightComponent.CascadeData[i].Frustum[3][2]);
			LightComponent.CascadeData[i].Frustum[3][0] /= T;
			LightComponent.CascadeData[i].Frustum[3][1] /= T;
			LightComponent.CascadeData[i].Frustum[3][2] /= T;
			LightComponent.CascadeData[i].Frustum[3][3] /= T;

			/* Extract the FAR plane */
			LightComponent.CascadeData[i].Frustum[4][0] = Clip[3] - Clip[2];
			LightComponent.CascadeData[i].Frustum[4][1] = Clip[7] - Clip[6];
			LightComponent.CascadeData[i].Frustum[4][2] = Clip[11] - Clip[10];
			LightComponent.CascadeData[i].Frustum[4][3] = Clip[15] - Clip[14];

			/* Normalize the result */
			T = sqrt(LightComponent.CascadeData[i].Frustum[4][0] * LightComponent.CascadeData[i].Frustum[4][0] + LightComponent.CascadeData[i].Frustum[4][1] * LightComponent.CascadeData[i].Frustum[4][1] + LightComponent.CascadeData[i].Frustum[4][2] * LightComponent.CascadeData[i].Frustum[4][2]);
			LightComponent.CascadeData[i].Frustum[4][0] /= T;
			LightComponent.CascadeData[i].Frustum[4][1] /= T;
			LightComponent.CascadeData[i].Frustum[4][2] /= T;
			LightComponent.CascadeData[i].Frustum[4][3] /= T;

			/* Extract the NEAR plane */
			LightComponent.CascadeData[i].Frustum[5][0] = Clip[3] + Clip[2];
			LightComponent.CascadeData[i].Frustum[5][1] = Clip[7] + Clip[6];
			LightComponent.CascadeData[i].Frustum[5][2] = Clip[11] + Clip[10];
			LightComponent.CascadeData[i].Frustum[5][3] = Clip[15] + Clip[14];

			/* Normalize the result */
			T = sqrt(LightComponent.CascadeData[i].Frustum[5][0] * LightComponent.CascadeData[i].Frustum[5][0] + LightComponent.CascadeData[i].Frustum[5][1] * LightComponent.CascadeData[i].Frustum[5][1] + LightComponent.CascadeData[i].Frustum[5][2] * LightComponent.CascadeData[i].Frustum[5][2]);
			LightComponent.CascadeData[i].Frustum[5][0] /= T;
			LightComponent.CascadeData[i].Frustum[5][1] /= T;
			LightComponent.CascadeData[i].Frustum[5][2] /= T;
			LightComponent.CascadeData[i].Frustum[5][3] /= T;
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
			LightComponent.CascadeData[i].ViewMat = CascadeView;

			NearPlane = FarPlane;
			if (i == 0)
			{
				FarPlane = (LightComponent.ShadowCoverage / 4.0f) * TestCSM0;
			}
			else if (i == 1)
			{
				FarPlane = (LightComponent.ShadowCoverage / 4.0f) * TestCSM1;
			}
			else if (i == 2)
			{
				FarPlane = (LightComponent.ShadowCoverage / 4.0f) * TestCSM2;
			}
			else if (i == 3)
			{
				FarPlane = (LightComponent.ShadowCoverage / 4.0f) * TestCSM3;
			}
			LightComponent.CascadeData[i].Size = FarPlane * TestCSMScale;

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
				FrustumEdges[j] = LightComponent.CascadeData[0].ViewMat * InverseVm * FrustumEdges[j];

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

			LightComponent.CascadeData[i].ProjectionMat = glm::ortho(MinX - FarPlane * (LightComponent.CSMXYDepth / 4.0f), MaxX + FarPlane * (LightComponent.CSMXYDepth / 4.0f),
																	 MinY - FarPlane * (LightComponent.CSMXYDepth / 4.0f), MaxY + FarPlane * (LightComponent.CSMXYDepth / 4.0f),
																	 MinZ - FarPlane * LightComponent.CSMZDepth, MaxZ + FarPlane * LightComponent.CSMZDepth);
		}
	}
}