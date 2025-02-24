#pragma once
#include "../Renderer/FEViewport.h"

namespace FocalEngine
{
	enum class FERenderingPipeline
	{
		Deferred,
		Forward_Simplified,
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
		void SetViewMatrix(const glm::mat4 NewViewMatrix);
		glm::mat4 GetProjectionMatrix() const;
		void SetProjectionMatrix(const glm::mat4 NewProjectionMatrix);

		glm::vec3 GetUp() const;
		glm::vec3 GetForward() const;
		glm::vec3 GetRight() const;

		bool IsActive() const;
		void SetActive(const bool Active);

		bool IsClearColorEnabled() const;
		void SetIsClearColorEnabled(const bool bEnabled);

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

		void UpdateFrustumPlanes();
		std::vector<std::vector<float>> GetFrustumPlanes();

		int GetRenderTargetWidth() const;
		int GetRenderTargetHeight() const;

		float GetRenderScale();

		bool TryToSetViewportSize(const int Width, const int Height);

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

		const FEViewport* GetViewport();

		FERenderingPipeline GetRenderingPipeline() const;

		// *********** Temporal Anti-Aliasing common ***********
		bool IsTemporalJitterEnabled();
		void SetTemporalJitterEnabled(const bool NewValue);

		size_t GetTemporalJitterSequenceLength();
		void SetTemporalJitterSequenceLength(size_t NewValue);

		glm::vec2 GetTemporalJitterOffset();
	private:
		bool bIsActive = false;
		bool bIsMainCamera = false;

		FERenderingPipeline RenderingPipeline = FERenderingPipeline::Deferred;
		FEViewport* Viewport = nullptr;

		glm::vec4 ClearColor = DEFAULT_CAMERA_CLEAR_COLOR;
		bool bClearColorEnabled = true;

		float RenderScale = 1.0f;
		int RenderTargetWidth = 0;
		int RenderTargetHeight = 0;

		float FOV = 50.68f;
		float NearPlane = 0.01f;
		float FarPlane = 5000.0f;
		float AspectRatio = 1.0f;

		float Gamma = 2.2f;
		float Exposure = 1.0f;

		glm::mat4 ViewMatrix = glm::mat4(1.0f);
		glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
		std::vector<std::vector<float>> Frustum;

		bool TryToSetViewportSizeInternal(const int Width, const int Height);

		// *********** Temporal Anti-Aliasing variables ***********
		bool bTemporalJitterEnabled = false;
		unsigned long long LastTemporalFrameIndexUpdateEngineFrame = 0;
		int TemporalFrameIndex = 0;
		glm::vec2 CurrentTemporalJitterOffset = glm::vec2(0.0f);
		size_t TemporalJitterSequenceLength = 64;
		glm::mat4 PreviousFrameViewMatrix = glm::mat4(1.0f);

		void UpdateTemporalJitterOffset();
		
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