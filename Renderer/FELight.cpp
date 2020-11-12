#include "FELight.h"
using namespace FocalEngine;

FELight::FELight(FELightType Type)
{
	type = Type;
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

FELightType FELight::getType()
{
	return type;
}

std::string FELight::getName()
{
	return name;
}

void FELight::setName(std::string newName)
{
	name = newName;
}

FECascadeData::FECascadeData()
{

}

FECascadeData::~FECascadeData()
{

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

			cascadeData[i].projectionMat = glm::ortho(minX - farPlane * (CSMXYDepth / 4.0f), maxX + farPlane * (CSMXYDepth / 4.0f),
				minY - farPlane * (CSMXYDepth / 4.0f), maxY + farPlane * (CSMXYDepth / 4.0f),
				minZ - farPlane * CSMZDepth, maxZ + farPlane * CSMZDepth);
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