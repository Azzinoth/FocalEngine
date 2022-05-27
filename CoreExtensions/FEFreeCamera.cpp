#include "FEFreeCamera.h"
using namespace FocalEngine;

FEFreeCamera::FEFreeCamera(std::string name) : FEBasicCamera (name)
{

}

FEFreeCamera::~FEFreeCamera()
{

}

void FEFreeCamera::move(float deltaTime)
{
	glm::vec4 forward = { 0.0f, 0.0f, -(movementSpeed * 2) * (deltaTime / 1000), 0.0f };
	glm::vec4 right = { (movementSpeed * 2) * (deltaTime / 1000), 0.0f, 0.0f, 0.0f };

	right = right * viewMatrix;
	forward = forward * viewMatrix;

	glm::normalize(right);
	glm::normalize(forward);

	if (leftKeyPressed)
	{
		position.x -= right.x;
		position.y -= right.y;
		position.z -= right.z;
	}

	if (upKeyPressed)
	{
		position.x += forward.x;
		position.y += forward.y;
		position.z += forward.z;
	}

	if (rightKeyPressed)
	{
		position.x += right.x;
		position.y += right.y;
		position.z += right.z;
	}

	if (downKeyPressed)
	{
		position.x -= forward.x;
		position.y -= forward.y;
		position.z -= forward.z;
	}

	updateViewMatrix();

	if (clientOnUpdateImpl)
		clientOnUpdateImpl(this);
}

void FEFreeCamera::setIsInputActive(bool isActive)
{
	if (isActive)
	{
		setCursorToCenter();
	}
	else
	{
		leftKeyPressed = false;
		upKeyPressed = false;
		downKeyPressed = false;
		rightKeyPressed = false;
	}

	FEBasicCamera::setIsInputActive(isActive);
}

void FEFreeCamera::reset()
{
	currentMouseXAngle = 0;
	currentMouseYAngle = 0;

	FEBasicCamera::reset();
}

void FEFreeCamera::setCursorToCenter()
{
	if (APPLICATION.isWindowInFocus())
	{
		lastMouseX = renderTargetCenterX;
		lastMouseY = renderTargetCenterY;

		SetCursorPos(lastMouseX, lastMouseY);

		lastMouseX = lastMouseX - renderTargetShiftX;
		lastMouseY = lastMouseY - renderTargetShiftY;
	}
}

void FEFreeCamera::mouseMoveInput(double xpos, double ypos)
{
	if (!isInputActive)
		return;

	int mouseX = int(xpos);
	int mouseY = int(ypos);

	if (lastMouseX == 0) lastMouseX = mouseX;
	if (lastMouseY == 0) lastMouseY = mouseY;

	int test = renderTargetCenterX;
	int test2 = renderTargetCenterX - renderTargetShiftX;

	if (lastMouseX < mouseX || abs(lastMouseX - mouseX) > correctionToSensitivity)
	{
		currentMouseXAngle += (mouseX - lastMouseX) * 0.15f;
		setCursorToCenter();
	}

	if (lastMouseY < mouseY || abs(lastMouseY - mouseY) > correctionToSensitivity)
	{
		currentMouseYAngle += (mouseY - lastMouseY) * 0.15f;
		setCursorToCenter();
	}

	setYaw(currentMouseXAngle);
	if (currentMouseYAngle > 89.0f)
		currentMouseYAngle = 89.0f;
	if (currentMouseYAngle < -89.0f)
		currentMouseYAngle = -89.0f;
	setPitch(currentMouseYAngle);
}

void FEFreeCamera::keyboardInput(int key, int scancode, int action, int mods)
{
	if (!isInputActive)
		return;

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		leftKeyPressed = true;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		leftKeyPressed = false;
	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		upKeyPressed = true;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		upKeyPressed = false;
	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		downKeyPressed = true;
	}
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
	{
		downKeyPressed = false;
	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		rightKeyPressed = true;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		rightKeyPressed = false;
	}
}

void FEFreeCamera::setYaw(float newYaw)
{
	FEBasicCamera::setYaw(newYaw);
	currentMouseXAngle = newYaw;
}

void FEFreeCamera::setPitch(float newPitch)
{
	FEBasicCamera::setPitch(newPitch);
	currentMouseYAngle = newPitch;
}

void FEFreeCamera::setRenderTargetCenterX(int newRenderTargetCenterX)
{
	renderTargetCenterX = newRenderTargetCenterX;
}

void FEFreeCamera::setRenderTargetCenterY(int newRenderTargetCenterY)
{
	renderTargetCenterY = newRenderTargetCenterY;
}

void FEFreeCamera::setRenderTargetShiftX(int newRenderTargetShiftX)
{
	renderTargetShiftX = newRenderTargetShiftX;
}

void FEFreeCamera::setRenderTargetShiftY(int newRenderTargetShiftY)
{
	renderTargetShiftY = newRenderTargetShiftY;
}