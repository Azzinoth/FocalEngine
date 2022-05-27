#include "FEBasicCamera.h"
using namespace FocalEngine;

FEBasicCamera::FEBasicCamera(std::string name) : FEObject(FE_CAMERA, name)
{
	yaw = 0.0f;
	updateAll();

	frustum = new float*[6];
	for (size_t i = 0; i < 6; i++)
	{
		frustum[i] = new float[4];
	}
}

FEBasicCamera::~FEBasicCamera()
{
	for (size_t i = 0; i < 6; i++)
	{
		delete[] frustum[i];
	}
	delete[] frustum;
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

glm::vec3 FEBasicCamera::getRight()
{
	return glm::normalize(glm::vec3(glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * viewMatrix));
}

glm::vec3 FEBasicCamera::getUp()
{
	return glm::normalize(glm::vec3(glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * viewMatrix));
}

void FEBasicCamera::updateFrustumPlanes()
{
	float   clip[16];
	float   t;

	glm::mat4 cliping = getProjectionMatrix() * getViewMatrix();
	for (int i = 0; i < 4; i++)
	{
		clip[i * 4] = cliping[i][0];
		clip[i * 4 + 1] = cliping[i][1];
		clip[i * 4 + 2] = cliping[i][2];
		clip[i * 4 + 3] = cliping[i][3];
	}

	/* Extract the numbers for the RIGHT plane */
	frustum[0][0] = clip[3] - clip[0];
	frustum[0][1] = clip[7] - clip[4];
	frustum[0][2] = clip[11] - clip[8];
	frustum[0][3] = clip[15] - clip[12];

	/* Normalize the result */
	t = sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
	frustum[0][0] /= t;
	frustum[0][1] /= t;
	frustum[0][2] /= t;
	frustum[0][3] /= t;

	/* Extract the numbers for the LEFT plane */
	frustum[1][0] = clip[3] + clip[0];
	frustum[1][1] = clip[7] + clip[4];
	frustum[1][2] = clip[11] + clip[8];
	frustum[1][3] = clip[15] + clip[12];

	/* Normalize the result */
	t = sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
	frustum[1][0] /= t;
	frustum[1][1] /= t;
	frustum[1][2] /= t;
	frustum[1][3] /= t;

	/* Extract the BOTTOM plane */
	frustum[2][0] = clip[3] + clip[1];
	frustum[2][1] = clip[7] + clip[5];
	frustum[2][2] = clip[11] + clip[9];
	frustum[2][3] = clip[15] + clip[13];

	/* Normalize the result */
	t = sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
	frustum[2][0] /= t;
	frustum[2][1] /= t;
	frustum[2][2] /= t;
	frustum[2][3] /= t;

	/* Extract the TOP plane */
	frustum[3][0] = clip[3] - clip[1];
	frustum[3][1] = clip[7] - clip[5];
	frustum[3][2] = clip[11] - clip[9];
	frustum[3][3] = clip[15] - clip[13];

	/* Normalize the result */
	t = sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
	frustum[3][0] /= t;
	frustum[3][1] /= t;
	frustum[3][2] /= t;
	frustum[3][3] /= t;

	/* Extract the FAR plane */
	frustum[4][0] = clip[3] - clip[2];
	frustum[4][1] = clip[7] - clip[6];
	frustum[4][2] = clip[11] - clip[10];
	frustum[4][3] = clip[15] - clip[14];

	/* Normalize the result */
	t = sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
	frustum[4][0] /= t;
	frustum[4][1] /= t;
	frustum[4][2] /= t;
	frustum[4][3] /= t;

	/* Extract the NEAR plane */
	frustum[5][0] = clip[3] + clip[2];
	frustum[5][1] = clip[7] + clip[6];
	frustum[5][2] = clip[11] + clip[10];
	frustum[5][3] = clip[15] + clip[14];

	/* Normalize the result */
	t = sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
	frustum[5][0] /= t;
	frustum[5][1] /= t;
	frustum[5][2] /= t;
	frustum[5][3] /= t;
}

float** FEBasicCamera::getFrustumPlanes()
{
	return frustum;
}

float FEBasicCamera::getMovementSpeed()
{
	return movementSpeed;
}

void FEBasicCamera::setMovementSpeed(float newValue)
{
	movementSpeed = newValue;
}