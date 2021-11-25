#pragma once

#include "../SubSystems/FEBasicCamera.h"

namespace FocalEngine
{
	class FEFreeCamera : public FEBasicCamera
	{
	public:
		FEFreeCamera(GLFWwindow* Window, std::string name);
		~FEFreeCamera();

		float currentMouseXAngle = 0;
		float currentMouseYAngle = 0;
		
		void setYaw(float newYaw);
		void setPitch(float newPitch);

		void setIsInputActive(bool isActive) final;

		void keyboardInput(int key, int scancode, int action, int mods) final;
		void mouseMoveInput(double xpos, double ypos) final;
		void move(float deltaTime = 0.0f);

		void reset() override;

		void setRenderTargetCenterX(int newRenderTargetCenterX);
		void setRenderTargetCenterY(int newRenderTargetCenterY);

		void setRenderTargetShiftX(int newRenderTargetShiftX);
		void setRenderTargetShiftY(int newRenderTargetShiftY);
	private:
		int lastMouseX = 0;
		int lastMouseY = 0;

		int renderTargetCenterX = 0;
		int renderTargetCenterY = 0;

		int renderTargetShiftX = 0;
		int renderTargetShiftY = 0;

		bool leftKeyPressed = false;
		bool upKeyPressed = false;
		bool rightKeyPressed = false;
		bool downKeyPressed = false;

		const int correctionToSensitivity = 3;

		void setCursorToCenter();
	};
}