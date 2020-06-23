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

void FEDirectionalLight::updateCascades(glm::vec3 cameraPosition, glm::vec3 cameraDirection)
{
	static glm::vec4 basisX = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	static glm::vec4 basisY = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	static glm::vec4 basisZ = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	glm::vec4 fbasisX = glm::normalize(transform.getTransformMatrix() * basisX);
	glm::vec4 fbasisY = glm::normalize(transform.getTransformMatrix() * basisY);
	glm::vec4 fbasisZ = glm::normalize(transform.getTransformMatrix() * basisZ);

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

	for (size_t i = 0; i < 4; i++)
	{
		cameraPosition += cameraDirection * firstCascadeSize;

		glm::vec3 newLightPosition = cameraPosition + (-getDirection() * cascadeData[i].size * 2.0f);
		transform.setPosition(newLightPosition);

		cascadeData[i].viewMat = cascadeView;
		cascadeData[i].viewMat = glm::translate(cascadeData[i].viewMat, -transform.getPosition());
	}
}

FEDirectionalLight::FEDirectionalLight() : FELight(FE_DIRECTIONAL_LIGHT)
{
	updateProjectionMat();
}

void FEDirectionalLight::updateProjectionMat()
{
	cascadeData[0].size = firstCascadeSize;
	cascadeData[0].projectionMat = glm::ortho(-cascadeData[0].size,
											  cascadeData[0].size,
											  -cascadeData[0].size,
											  cascadeData[0].size, 0.1f, cascadeData[0].size * 10.0f);

	cascadeData[1].size = cascadeData[0].size * cascadeDistributionExponent;
	cascadeData[1].projectionMat = glm::ortho(-cascadeData[1].size,
											  cascadeData[1].size,
											  -cascadeData[1].size,
											  cascadeData[1].size, 0.1f, cascadeData[1].size * 10.0f);

	cascadeData[2].size = cascadeData[1].size * cascadeDistributionExponent;
	cascadeData[2].projectionMat = glm::ortho(-cascadeData[2].size,
											  cascadeData[2].size,
											  -cascadeData[2].size,
											  cascadeData[2].size, 0.1f, cascadeData[2].size * 10.0f);

	cascadeData[3].size = cascadeData[2].size * cascadeDistributionExponent;
	cascadeData[3].projectionMat = glm::ortho(-cascadeData[3].size,
											  cascadeData[3].size,
											  -cascadeData[3].size,
											  cascadeData[3].size, 0.1f, cascadeData[3].size * 10.0f);
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

float FEDirectionalLight::getCascadeDistributionExponent()
{
	return cascadeDistributionExponent;
}

void FEDirectionalLight::setCascadeDistributionExponent(float newCascadeDistributionExponent)
{
	if (newCascadeDistributionExponent <= 1.0f)
		newCascadeDistributionExponent = 1.1f;

	cascadeDistributionExponent = newCascadeDistributionExponent;
	updateProjectionMat();
}

float FEDirectionalLight::getFirstCascadeSize()
{
	return firstCascadeSize;
}

void FEDirectionalLight::setFirstCascadeSize(float newFirstCascadeSize)
{
	if (newFirstCascadeSize <= 0.0f)
		newFirstCascadeSize = 0.1f;

	firstCascadeSize = newFirstCascadeSize;
	updateProjectionMat();
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