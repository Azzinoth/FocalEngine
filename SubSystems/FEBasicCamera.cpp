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

float FEBasicCamera::getGamma()
{
	return gamma;
}

void FEBasicCamera::setGamma(float newGamma)
{
	gamma = newGamma;
}

float FEBasicCamera::getExposure()
{
	return exposure;
}

void FEBasicCamera::setExposure(float newExposure)
{
	exposure = newExposure;
}

bool FEBasicCamera::getIsInputActive()
{
	return isInputActive;
}

void FEBasicCamera::setIsInputActive(bool isActive)
{
	isInputActive = isActive;
}

void FocalEngine::FEBasicCamera::reset()
{
	fov = 70.0f;
	nearPlane = 0.1f;
	farPlane = 5000.0f;
	aspectRatio = 1.0f;

	yaw = 0.0f;
	pitch = 0.0f;
	roll = 0.0f;

	gamma = 2.2f;
	exposure = 1.0f;

	glm::vec3 position = glm::vec3(0.0f);

	updateAll();
}

void FEBasicCamera::setLookAt(glm::vec3 lookAt)
{
	viewMatrix = glm::lookAt(getPosition(), lookAt, glm::vec3(0.0, 1.0, 0.0));
}

glm::vec3 FEBasicCamera::getForward()
{
	return glm::normalize(glm::vec3(glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) * viewMatrix));
}

void FEBasicCamera::setOnUpdate(void(*func)(FEBasicCamera*))
{
	clientOnUpdateImpl = func;
}