#include "FEFreeCamera.h"
using namespace FocalEngine;

FEFreeCamera::FEFreeCamera(GLFWwindow* Window)
{
	window = Window;
}

FEFreeCamera::~FEFreeCamera()
{

}

void FEFreeCamera::move(float deltaTime)
{
	glm::vec4 forward = { 0.0f, 0.0f, -speed * deltaTime, 0.0f };
	glm::vec4 right = { speed * deltaTime, 0.0f, 0.0f, 0.0f };

	right = right * viewMatrix;
	forward = forward * viewMatrix;

	glm::normalize(right);
	glm::normalize(forward);

	if (leftKeyPreesed)
	{
		position.x -= right.x;
		position.y -= right.y;
		position.z -= right.z;
	}

	if (upKeyPreesed)
	{
		position.x += forward.x;
		position.y += forward.y;
		position.z += forward.z;
	}

	if (rightKeyPreesed)
	{
		position.x += right.x;
		position.y += right.y;
		position.z += right.z;
	}

	if (downKeyPreesed)
	{
		position.x -= forward.x;
		position.y -= forward.y;
		position.z -= forward.z;
	}

	updateViewMatrix();
}

void FEFreeCamera::reset()
{
	currentMouseXAngle = 0;
	currentMouseYAngle = 0;

	FEBasicCamera::reset();
}

void FEFreeCamera::setCursorToCenter()
{
	if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
	{
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		int xpos, ypos;
		glfwGetWindowPos(window, &xpos, &ypos);

		lastMouseX = xpos + (width / 2);
		lastMouseY = ypos + (height / 2);

		SetCursorPos(lastMouseX, lastMouseY);

		lastMouseX = lastMouseX - xpos;
		lastMouseY = lastMouseY - ypos;
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

	if (lastMouseX < mouseX && abs(lastMouseX - mouseX) > correctionToSensitivity)
	{
		currentMouseXAngle += (mouseX - lastMouseX) * 0.15f;
		setCursorToCenter();
	}
	else if (abs(lastMouseX - mouseX) > correctionToSensitivity)
	{
		currentMouseXAngle += (mouseX - lastMouseX) * 0.15f;
		setCursorToCenter();
	}

	if (lastMouseY < mouseY && abs(lastMouseY - mouseY) > correctionToSensitivity)
	{
		currentMouseYAngle += (mouseY - lastMouseY) * 0.15f;
		setCursorToCenter();
	}
	else if (abs(lastMouseY - mouseY) > correctionToSensitivity)
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

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		leftKeyPreesed = true;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
	{
		leftKeyPreesed = false;
	}

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		upKeyPreesed = true;
	}
	else if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
	{
		upKeyPreesed = false;
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		downKeyPreesed = true;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
	{
		downKeyPreesed = false;
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		rightKeyPreesed = true;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
	{
		rightKeyPreesed = false;
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