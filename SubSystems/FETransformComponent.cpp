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

void FETransformComponent::rotateQuaternion(float angle, glm::vec3 axis)
{
	rotationQuaternion = glm::quat(cos(angle / 2),
								   axis.x * sin(angle / 2),
								   axis.y * sin(angle / 2),
								   axis.z * sin(angle / 2)) * rotationQuaternion;
}

void FETransformComponent::setRotation(glm::vec3 newRotation)
{
	if (rotationAngles == newRotation)
		return;

	rotationQuaternion = glm::quat(1.0f, glm::vec3(0.0f));
	rotateQuaternion((float)(newRotation.x) * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
	rotateQuaternion((float)(newRotation.y) * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
	rotateQuaternion((float)(newRotation.z) * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));
	
	rotationAngles = newRotation;
	update();
}

void FETransformComponent::rotateByQuaternion(glm::quat quaternion)
{
	rotationQuaternion = quaternion * rotationQuaternion;
	glm::vec3 newRotationAngle = glm::eulerAngles(getQuaternion());

	newRotationAngle.x /= ANGLE_TORADIANS_COF;
	newRotationAngle.y /= ANGLE_TORADIANS_COF;
	newRotationAngle.z /= ANGLE_TORADIANS_COF;

	rotationAngles = newRotationAngle;
	update();
}

void FETransformComponent::changeScaleUniformlyBy(float delta)
{
	scale += delta;
	update();
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

	if (previousTransformMatrix != transformMatrix)
		dirtyFlag = true;
	previousTransformMatrix = transformMatrix;
}

glm::mat4 FETransformComponent::getTransformMatrix()
{
	return transformMatrix;
}

glm::quat FETransformComponent::getQuaternion()
{
	return rotationQuaternion;
}