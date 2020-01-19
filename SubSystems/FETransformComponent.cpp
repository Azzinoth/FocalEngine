#include "FETransformComponent.h"
using namespace FocalEngine;

FETransformComponent::FETransformComponent()
{
	position = glm::vec3(0.0f);
	rotationAngles = glm::vec3(0.0f);
	rotationQuaternion = glm::quat(1.0f, glm::vec3(0.0f));
	scale = glm::vec3(1.0f, 1.0f, 1.0f);

	update();
}

FETransformComponent::~FETransformComponent()
{
}

glm::vec3 FETransformComponent::getPosition()
{
	return position;
}

glm::vec3 FETransformComponent::getRotation()
{
	return rotationAngles;
}

glm::vec3 FETransformComponent::getScale()
{
	return scale;
}

void FETransformComponent::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	update();
}

void FETransformComponent::setRotation(glm::vec3 newRotation)
{
	if (rotationAngles == newRotation)
		return;

	float RotationAngle = 0.0f;
	glm::vec3 RotationAxis = glm::vec3(0.0f);

	if (rotationAngles.x != newRotation.x)
	{
		RotationAngle = (float)(rotationAngles.x - newRotation.x) * ANGLE_TORADIANS_COF;
		RotationAxis = glm::vec3(1, 0, 0);
	}
	else if (rotationAngles.y != newRotation.y)
	{
		RotationAngle = (float)(rotationAngles.y - newRotation.y) * ANGLE_TORADIANS_COF;
		RotationAxis = glm::vec3(0, 1, 0);
	}
	else if (rotationAngles.z != newRotation.z)
	{
		RotationAngle = (float)(rotationAngles.z - newRotation.z) * ANGLE_TORADIANS_COF;
		RotationAxis = glm::vec3(0, 0, 1);
	}

	rotationQuaternion = glm::quat(cos(RotationAngle / 2),
								   RotationAxis.x * sin(RotationAngle / 2),
								   RotationAxis.y * sin(RotationAngle / 2),
								   RotationAxis.z * sin(RotationAngle / 2)) * rotationQuaternion;

	rotationAngles = newRotation;
	update();
}

void FETransformComponent::changeScaleUniformlyBy(float delta)
{
	scale += delta;
}

void FETransformComponent::changeXScaleBy(float delta)
{
	if (uniformScaling)
	{
		changeScaleUniformlyBy(delta);
	}
	else
	{
		scale[0] += delta;
	}

	update();
}

void FETransformComponent::changeYScaleBy(float delta)
{
	if (uniformScaling)
	{
		changeScaleUniformlyBy(delta);
	}
	else
	{
		scale[1] += delta;
	}

	update();
}

void FETransformComponent::changeZScaleBy(float delta)
{
	if (uniformScaling)
	{
		changeScaleUniformlyBy(delta);
	}
	else
	{
		scale[2] += delta;
	}

	update();
}

void FETransformComponent::setScale(glm::vec3 newScale)
{
	scale = newScale;
	update();
}

void FETransformComponent::update()
{
	transformMatrix = glm::mat4(1.0);
	transformMatrix = glm::translate(transformMatrix, position);

	transformMatrix *= glm::toMat4(rotationQuaternion);
	transformMatrix = glm::scale(transformMatrix, glm::vec3(scale[0], scale[1], scale[2]));
}

glm::mat4 FETransformComponent::getTransformMatrix()
{
	return transformMatrix;
}

glm::quat FETransformComponent::getQuaternion()
{
	return rotationQuaternion;
}