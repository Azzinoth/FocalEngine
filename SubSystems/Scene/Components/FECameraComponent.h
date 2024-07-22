#pragma once
#include "../Renderer/FEFramebuffer.h"

namespace FocalEngine
{
	struct FECameraComponent
	{
		friend class FECameraSystem;
		friend class FERenderer;

		FECameraComponent();
		FECameraComponent(const FECameraComponent& Other);
		void operator=(const FECameraComponent& Other);

		bool IsMainCamera() const;

		glm::mat4 GetViewMatrix() const;
		glm::mat4 GetProjectionMatrix() const;

		glm::vec3 GetUp() const;
		glm::vec3 GetForward() const;
		glm::vec3 GetRight() const;

		bool IsActive() const;
		void SetActive(const bool Active);

		// FIX ME! Should not be here.
		bool IsInputActive() const;

		bool IsUsingDefaultRenderTargetSize() const;
		void SetIsUsingDefaultRenderTargetSize(const bool NewValue);

		float GetFOV() const;
		void SetFOV(const float FOV);

		float GetNearPlane() const;
		void SetNearPlane(const float NearPlane);

		float GetFarPlane() const;
		void SetFarPlane(const float FarPlane);

		float GetAspectRatio() const;
		void SetAspectRatio(const float AspectRatio);

		float GetGamma() const;
		void SetGamma(const float Gamma);

		float GetExposure() const;
		void SetExposure(const float Exposure);

		// FIX ME! Should not be here.
		float GetMovementSpeed() const;
		void SetMovementSpeed(const float MovementSpeed);

		//FIX ME! It should not be here.
		float CurrentMouseXAngle = 0.0f;
		float CurrentMouseYAngle = 0.0f;

		void UpdateFrustumPlanes();
		std::vector<std::vector<float>> GetFrustumPlanes();

		int GetRenderTargetWidth() const;
		int GetRenderTargetHeight() const;

		void SetRenderTargetSize(const int Width, const int Height);

		//FEFramebuffer* GetCameraFramebuffer() const;
	private:
		// FIX ME! Should not be here.
		bool bIsInputActive = false;

		bool bIsActive = false;
		bool bIsMainCamera = false;
		bool bUseDefaultRenderTargetSize = true;

		int RenderTargetWidth = 0;
		int RenderTargetHeight = 0;

		float FOV = 70.0f;
		float NearPlane = 0.1f;
		float FarPlane = 15000.0f;
		float AspectRatio = 1.0f;

		/*float Yaw = 0.0f;
		float Pitch = 0.0f;
		float Roll = 0.0f;*/

		float Gamma = 2.2f;
		float Exposure = 1.0f;

		// FIX ME! Should not be here.
		float MovementSpeed = 10.0f;

		//glm::vec3 Position = glm::vec3(0.0f);
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;

		//void(*ClientOnUpdateImpl)(FEBasicCamera*) = nullptr;

		//float** Frustum;

		// FIX ME! Should not be here.
		int Type = 0;

		// FIX ME! Should not be here.
		int LastMouseX = 0;
		int LastMouseY = 0;
		
		const int CorrectionToSensitivity = 3;

		std::vector<std::vector<float>> Frustum;

		// FIX ME! Should not be here.
		double DistanceToModel = 10.0;
		double CurrentPolarAngle = 90.0;
		double CurrentAzimutAngle = 90.0;
		glm::vec3 TrackingObjectPosition = glm::vec3(0.0f);
		
		//FEFramebuffer* CameraFramebuffer = nullptr;
		void SetRenderTargetSizeInternal(const int Width, const int Height);
	};
}