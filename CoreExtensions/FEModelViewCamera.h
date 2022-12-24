#pragma once

#include "../SubSystems/FEBasicCamera.h"

namespace FocalEngine
{
	class FEModelViewCamera : public FEBasicCamera
	{
	public:
		FEModelViewCamera(std::string Name);
		~FEModelViewCamera();

		void KeyboardInput(int Key, int Scancode, int Action, int Mods) final;
		void MouseMoveInput(double Xpos, double Ypos) final;
		void MouseScrollInput(double Xoffset, double Yoffset) final;
		void Move(float DeltaTime = 0.0f) override;

		void Reset() override;
		void UpdateViewMatrix() override;

		double GetPolarAngle();
		void SetPolarAngle(double NewValue);
		double GetAzimutAngle();
		void SetAzimutAngle(double NewValue);

		double GetDistanceToModel();
		void SetDistanceToModel(double NewValue);

		glm::vec3 GetTrackingObjectPosition();
		void SetTrackingObjectPosition(glm::vec3 NewValue);
	private:
		int LastMouseX = 0;
		int LastMouseY = 0;

		double DistanceToModel = 10.0;
		double CurrentPolarAngle = 90.0;
		double CurrentAzimutAngle = 90.0;

		glm::vec3 TrackingObjectPosition = glm::vec3(0.0f);

		glm::dvec3 PolarToCartesian(double PolarAngle, double AzimutAngle, double R = 1.0);
	};
}