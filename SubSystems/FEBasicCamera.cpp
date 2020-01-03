#include "FEBasicCamera.h"
using namespace FocalEngine;

FEBasicCamera::FEBasicCamera()
{
	yaw = 0.0f;
	updateAll();
}

FEBasicCamera::~FEBasicCamera()
{

}

float FEBasicCamera::getYaw()
{
	return yaw;
}

void FEBasicCamera::setYaw(float newYaw)
{
	yaw = newYaw;
	updateViewMatrix();
}

float FEBasicCamera::getPitch()
{
	return pitch;
}

void FEBasicCamera::setPitch(float newPitch)
{
	pitch = newPitch;
	updateViewMatrix();
}

float FEBasicCamera::getRoll()
{
	return roll;
}

void FEBasicCamera::setRoll(float newRoll)
{
	roll = newRoll;
	updateViewMatrix();
}

glm::vec3 FEBasicCamera::getPosition()
{
	return position;
}

void FEBasicCamera::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	updateViewMatrix();
}

float FEBasicCamera::getAspectRatio()
{
	return aspectRatio;
}

void FEBasicCamera::setAspectRatio(float newAspectRatio)
{
	aspectRatio = newAspectRatio;
	updateProjectionMatrix();
}

void FEBasicCamera::updateViewMatrix()
{
	viewMatrix = glm::mat4(1.0f);

	viewMatrix = glm::rotate(viewMatrix, getPitch() * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
	viewMatrix = glm::rotate(viewMatrix, getYaw() * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
	viewMatrix = glm::rotate(viewMatrix, getRoll() * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));

	glm::vec3 cameraPosition = getPosition();
	glm::vec3 negativeCameraPosition = -cameraPosition;

	viewMatrix = glm::translate(viewMatrix, negativeCameraPosition);
}

glm::mat4 FEBasicCamera::getViewMatrix()
{
	return viewMatrix;
}

glm::mat4 FEBasicCamera::getProjectionMatrix()
{
	return projectionMatrix;
}

void FEBasicCamera::updateProjectionMatrix()
{
	projectionMatrix = glm::perspective(fov, aspectRatio, nearPlane, farPlane);
}

void FEBasicCamera::updateAll()
{
	updateViewMatrix();
	updateProjectionMatrix();
}

float FEBasicCamera::getFov()
{
	return fov;
}

void FEBasicCamera::setFov(float newFov)
{
	fov = newFov;
	updateProjectionMatrix();
}

float FEBasicCamera::getNearPlane()
{
	return nearPlane;
}

void FEBasicCamera::setNearPlane(float newNearPlane)
{
	nearPlane = newNearPlane;
	updateProjectionMatrix();
}

float FEBasicCamera::getFarPlane()
{
	return farPlane;
}

void FEBasicCamera::setFarPlane(float newFarPlane)
{
	farPlane = newFarPlane;
	updateProjectionMatrix();
}