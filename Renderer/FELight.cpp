#include "FELight.h"
using namespace FocalEngine;

FELight::FELight()
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

bool FELight::isLightProjectingShadow()
{
	return projectShadow;
}

void FELight::setLightProjectingShadow(bool isLightEnabled)
{
	projectShadow = isLightEnabled;
}

float FELight::getShadowBias()
{
	return shadowBias;
}

void FELight::setShadowBias(float newShadowBias)
{
	shadowBias = newShadowBias;
}