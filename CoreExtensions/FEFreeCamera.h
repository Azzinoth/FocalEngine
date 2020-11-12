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
		
		float getSpeed();
		void setSpeed(float newSpeed);

		void setYaw(float newYaw);
		void setPitch(float newPitch);

		void setIsInputActive(bool isActive) final;

		void keyboardInput(int key, int scancode, int action, int mods) final;
		void mouseMoveInput(double xpos, double ypos) final;
		void move(float deltaTime = 0.0f);

		void reset() override;
	private:
		int lastMouseX = 0;
		int lastMouseY = 0;
		float speed = 10.0f;

		bool leftKeyPressed = false;
		bool upKeyPressed = false;
		bool rightKeyPressed = false;
		bool downKeyPressed = false;

		const int correctionToSensitivity = 3;

		void setCursorToCenter();
	};
}