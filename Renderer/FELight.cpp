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

glm::vec3 FELight::getPosition()
{
	return position;
}

void FELight::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
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

glm::vec3 FELight::getRotation()
{
	return rotation;
}

void FELight::setRotation(glm::vec3 newRotation)
{
	rotation = newRotation;

	glm::mat4 rotationMatrix = glm::mat4(1.0);
	rotationMatrix = glm::rotate(rotationMatrix, (float)rotation.y * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
	rotationMatrix = glm::rotate(rotationMatrix, (float)rotation.z * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
	rotationMatrix = glm::rotate(rotationMatrix, (float)rotation.x * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));
	
	direction = glm::normalize(rotationMatrix * glm::vec4(defaultDirection, 1.0f));
}

std::string FELight::getName()
{
	return name;
}

void FELight::setName(std::string newName)
{
	name = newName;
}