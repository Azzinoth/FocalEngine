#pragma once
#include "../Renderer/FEFramebuffer.h"

namespace FocalEngine
{
	enum FEViewportType
	{
		FE_VIEWPORT_NULL = 0,
		FE_VIEWPORT_OS_WINDOW = 1,
		FE_VIEWPORT_GLFW_WINDOW = 2,
		FE_VIEWPORT_FEWINDOW = 3,
		FE_VIEWPORT_IMGUI_WINDOW = 4
	};

	class FEViewport
	{
		friend class FERenderer;
		friend class FEngine;
		friend class FECameraSystem;
		friend struct FECameraComponent;

		std::string ID = "";

		int X = 0;
		int Y = 0;
		int Width = 0;
		int Height = 0;

		FEViewportType Type = FE_VIEWPORT_NULL;
		void* WindowHandle = nullptr;

		FEViewport()
		{
			ID = APPLICATION.GetUniqueHexID();
		}

	public:
		std::string GetID() const
		{
			return ID;
		}

		int GetX() const
		{
			return X;
		}

		int GetY() const
		{
			return Y;
		}

		int GetWidth() const
		{
			return Width;
		}

		int GetHeight() const
		{
			return Height;
		}
	};

	struct FOCAL_ENGINE_API FECameraComponent
	{
#define DEFAULT_CAMERA_CLEAR_COLOR glm::vec4(0.55f, 0.73f, 0.87f, 1.0f)

		friend class FECameraSystem;
		friend class FERenderer;

		FECameraComponent();
		FECameraComponent(const FECameraComponent& Other) = default;

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

		FEViewport* GetViewport() const;

		glm::vec4 GetClearColor() const;
		void SetClearColor(glm::vec4 NewClearColor);

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

		// *********** Anti-Aliasing(FXAA) ***********
		float GetFXAASpanMax();
		void SetFXAASpanMax(float NewValue);

		float GetFXAAReduceMin();
		void SetFXAAReduceMin(float NewValue);

		float GetFXAAReduceMul();
		void SetFXAAReduceMul(float NewValue);

		// *********** Bloom ***********
		float GetBloomThreshold();
		void SetBloomThreshold(float NewValue);

		float GetBloomSize();
		void SetBloomSize(float NewValue);

		// *********** Depth of Field ***********
		float GetDOFNearDistance();
		void SetDOFNearDistance(float NewValue);

		float GetDOFFarDistance();
		void SetDOFFarDistance(float NewValue);

		float GetDOFStrength();
		void SetDOFStrength(float NewValue);

		float GetDOFDistanceDependentStrength();
		void SetDOFDistanceDependentStrength(float NewValue);

		// *********** Chromatic Aberration ***********
		float GetChromaticAberrationIntensity();
		void SetChromaticAberrationIntensity(float NewValue);

		// *********** SSAO ***********
		bool IsSSAOEnabled();
		void SetSSAOEnabled(const bool NewValue);

		int GetSSAOSampleCount();
		void SetSSAOSampleCount(int NewValue);

		bool IsSSAOSmallDetailsEnabled();
		void SetSSAOSmallDetailsEnabled(const bool NewValue);

		bool IsSSAOResultBlured();
		void SetSSAOResultBlured(const bool NewValue);

		float GetSSAOBias();
		void SetSSAOBias(const float NewValue);

		float GetSSAORadius();
		void SetSSAORadius(const float NewValue);

		float GetSSAORadiusSmallDetails();
		void SetSSAORadiusSmallDetails(const float NewValue);

		float GetSSAOSmallDetailsWeight();
		void SetSSAOSmallDetailsWeight(const float NewValue);

		//************** Distance Fog **************
		bool IsDistanceFogEnabled();
		void SetDistanceFogEnabled(const bool NewValue);

		float GetDistanceFogDensity();
		void SetDistanceFogDensity(const float NewValue);

		float GetDistanceFogGradient();
		void SetDistanceFogGradient(const float NewValue);

		// FIX ME! Should not be here.
		int Type = 0;
		bool bIsInputGrabingActive = false;
		bool bIsInputReadingActive = false;

		// FIX ME! Should not be here.
		double DistanceToModel = 10.0;
	private:
		bool bIsActive = false;
		bool bIsMainCamera = false;
		// FIX ME! It should be either free or bound to a specific window(canvas).
		//bool bUseDefaultRenderTargetSize = true;

		FEViewport* Viewport = nullptr;

		glm::vec4 ClearColor = DEFAULT_CAMERA_CLEAR_COLOR;

		int RenderTargetWidth = 0;
		int RenderTargetHeight = 0;

		// FIX ME! Should not be here.
		int RenderTargetCenterX = 0;
		int RenderTargetCenterY = 0;

		float FOV = 50.68f;
		float NearPlane = 0.01f;
		float FarPlane = 5000.0f;
		float AspectRatio = 1.0f;

		float Gamma = 2.2f;
		float Exposure = 1.0f;

		// FIX ME! Should not be here.
		float MovementSpeed = 10.0f;

		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;

		int CorrectionToSensitivity = 2;

		std::vector<std::vector<float>> Frustum;

		// FIX ME! Should not be here.
		int LastMouseX = 0;
		int LastMouseY = 0;
		// FIX ME! Should not be here.
		double CurrentPolarAngle = 90.0;
		double CurrentAzimutAngle = 90.0;
		glm::vec3 TrackingObjectPosition = glm::vec3(0.0f);
		
		void SetRenderTargetSizeInternal(const int Width, const int Height);

		// *********** Anti-Aliasing(FXAA) ***********
		float FXAASpanMax = 8.0f;
		float FXAAReduceMin = 1.0f / 128.0f;
		float FXAAReduceMul = 0.4f;

		// *********** Bloom ***********
		float BloomThreshold = 1.0f;
		float BloomSize = 5.0f;

		// *********** Depth of Field ***********
		float DOFStrength = 2.0f;
		float DOFNearDistance = 0.0f;
		float DOFFarDistance = 9000.0f;
		float DOFDistanceDependentStrength = 100.0f;

		// *********** Chromatic Aberration ***********
		float ChromaticAberrationIntensity = 1.0f;

		// *********** SSAO ***********
		bool bSSAOActive = true;
		int SSAOSampleCount = 16;

		bool bSSAOSmallDetails = true;
		bool bSSAOBlured = true;

		float SSAOBias = 0.013f;
		float SSAORadius = 10.0f;
		float SSAORadiusSmallDetails = 0.4f;
		float SSAOSmallDetailsWeight = 0.2f;

		//************** Distance Fog **************
		float DistanceFogDensity = 0.007f;
		float DistanceFogGradient = 2.5f;
		bool bDistanceFogEnabled = false;
	};
}