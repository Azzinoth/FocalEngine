#include "FELight.h"
using namespace FocalEngine;

FELight::FELight(FELightType Type)
{
	type = Type;
	//if (type == FE_DIRECTIONAL_LIGHT)
	//{
		shadowMapProjectionSize = 50.0f;
		shadowProjectionMatrix = glm::ortho(-shadowMapProjectionSize,
											shadowMapProjectionSize,
											-shadowMapProjectionSize,
											shadowMapProjectionSize, 0.01f, 700.0f);
	//}
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

float FELight::getRange()
{
	return range;
}

void FELight::setRange(float newRange)
{
	range = newRange;
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

glm::vec3 FELight::getDirection()
{
	direction = glm::normalize(transform.getTransformMatrix() * glm::vec4(defaultDirection, 0.0f));
	return direction;
}

void FELight::setDirection(glm::vec3 newDirection)
{
	direction = newDirection;
}

float FELight::getSpotAngle()
{
	return spotAngle;
}

void FELight::setSpotAngle(float newSpotAngle)
{
	spotAngle = newSpotAngle;
}

float FELight::getSpotAngleOuter()
{
	return spotAngleOuter;
}

void FELight::setSpotAngleOuter(float newSpotAngleOuter)
{
	spotAngleOuter = newSpotAngleOuter;
}

std::string FELight::getName()
{
	return name;
}

void FELight::setName(std::string newName)
{
	name = newName;
}

glm::mat4 FELight::getViewMatrixForShadowMap()
{
	static glm::vec4 basisX = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	static glm::vec4 basisY = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	static glm::vec4 basisZ = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

	glm::vec4 fbasisX = glm::normalize(transform.getTransformMatrix() * basisX);
	glm::vec4 fbasisY = glm::normalize(transform.getTransformMatrix() * basisY);
	glm::vec4 fbasisZ = glm::normalize(transform.getTransformMatrix() * basisZ);

	glm::mat4 testView = glm::mat4(1.0f);
	testView[0][0] = fbasisX.x;
	testView[1][0] = fbasisX.y;
	testView[2][0] = fbasisX.z;
	testView[0][1] = fbasisY.x;
	testView[1][1] = fbasisY.y;
	testView[2][1] = fbasisY.z;
	testView[0][2] = fbasisZ.x;
	testView[1][2] = fbasisZ.y;
	testView[2][2] = fbasisZ.z;
	testView[3][0] = transform.getPosition()[0];
	testView[3][1] = transform.getPosition()[1];
	//to-do: fix magic number
	testView[3][2] = -50.0f/*itLight->second->transform.getPosition()[2]*/;

	return testView;
}