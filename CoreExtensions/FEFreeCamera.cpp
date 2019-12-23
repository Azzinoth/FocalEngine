#include "FEFreeCamera.h"
using namespace FocalEngine;

FEFreeCamera::FEFreeCamera(GLFWwindow* Window)
{
	window = Window;
}

FEFreeCamera::~FEFreeCamera()
{

}

void FEFreeCamera::move()
{
	
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
	setPitch(currentMouseYAngle);
}

void FEFreeCamera::keyboardInput(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UP && action != GLFW_RELEASE)
	{
		glm::vec4 forward = { 0.0f, 0.0f, -speed, 0.0f };
		forward = forward * viewMatrix;
		glm::normalize(forward);

		position.x += forward.x;
		position.y += forward.y;
		position.z += forward.z;
	}
	if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE)
	{
		glm::vec4 forward = { 0.0f, 0.0f, -speed, 0.0f };
		forward = forward * viewMatrix;
		glm::normalize(forward);

		position.x -= forward.x;
		position.y -= forward.y;
		position.z -= forward.z;
	}

	if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE)
	{
		glm::vec4 right = { speed, 0.0f, 0.0f, 0.0f };
		right = right * viewMatrix;
		glm::normalize(right);

		position.x -= right.x;
		position.y -= right.y;
		position.z -= right.z;
	}
	if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE)
	{
		glm::vec4 right = { speed, 0.0f, 0.0f, 0.0f };
		right = right * viewMatrix;
		glm::normalize(right);

		position.x += right.x;
		position.y += right.y;
		position.z += right.z;
	}

	updateViewMatrix();
}