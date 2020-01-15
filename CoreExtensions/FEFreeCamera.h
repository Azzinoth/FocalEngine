#pragma once

#include "../SubSystems/FEBasicCamera.h"

namespace FocalEngine
{
	class FEFreeCamera : public FEBasicCamera
	{
	public:
		FEFreeCamera(GLFWwindow* Window);
		~FEFreeCamera();

		float currentMouseXAngle = 0;
		float currentMouseYAngle = 0;
		float speed = 0.01f;

		void setYaw(float newYaw);
		void setPitch(float newPitch);

		void keyboardInput(int key, int scancode, int action, int mods) final;
		void mouseMoveInput(double xpos, double ypos) final;
		void move(float deltaTime = 0.0f);
	private:
		int lastMouseX = 0;
		int lastMouseY = 0;

		bool leftKeyPreesed = false;
		bool upKeyPreesed = false;
		bool rightKeyPreesed = false;
		bool downKeyPreesed = false;

		const int correctionToSensitivity = 3;

		void setCursorToCenter();
	};
}