#pragma once

// FIX ME! It should not be here.
#include "Scene/Components/FETransformComponent.h"

namespace FocalEngine
{
	class FEBasicCamera : public FEObject
	{
		friend class FERenderer;
		friend class FEOpenXRRendering;
	public:
		FEBasicCamera(std::string Name);
		~FEBasicCamera();

		float GetFov() const;
		void SetFov(float NewFov);
		float GetNearPlane() const;
		void SetNearPlane(float NewNearPlane);
		float GetFarPlane() const;
		void SetFarPlane(float NewFarPlane);

		virtual float GetYaw();
		virtual void SetYaw(float NewYaw);
		virtual float GetPitch();
		virtual void SetPitch(float NewPitch);
		virtual float GetRoll();
		virtual void SetRoll(float NewRoll);
		glm::vec3 GetPosition() const;
		void SetPosition(glm::vec3 NewPosition);
		virtual glm::vec3 GetUp();
		virtual glm::vec3 GetForward();
		virtual glm::vec3 GetRight();

		virtual void SetLookAt(glm::vec3 LookAt);

		float GetAspectRatio() const;
		void SetAspectRatio(float NewAspectRatio);

		float GetGamma() const;
		void SetGamma(float NewGamma);
		float GetExposure() const;
		void SetExposure(float NewExposure);

		glm::mat4 GetViewMatrix() const;
		virtual void UpdateViewMatrix();
		glm::mat4 GetProjectionMatrix() const;
		virtual void UpdateProjectionMatrix();

		virtual void UpdateAll();

		virtual void Move(float DeltaTime = 0.0f) {};

		virtual bool GetIsInputActive();
		virtual void SetIsInputActive(bool bIsActive);

		virtual void KeyboardInput(int Key, int Scancode, int Action, int Mods) {};
		virtual void MouseMoveInput(double Xpos, double Ypos) {};
		virtual void MouseScrollInput(double Xoffset, double Yoffset) {};

		virtual void Reset();

		virtual void SetOnUpdate(void(*Func)(FEBasicCamera*));

		virtual void UpdateFrustumPlanes();
		virtual float** GetFrustumPlanes();

		virtual float GetMovementSpeed();
		virtual void SetMovementSpeed(float NewValue);

		int GetCameraType() const;

		FETransformComponent TestTransform;
	protected:
		bool bIsInputActive = true;

		float Fov = 70.0f;
		float NearPlane = 0.1f;
		float FarPlane = 15000.0f;
		float AspectRatio = 1.0f;

		// Start with this variables commented out.
		/*float Yaw = 0.0f;
		float Pitch = 0.0f;
		float Roll = 0.0f;*/

		float Gamma = 2.2f;
		float Exposure = 1.0f;

		float MovementSpeed = 10.0f;

		// Proceed with this variable commented out.
		//glm::vec3 Position = glm::vec3(0.0f);
		
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;

		void(*ClientOnUpdateImpl)(FEBasicCamera*) = nullptr;

		float** Frustum;

		int Type = 0;
	};
}