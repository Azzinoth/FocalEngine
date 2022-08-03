#pragma once

#include "../SubSystems/FEBasicCamera.h"

namespace FocalEngine
{
	class FEFreeCamera : public FEBasicCamera
	{
	public:
		FEFreeCamera(std::string Name);
		~FEFreeCamera();

		float CurrentMouseXAngle = 0;
		float CurrentMouseYAngle = 0;
		
		void SetYaw(float NewYaw) override;
		void SetPitch(float NewPitch) override;

		void SetIsInputActive(bool bIsActive) final;

		void KeyboardInput(int Key, int Scancode, int Action, int Mods) final;
		void MouseMoveInput(double Xpos, double Ypos) final;
		void Move(float DeltaTime = 0.0f) override;

		void Reset() override;

		void SetRenderTargetCenterX(int NewRenderTargetCenterX);
		void SetRenderTargetCenterY(int NewRenderTargetCenterY);

		void SetRenderTargetShiftX(int NewRenderTargetShiftX);
		void SetRenderTargetShiftY(int NewRenderTargetShiftY);
	private:
		int LastMouseX = 0;
		int LastMouseY = 0;

		int RenderTargetCenterX = 0;
		int RenderTargetCenterY = 0;

		int RenderTargetShiftX = 0;
		int RenderTargetShiftY = 0;

		bool bLeftKeyPressed = false;
		bool bUpKeyPressed = false;
		bool bRightKeyPressed = false;
		bool bDownKeyPressed = false;

		const int CorrectionToSensitivity = 3;

		void SetCursorToCenter();
	};
}