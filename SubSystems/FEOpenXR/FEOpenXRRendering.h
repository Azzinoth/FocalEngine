#pragma once

#include "FEOpenXRInput.h"

namespace FocalEngine
{
	class FEOpenXRRendering
	{
		friend class FEOpenXR;
	public:
		SINGLETON_PUBLIC_PART(FEOpenXRRendering)

		void Init();
		void Update();

		//bool bShowCube = true;
		glm::vec2 EyeResolution();

		// Returns the current swap chain for rendering.
		// Note: 
		// - If OpenXR is not in the rendering process, this will return nullptr.
		// - It's recommended to use this function only within RENDERER callbacks.
		FEFramebuffer* GetCurrentSwapChain();

		// Retrieves the current view matrix for rendering.
		// Note: 
		// - If OpenXR is not in the rendering process, an identity matrix will be returned.
		// - Intended for use within RENDERER callbacks.
		glm::mat4 GetCurrentViewMatrix();

		// Gets the current projection matrix for rendering.
		// Note: 
		// - If OpenXR is not in the rendering process, an identity matrix will be returned.
		// - Use this function primarily within RENDERER callbacks.
		glm::mat4 GetCurrentProjectionMatrix();
	private:
		SINGLETON_PRIVATE_PART(FEOpenXRRendering)

		XrFrameState FrameState;

		// Each physical Display/Eye is described by a view
		uint32_t ViewCount = 0;
		std::vector<XrViewConfigurationView> ViewConfigs;
		std::vector<XrCompositionLayerProjectionView> Projections;
		std::vector<XrView> Views;

		std::vector<std::vector<XrSwapchainImageOpenGLKHR>> SwapChainImages;
		std::vector<XrSwapchain> SwapChains;
		FEFramebuffer* SwapChainFB = nullptr;
		bool bValidSwapChain = false;
		glm::mat4 CurrentProjectionMatrix;
		glm::mat4 CurrentViewMatrix;

		void GetViews();
		void CreateSwapChain();

		bool RenderLayer(XrTime PredictedDisplayTime, std::vector<XrCompositionLayerProjectionView>& ProjectionLayerViews, XrCompositionLayerProjection& Layer);
		void OpenGLRenderLoop(const XrCompositionLayerProjectionView& LayerView, const XrSwapchainImageBaseHeader* SwapChainImage, int64_t SwapchainFormat);
		void RenderLoop();
	};

#define FEOpenXR_RENDERING FEOpenXRRendering::GetInstance()
}