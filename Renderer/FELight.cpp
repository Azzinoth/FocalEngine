#include "FELight.h"
using namespace FocalEngine;

FELight::FELight(FEObjectType lightType) : FEObject(lightType, "")
{
}

FELight::~FELight()
{
}

glm::vec3 FELight::getColor()
{
	return color;
}

void FELight::setColor(glm::vec3 newColor)
{
	color = newColor;
}

bool FELight::isLightEnabled()
{
	return enabled;
}

void FELight::setLightEnabled(bool isLightEnabled)
{
	enabled = isLightEnabled;
}

float FELight::getShadowBias()
{
	return shadowBias;
}

void FELight::setShadowBias(float newShadowBias)
{
	shadowBias = newShadowBias;
}

float FELight::getIntensity()
{
	return intensity;
}

void FELight::setIntensity(float newIntensity)
{
	intensity = newIntensity;
}

bool FELight::isCastShadows()
{
	return castShadows;
}

void FELight::setCastShadows(bool isCastShadows)
{
	castShadows = isCastShadows;
}

FECascadeData::FECascadeData()
{
	frustum = new float* [6];
	for (size_t i = 0; i < 6; i++)
	{
		frustum[i] = new float[4];
	}
}

FECascadeData::~FECascadeData()
{
	for (size_t i = 0; i < 6; i++)
	{
		delete[] frustum[i];
	}
	delete[] frustum;
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


void FEDirectionalLight::updateCascades(float cameraFov, float aspectRatio, float nearPlane, float farPlane, glm::mat4 viewMatrix, glm::vec3 cameraForward, glm::vec3 cameraRight, glm::vec3 cameraUp)
{
	static glm::vec4 basisX = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	static glm::vec4 basisY = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	static glm::vec4 basisZ = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	glm::vec4 fbasisX = glm::normalize(glm::toMat4(transform.getQuaternion()) * basisX);
	glm::vec4 fbasisY = glm::normalize(glm::toMat4(transform.getQuaternion()) * basisY);
	glm::vec4 fbasisZ = glm::normalize(glm::toMat4(transform.getQuaternion()) * basisZ);

	glm::mat4 cascadeView = glm::mat4(1.0f);
	cascadeView[0][0] = fbasisX.x;
	cascadeView[1][0] = fbasisX.y;
	cascadeView[2][0] = fbasisX.z;
	cascadeView[0][1] = fbasisY.x;
	cascadeView[1][1] = fbasisY.y;
	cascadeView[2][1] = fbasisY.z;
	cascadeView[0][2] = fbasisZ.x;
	cascadeView[1][2] = fbasisZ.y;
	cascadeView[2][2] = fbasisZ.z;

	farPlane = nearPlane;
	glm::mat4 inverseVM = glm::inverse(viewMatrix);
	static std::vector<glm::vec4> frustumEdges;
	frustumEdges.resize(8);

	float TEST_CASCADE_DISTANCE_SCALE_FACTOR = 4.0f;
	float testCSM0 = 0.050f;
	float testCSM1 = 0.150f;
	float testCSM2 = 0.550f;
	float testCSM3 = 1.1f;
	float testCSMScale = 1.75f;

	float testCSMType = 1;

	// old and incorrect
	if (testCSMType == 0)
	{
		for (size_t i = 0; i < 4; i++)
		{
			cascadeData[i].viewMat = cascadeView;

			nearPlane = farPlane;
			farPlane = shadowCoverage * (0.0447f * float(pow(2.1867f, (i + 1))));
			cascadeData[i].size = float(int(farPlane) - 1) * TEST_CASCADE_DISTANCE_SCALE_FACTOR;
			if (cascadeData[i].size <= 0.01f)
				cascadeData[i].size = 1.0;

			float firstCascadeY1 = nearPlane * tan(glm::radians(cameraFov / 2.0f));
			float firstCascadeY2 = farPlane * tan(glm::radians(cameraFov / 2.0f));

			float firstCascadeX1 = nearPlane * tan((aspectRatio) / 2.0f);
			float firstCascadeX2 = farPlane * tan((aspectRatio) / 2.0f);

			frustumEdges[0] = glm::vec4(firstCascadeX1, -firstCascadeY1, -nearPlane, 1.0f);
			frustumEdges[1] = glm::vec4(firstCascadeX1, firstCascadeY1, -nearPlane, 1.0f);
			frustumEdges[2] = glm::vec4(-firstCascadeX1, firstCascadeY1, -nearPlane, 1.0f);
			frustumEdges[3] = glm::vec4(-firstCascadeX1, -firstCascadeY1, -nearPlane, 1.0f);

			frustumEdges[4] = glm::vec4(firstCascadeX2, -firstCascadeY2, -farPlane, 1.0f);
			frustumEdges[5] = glm::vec4(firstCascadeX2, firstCascadeY2, -farPlane, 1.0f);
			frustumEdges[6] = glm::vec4(-firstCascadeX2, firstCascadeY2, -farPlane, 1.0f);
			frustumEdges[7] = glm::vec4(-firstCascadeX2, -firstCascadeY2, -farPlane, 1.0f);

			for (size_t j = 0; j < frustumEdges.size(); j++)
				frustumEdges[j] = cascadeData[0].viewMat * inverseVM * frustumEdges[j];

			for (size_t j = 0; j < frustumEdges.size(); j++)
				frustumEdges[j].z = -frustumEdges[j].z;

			float minX = FLT_MAX;
			float maxX = FLT_MIN;
			float minY = FLT_MAX;
			float maxY = FLT_MIN;
			float minZ = FLT_MAX;
			float maxZ = FLT_MIN;

			for (size_t j = 0; j < frustumEdges.size(); j++)
			{
				minX = std::min(minX, frustumEdges[j].x);
				minY = std::min(minY, frustumEdges[j].y);
				minZ = std::min(minZ, frustumEdges[j].z);

				maxX = std::max(maxX, frustumEdges[j].x);
				maxY = std::max(maxY, frustumEdges[j].y);
				maxZ = std::max(maxZ, frustumEdges[j].z);
			}

			cascadeData[i].projectionMat = glm::ortho(minX - farPlane * (CSMXYDepth / 4.0f), maxX + farPlane * (CSMXYDepth / 4.0f),
				minY - farPlane * (CSMXYDepth / 4.0f), maxY + farPlane * (CSMXYDepth / 4.0f),
				minZ - farPlane * CSMZDepth, maxZ + farPlane * CSMZDepth);
		}
	}
	else if (testCSMType == 1)
	{
		for (size_t i = 0; i < 4; i++)
		{
			cascadeData[i].viewMat = cascadeView;

			nearPlane = farPlane;
			if (i == 0)
			{
				farPlane = (shadowCoverage / 4.0f) * testCSM0;
			}
			else if (i == 1)
			{
				farPlane = (shadowCoverage / 4.0f) * testCSM1;
			}
			else if (i == 2)
			{
				farPlane = (shadowCoverage / 4.0f) * testCSM2;
			}
			else if (i == 3)
			{
				farPlane = (shadowCoverage / 4.0f) * testCSM3;
			}
			cascadeData[i].size = farPlane * testCSMScale;

			float firstCascadeY1 = nearPlane * tan(glm::radians(cameraFov / 2.0f));
			float firstCascadeY2 = farPlane * tan(glm::radians(cameraFov / 2.0f));

			float firstCascadeX1 = nearPlane * tan((aspectRatio) / 2.0f);
			float firstCascadeX2 = farPlane * tan((aspectRatio) / 2.0f);

			frustumEdges[0] = glm::vec4(firstCascadeX1, -firstCascadeY1, -nearPlane, 1.0f);
			frustumEdges[1] = glm::vec4(firstCascadeX1, firstCascadeY1, -nearPlane, 1.0f);
			frustumEdges[2] = glm::vec4(-firstCascadeX1, firstCascadeY1, -nearPlane, 1.0f);
			frustumEdges[3] = glm::vec4(-firstCascadeX1, -firstCascadeY1, -nearPlane, 1.0f);

			frustumEdges[4] = glm::vec4(firstCascadeX2, -firstCascadeY2, -farPlane, 1.0f);
			frustumEdges[5] = glm::vec4(firstCascadeX2, firstCascadeY2, -farPlane, 1.0f);
			frustumEdges[6] = glm::vec4(-firstCascadeX2, firstCascadeY2, -farPlane, 1.0f);
			frustumEdges[7] = glm::vec4(-firstCascadeX2, -firstCascadeY2, -farPlane, 1.0f);

			for (size_t j = 0; j < frustumEdges.size(); j++)
				frustumEdges[j] = cascadeData[0].viewMat * inverseVM * frustumEdges[j];

			for (size_t j = 0; j < frustumEdges.size(); j++)
				frustumEdges[j].z = -frustumEdges[j].z;

			float minX = FLT_MAX;
			float maxX = -FLT_MAX;
			float minY = FLT_MAX;
			float maxY = -FLT_MAX;
			float minZ = FLT_MAX;
			float maxZ = -FLT_MAX;

			for (size_t j = 0; j < frustumEdges.size(); j++)
			{
				minX = std::min(minX, frustumEdges[j].x);
				minY = std::min(minY, frustumEdges[j].y);
				minZ = std::min(minZ, frustumEdges[j].z);

				maxX = std::max(maxX, frustumEdges[j].x);
				maxY = std::max(maxY, frustumEdges[j].y);
				maxZ = std::max(maxZ, frustumEdges[j].z);
			}

			float left = minX - farPlane * (CSMXYDepth / 4.0f);
			float right = maxX + farPlane * (CSMXYDepth / 4.0f);

			float bottom = minY - farPlane * (CSMXYDepth / 4.0f);
			float top = maxY + farPlane * (CSMXYDepth / 4.0f);

			float zNear = minZ - farPlane * CSMZDepth;
			float zFar = maxZ + farPlane * CSMZDepth;

			cascadeData[i].projectionMat = glm::ortho(left, right,
													  bottom, top,
													  zNear, zFar);

			float   clip[16];
			float   t;

			glm::mat4 cliping = cascadeData[i].projectionMat * cascadeView;
			for (size_t j = 0; j < 4; j++)
			{
				clip[j * 4] = cliping[j][0];
				clip[j * 4 + 1] = cliping[j][1];
				clip[j * 4 + 2] = cliping[j][2];
				clip[j * 4 + 3] = cliping[j][3];
			}

			/* Extract the numbers for the RIGHT plane */
			cascadeData[i].frustum[0][0] = clip[3] - clip[0];
			cascadeData[i].frustum[0][1] = clip[7] - clip[4];
			cascadeData[i].frustum[0][2] = clip[11] - clip[8];
			cascadeData[i].frustum[0][3] = clip[15] - clip[12];

			/* Normalize the result */
			t = sqrt(cascadeData[i].frustum[0][0] * cascadeData[i].frustum[0][0] + cascadeData[i].frustum[0][1] * cascadeData[i].frustum[0][1] + cascadeData[i].frustum[0][2] * cascadeData[i].frustum[0][2]);
			cascadeData[i].frustum[0][0] /= t;
			cascadeData[i].frustum[0][1] /= t;
			cascadeData[i].frustum[0][2] /= t;
			cascadeData[i].frustum[0][3] /= t;

			/* Extract the numbers for the LEFT plane */
			cascadeData[i].frustum[1][0] = clip[3] + clip[0];
			cascadeData[i].frustum[1][1] = clip[7] + clip[4];
			cascadeData[i].frustum[1][2] = clip[11] + clip[8];
			cascadeData[i].frustum[1][3] = clip[15] + clip[12];

			/* Normalize the result */
			t = sqrt(cascadeData[i].frustum[1][0] * cascadeData[i].frustum[1][0] + cascadeData[i].frustum[1][1] * cascadeData[i].frustum[1][1] + cascadeData[i].frustum[1][2] * cascadeData[i].frustum[1][2]);
			cascadeData[i].frustum[1][0] /= t;
			cascadeData[i].frustum[1][1] /= t;
			cascadeData[i].frustum[1][2] /= t;
			cascadeData[i].frustum[1][3] /= t;

			/* Extract the BOTTOM plane */
			cascadeData[i].frustum[2][0] = clip[3] + clip[1];
			cascadeData[i].frustum[2][1] = clip[7] + clip[5];
			cascadeData[i].frustum[2][2] = clip[11] + clip[9];
			cascadeData[i].frustum[2][3] = clip[15] + clip[13];

			/* Normalize the result */
			t = sqrt(cascadeData[i].frustum[2][0] * cascadeData[i].frustum[2][0] + cascadeData[i].frustum[2][1] * cascadeData[i].frustum[2][1] + cascadeData[i].frustum[2][2] * cascadeData[i].frustum[2][2]);
			cascadeData[i].frustum[2][0] /= t;
			cascadeData[i].frustum[2][1] /= t;
			cascadeData[i].frustum[2][2] /= t;
			cascadeData[i].frustum[2][3] /= t;

			/* Extract the TOP plane */
			cascadeData[i].frustum[3][0] = clip[3] - clip[1];
			cascadeData[i].frustum[3][1] = clip[7] - clip[5];
			cascadeData[i].frustum[3][2] = clip[11] - clip[9];
			cascadeData[i].frustum[3][3] = clip[15] - clip[13];

			/* Normalize the result */
			t = sqrt(cascadeData[i].frustum[3][0] * cascadeData[i].frustum[3][0] + cascadeData[i].frustum[3][1] * cascadeData[i].frustum[3][1] + cascadeData[i].frustum[3][2] * cascadeData[i].frustum[3][2]);
			cascadeData[i].frustum[3][0] /= t;
			cascadeData[i].frustum[3][1] /= t;
			cascadeData[i].frustum[3][2] /= t;
			cascadeData[i].frustum[3][3] /= t;

			/* Extract the FAR plane */
			cascadeData[i].frustum[4][0] = clip[3] - clip[2];
			cascadeData[i].frustum[4][1] = clip[7] - clip[6];
			cascadeData[i].frustum[4][2] = clip[11] - clip[10];
			cascadeData[i].frustum[4][3] = clip[15] - clip[14];

			/* Normalize the result */
			t = sqrt(cascadeData[i].frustum[4][0] * cascadeData[i].frustum[4][0] + cascadeData[i].frustum[4][1] * cascadeData[i].frustum[4][1] + cascadeData[i].frustum[4][2] * cascadeData[i].frustum[4][2]);
			cascadeData[i].frustum[4][0] /= t;
			cascadeData[i].frustum[4][1] /= t;
			cascadeData[i].frustum[4][2] /= t;
			cascadeData[i].frustum[4][3] /= t;

			/* Extract the NEAR plane */
			cascadeData[i].frustum[5][0] = clip[3] + clip[2];
			cascadeData[i].frustum[5][1] = clip[7] + clip[6];
			cascadeData[i].frustum[5][2] = clip[11] + clip[10];
			cascadeData[i].frustum[5][3] = clip[15] + clip[14];

			/* Normalize the result */
			t = sqrt(cascadeData[i].frustum[5][0] * cascadeData[i].frustum[5][0] + cascadeData[i].frustum[5][1] * cascadeData[i].frustum[5][1] + cascadeData[i].frustum[5][2] * cascadeData[i].frustum[5][2]);
			cascadeData[i].frustum[5][0] /= t;
			cascadeData[i].frustum[5][1] /= t;
			cascadeData[i].frustum[5][2] /= t;
			cascadeData[i].frustum[5][3] /= t;

			int ty = 0;
			ty++;
		}
	}
	// my way of frustum reconstruction
	else if (testCSMType == 2)
	{
		float alphaDivTwo = cameraFov / 2.0f;
		float nearPlaneXLength = nearPlane * glm::tan(aspectRatio / 2.0f) * 2.0f;
		float nearPlaneYLength = nearPlaneXLength / aspectRatio;

		glm::vec3 cameraPos = glm::vec3(viewMatrix[3][0], viewMatrix[3][1], viewMatrix[3][2]);

		glm::vec3 nearTopLeft = cameraPos;
		nearTopLeft += -cameraRight * (nearPlaneXLength / 2.0f);
		nearTopLeft += cameraUp * (nearPlaneYLength / 2.0f);
		nearTopLeft += cameraForward * nearPlane;

		glm::vec3 nearTopRight = cameraPos;
		nearTopRight += cameraRight * (nearPlaneXLength / 2.0f);
		nearTopRight += cameraUp * (nearPlaneYLength / 2.0f);
		nearTopRight += cameraForward * nearPlane;

		glm::vec3 nearBottomLeft = cameraPos;
		nearBottomLeft += -cameraRight * (nearPlaneXLength / 2.0f);
		nearBottomLeft += -cameraUp * (nearPlaneYLength / 2.0f);
		nearBottomLeft += cameraForward * nearPlane;

		glm::vec3 nearBottomRight = cameraPos;
		nearBottomRight += cameraRight * (nearPlaneXLength / 2.0f);
		nearBottomRight += -cameraUp * (nearPlaneYLength / 2.0f);
		nearBottomRight += cameraForward * nearPlane;

		glm::vec3 cameraToTopLeft = glm::normalize(nearTopLeft - cameraPos);
		glm::vec3 cameraToTopRight = glm::normalize(nearTopRight - cameraPos);
		glm::vec3 cameraToBottomLeft = glm::normalize(nearBottomLeft - cameraPos);
		glm::vec3 cameraToBottomRight = glm::normalize(nearBottomRight - cameraPos);

		for (size_t i = 0; i < 4; i++)
		{
			cascadeData[i].viewMat = cascadeView;

			nearPlane = farPlane;
			if (i == 0)
			{
				farPlane = (shadowCoverage / 4.0f) * testCSM0;
			}
			else if (i == 1)
			{
				farPlane = (shadowCoverage / 4.0f) * testCSM1;
			}
			else if (i == 2)
			{
				farPlane = (shadowCoverage / 4.0f) * testCSM2;
			}
			else if (i == 3)
			{
				farPlane = (shadowCoverage / 4.0f) * testCSM3;
			}
			cascadeData[i].size = farPlane * testCSMScale;

			cameraPos = glm::vec3(0.0f);

			frustumEdges[0] = glm::vec4(cameraPos + cameraToTopLeft * nearPlane, 1.0f);
			frustumEdges[1] = glm::vec4(cameraPos + cameraToTopRight * nearPlane, 1.0f);
			frustumEdges[2] = glm::vec4(cameraPos + cameraToBottomLeft * nearPlane, 1.0f);
			frustumEdges[3] = glm::vec4(cameraPos + cameraToBottomRight * nearPlane, 1.0f);

			frustumEdges[4] = glm::vec4(cameraPos + cameraToTopLeft * farPlane, 1.0f);
			frustumEdges[5] = glm::vec4(cameraPos + cameraToTopRight * farPlane, 1.0f);
			frustumEdges[6] = glm::vec4(cameraPos + cameraToBottomLeft * farPlane, 1.0f);
			frustumEdges[7] = glm::vec4(cameraPos + cameraToBottomRight * farPlane, 1.0f);

			for (size_t j = 0; j < frustumEdges.size(); j++)
				frustumEdges[j] = cascadeData[0].viewMat * inverseVM * frustumEdges[j];

			for (size_t j = 0; j < frustumEdges.size(); j++)
				frustumEdges[j].z = -frustumEdges[j].z;

			float minX = FLT_MAX;
			float maxX = -FLT_MAX;
			float minY = FLT_MAX;
			float maxY = -FLT_MAX;
			float minZ = FLT_MAX;
			float maxZ = -FLT_MAX;

			for (size_t j = 0; j < frustumEdges.size(); j++)
			{
				minX = std::min(minX, frustumEdges[j].x);
				minY = std::min(minY, frustumEdges[j].y);
				minZ = std::min(minZ, frustumEdges[j].z);

				maxX = std::max(maxX, frustumEdges[j].x);
				maxY = std::max(maxY, frustumEdges[j].y);
				maxZ = std::max(maxZ, frustumEdges[j].z);
			}

			cascadeData[i].projectionMat = glm::ortho(minX - farPlane * (CSMXYDepth / 4.0f), maxX + farPlane * (CSMXYDepth / 4.0f),
				minY - farPlane * (CSMXYDepth / 4.0f), maxY + farPlane * (CSMXYDepth / 4.0f),
				minZ - farPlane * CSMZDepth, maxZ + farPlane * CSMZDepth);
		}
	}
}

FEDirectionalLight::FEDirectionalLight() : FELight(FE_DIRECTIONAL_LIGHT)
{
}

glm::vec3 FEDirectionalLight::getDirection()
{
	direction = glm::normalize(transform.getTransformMatrix() * glm::vec4(defaultDirection, 0.0f));
	return direction;
}

void FEDirectionalLight::setDirection(glm::vec3 newDirection)
{
	direction = newDirection;
}

FEDirectionalLight::~FEDirectionalLight()
{
	delete cascadeData[0].frameBuffer;
	delete cascadeData[1].frameBuffer;
	delete cascadeData[2].frameBuffer;
	delete cascadeData[3].frameBuffer;
}

int FEDirectionalLight::getActiveCascades()
{
	return activeCascades;
}

void FEDirectionalLight::setActiveCascades(int newActiveCascades)
{
	if (newActiveCascades < 1 || newActiveCascades > 4)
		newActiveCascades = 1;

	activeCascades = newActiveCascades;
}

float FEDirectionalLight::getShadowCoverage()
{
	return shadowCoverage;
}

void FEDirectionalLight::setShadowCoverage(float newShadowCoverage)
{
	if (newShadowCoverage <= 0.0f)
		newShadowCoverage = 0.1f;

	shadowCoverage = newShadowCoverage;
	//updateProjectionMat();
}

float FEDirectionalLight::getCSMZDepth()
{
	return CSMZDepth;
}

void FEDirectionalLight::setCSMZDepth(float newCSMZDepth)
{
	if (newCSMZDepth <= 0.5f)
		newCSMZDepth = 0.5f;

	CSMZDepth = newCSMZDepth;
}

float FEDirectionalLight::getCSMXYDepth()
{
	return CSMXYDepth;
}

void FEDirectionalLight::setCSMXYDepth(float newCSMXYDepth)
{
	if (newCSMXYDepth <= 0.5f)
		newCSMXYDepth = 0.5f;

	CSMXYDepth = newCSMXYDepth;
}

FESpotLight::FESpotLight() : FELight(FE_SPOT_LIGHT)
{
}

FESpotLight::~FESpotLight()
{
}

float FESpotLight::getSpotAngle()
{
	return spotAngle;
}

void FESpotLight::setSpotAngle(float newSpotAngle)
{
	spotAngle = newSpotAngle;
}

float FESpotLight::getSpotAngleOuter()
{
	return spotAngleOuter;
}

void FESpotLight::setSpotAngleOuter(float newSpotAngleOuter)
{
	spotAngleOuter = newSpotAngleOuter;
}

float FESpotLight::getRange()
{
	return range;
}

void FESpotLight::setRange(float newRange)
{
	range = newRange;
}

glm::vec3 FESpotLight::getDirection()
{
	direction = glm::normalize(transform.getTransformMatrix() * glm::vec4(defaultDirection, 0.0f));
	return direction;
}

void FESpotLight::setDirection(glm::vec3 newDirection)
{
	direction = newDirection;
}

FEPointLight::FEPointLight() : FELight(FE_POINT_LIGHT)
{
}

FEPointLight::~FEPointLight()
{
}

float FEPointLight::getRange()
{
	return range;
}

void FEPointLight::setRange(float newRange)
{
	range = newRange;
}

bool FELight::isStaticShadowBias()
{
	return staticShadowBias;
}

void FELight::setIsStaticShadowBias(bool isStaticShadowBias)
{
	staticShadowBias = isStaticShadowBias;
}

float FELight::getShadowBiasVariableIntensity()
{
	return shadowBiasVariableIntensity;
}

void FELight::setShadowBiasVariableIntensity(float newShadowBiasVariableIntensity)
{
	if (newShadowBiasVariableIntensity <= 0.0f)
		newShadowBiasVariableIntensity = 0.01f;

	shadowBiasVariableIntensity = newShadowBiasVariableIntensity;
}