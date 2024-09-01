#include "FEOpenXRRendering.h"

using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetOpenXRRendering()
{
	return FEOpenXRRendering::GetInstancePointer();
}
#endif

FEOpenXRRendering::FEOpenXRRendering() {}
FEOpenXRRendering::~FEOpenXRRendering() {}

void FEOpenXRRendering::GetViews()
{
	if (!FEOpenXR_CORE.bInitializedCorrectly)
		return;

	FE_OPENXR_ERROR(xrEnumerateViewConfigurationViews(FEOpenXR_CORE.OpenXRInstance, FEOpenXR_CORE.SystemID, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &ViewCount, nullptr));
	ViewConfigs.resize(ViewCount, { XR_TYPE_VIEW_CONFIGURATION_VIEW, nullptr });
	FE_OPENXR_ERROR(xrEnumerateViewConfigurationViews(FEOpenXR_CORE.OpenXRInstance, FEOpenXR_CORE.SystemID, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, ViewCount, &ViewCount, ViewConfigs.data()));
	Views.resize(ViewCount, { XR_TYPE_VIEW , nullptr });
}

void FEOpenXRRendering::CreateSwapChain()
{
	if (!FEOpenXR_CORE.bInitializedCorrectly)
		return;

	SwapChains.resize(ViewCount);
	SwapChainImages.resize(ViewCount);
	for (uint32_t i = 0; i < ViewCount; i++)
	{
		XrSwapchainCreateInfo SwapChainCreateInfo;
		SwapChainCreateInfo.type = XR_TYPE_SWAPCHAIN_CREATE_INFO;
		SwapChainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		SwapChainCreateInfo.createFlags = 0;
		SwapChainCreateInfo.format = /*GL_RGBA16F*/ 34842;
		SwapChainCreateInfo.sampleCount = ViewConfigs[i].recommendedSwapchainSampleCount;
		SwapChainCreateInfo.width = ViewConfigs[i].recommendedImageRectWidth;
		SwapChainCreateInfo.height = ViewConfigs[i].recommendedImageRectHeight;
		SwapChainCreateInfo.faceCount = 1;
		SwapChainCreateInfo.arraySize = 1;
		SwapChainCreateInfo.mipCount = 1;
		SwapChainCreateInfo.next = nullptr;

		FE_OPENXR_ERROR(xrCreateSwapchain(FEOpenXR_CORE.Session, &SwapChainCreateInfo, &SwapChains[i]));

		uint32_t SwapChainLength;
		FE_OPENXR_ERROR(xrEnumerateSwapchainImages(SwapChains[i], 0, &SwapChainLength, nullptr));

		// These are wrappers for the actual OpenGL texture id.
		SwapChainImages[i].resize(SwapChainLength, { XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR , nullptr });
		FE_OPENXR_ERROR(xrEnumerateSwapchainImages(SwapChains[i], SwapChainLength, &SwapChainLength, (XrSwapchainImageBaseHeader*)SwapChainImages[i].data()));

		if (SwapChainFB == nullptr)
		{
			SwapChainFB = RESOURCE_MANAGER.CreateFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, ViewConfigs[i].recommendedImageRectWidth, ViewConfigs[i].recommendedImageRectHeight);
			FETexture* TempTexture = SwapChainFB->GetColorAttachment();
			SwapChainFB->SetColorAttachment(nullptr);
			delete TempTexture;
		}
	}
}

void FEOpenXRRendering::Init()
{
	GetViews();
	CreateSwapChain();
}

/**
	* @brief Generates an asymmetric projection matrix tailored for VR applications.
	*
	* In VR, headset lenses often produce an asymmetric viewing frustum due to their unique optical
	* characteristics. This results in a non-uniform field-of-view (FOV) around the central line of sight.
	* This function creates a projection matrix based on such an asymmetric FOV, ensuring accurate
	* rendering that matches the optical properties of VR headsets.
	*
	* @param VRFov A structure containing the tangents of half-angles that define the VR viewing frustum.
	* @param NearZ The near clipping plane distance.
	* @param FarZ The far clipping plane distance.
	* @return A glm::mat4 representing the computed asymmetric projection matrix for VR.
*/
glm::mat4 CreateVRProjectionFov(const XrFovf& VRFov, float NearZ, float FarZ)
{
	float Left = NearZ * std::tan(VRFov.angleLeft);
	float Right = NearZ * std::tan(VRFov.angleRight);
	float Bottom = NearZ * std::tan(VRFov.angleDown);
	float Top = NearZ * std::tan(VRFov.angleUp);

	glm::mat4 Result(0.0f);

	Result[0][0] = 2.0f * NearZ / (Right - Left);
	Result[1][1] = 2.0f * NearZ / (Top - Bottom);
	Result[2][0] = (Right + Left) / (Right - Left);
	Result[2][1] = (Top + Bottom) / (Top - Bottom);
	Result[2][2] = -(FarZ + NearZ) / (FarZ - NearZ);
	Result[2][3] = -1.0f;
	Result[3][2] = -2.0f * FarZ * NearZ / (FarZ - NearZ);

	return Result;
}

void FEOpenXRRendering::OpenGLRenderLoop(const XrCompositionLayerProjectionView& LayerView, const XrSwapchainImageBaseHeader* SwapChainImage, int64_t SwapchainFormat)
{
	SwapChainFB->Bind();

	const uint32_t ColorTexture = reinterpret_cast<const XrSwapchainImageOpenGLKHR*>(SwapChainImage)->image;
	FE_GL_ERROR(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorTexture, 0));

	RENDERER.SetViewport(static_cast<int>(LayerView.subImage.imageRect.offset.x),
						 static_cast<int>(LayerView.subImage.imageRect.offset.y),
						 static_cast<int>(LayerView.subImage.imageRect.extent.width),
						 static_cast<int>(LayerView.subImage.imageRect.extent.height));

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.7f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	CurrentProjectionMatrix = CreateVRProjectionFov(LayerView.fov, 0.1f, 30000.0f);
	CurrentViewMatrix = glm::mat4(1.0f);
	glm::vec3 EyePosition = glm::vec3(LayerView.pose.position.x, LayerView.pose.position.y, LayerView.pose.position.z);
	CurrentViewMatrix = glm::translate(CurrentViewMatrix, EyePosition);

	glm::quat EyeOrientation = glm::quat(LayerView.pose.orientation.w, LayerView.pose.orientation.x, LayerView.pose.orientation.y, LayerView.pose.orientation.z);
	CurrentViewMatrix *= glm::toMat4(EyeOrientation);
	CurrentViewMatrix = glm::inverse(CurrentViewMatrix);

	// FIX ME!
	//static FEBasicCamera* CurrentCamera = new FEBasicCamera("VRCamera");
	//CurrentCamera->SetPosition(EyePosition);
	//CurrentCamera->ProjectionMatrix = CurrentProjectionMatrix;
	//CurrentCamera->ViewMatrix = CurrentViewMatrix;

	bValidSwapChain = true;
	// FIX ME! Temporary solution, only supports one scene
	FEScene* CurrentScene = SCENE_MANAGER.GetScenesByFlagMask(FESceneFlag::Active)[0];
	RENDERER.RenderVR(CurrentScene);
	bValidSwapChain = false;
}

bool FEOpenXRRendering::RenderLayer(XrTime PredictedDisplayTime, std::vector<XrCompositionLayerProjectionView>& ProjectionLayerViews, XrCompositionLayerProjection& Layer)
{
	XrViewState ViewState{ XR_TYPE_VIEW_STATE };
	uint32_t ViewCapacityInput = (uint32_t)Views.size();
	uint32_t ViewCountOutput;

	XrViewLocateInfo ViewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
	ViewLocateInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	ViewLocateInfo.displayTime = PredictedDisplayTime;
	ViewLocateInfo.space = FEOpenXR_CORE.ApplicationSpace;

	FE_OPENXR_ERROR(xrLocateViews(FEOpenXR_CORE.Session, &ViewLocateInfo, &ViewState, ViewCapacityInput, &ViewCountOutput, Views.data()));
	if ((ViewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT) == 0 ||
		(ViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT) == 0)
	{
		// There is no valid tracking poses for the views.
		return false;
	}
	ProjectionLayerViews.resize(ViewCountOutput);

	// Update view to the appropriate part of the swapchain image.
	for (uint32_t i = 0; i < ViewCountOutput; i++)
	{
		XrSwapchainImageAcquireInfo AcquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };

		uint32_t SwapChainImageIndex;
		FE_OPENXR_ERROR(xrAcquireSwapchainImage(SwapChains[i], &AcquireInfo, &SwapChainImageIndex));

		XrSwapchainImageWaitInfo WaitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
		WaitInfo.timeout = XR_INFINITE_DURATION;
		FE_OPENXR_ERROR(xrWaitSwapchainImage(SwapChains[i], &WaitInfo));

		ProjectionLayerViews[i] = { XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW };
		ProjectionLayerViews[i].pose = Views[i].pose;
		ProjectionLayerViews[i].fov = Views[i].fov;
		
		ProjectionLayerViews[i].subImage.swapchain = SwapChains[i];
		ProjectionLayerViews[i].subImage.imageRect.offset = { 0, 0 };
		ProjectionLayerViews[i].subImage.imageRect.extent = { int(ViewConfigs[i].recommendedImageRectWidth), int(ViewConfigs[i].recommendedImageRectHeight) };
		
		const XrSwapchainImageBaseHeader* const SwapChainImage = reinterpret_cast<XrSwapchainImageBaseHeader*>(&SwapChainImages[i][SwapChainImageIndex]);
		OpenGLRenderLoop(ProjectionLayerViews[i], SwapChainImage, /*m_colorSwapchainFormat*//*GL_RGBA16F*/ /*34842*/ -1);

		XrSwapchainImageReleaseInfo ReleaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
		FE_OPENXR_ERROR(xrReleaseSwapchainImage(SwapChains[i], &ReleaseInfo));
	}

	Layer.space = FEOpenXR_CORE.ApplicationSpace;
	Layer.layerFlags =
		/*m_options->Parsed.EnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND
		? XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT | XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT
		:*/ 0;
	Layer.viewCount = (uint32_t)ProjectionLayerViews.size();
	Layer.views = ProjectionLayerViews.data();

	return true;
}

void FEOpenXRRendering::RenderLoop()
{
	XrFrameWaitInfo FrameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
	FE_OPENXR_ERROR(xrWaitFrame(FEOpenXR_CORE.Session, &FrameWaitInfo, &FrameState));

	XrFrameBeginInfo FrameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
	FE_OPENXR_ERROR(xrBeginFrame(FEOpenXR_CORE.Session, &FrameBeginInfo));

	std::vector<XrCompositionLayerBaseHeader*> Layers;
	XrCompositionLayerProjection Layer{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	std::vector<XrCompositionLayerProjectionView> ProjectionLayerViews;

	if (FrameState.shouldRender == XR_TRUE)
	{
		if (RenderLayer(FrameState.predictedDisplayTime, ProjectionLayerViews, Layer))
			Layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&Layer));
	}

	XrFrameEndInfo FrameEndInfo{ XR_TYPE_FRAME_END_INFO };
	FrameEndInfo.displayTime = FrameState.predictedDisplayTime;
	FrameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	FrameEndInfo.layerCount = (uint32_t)Layers.size();
	FrameEndInfo.layers = Layers.data();
	FE_OPENXR_ERROR(xrEndFrame(FEOpenXR_CORE.Session, &FrameEndInfo));
}

void FEOpenXRRendering::Update()
{
	RenderLoop();
}

glm::vec2 FEOpenXRRendering::EyeResolution()
{
	return glm::vec2(int(ViewConfigs[0].recommendedImageRectWidth), int(ViewConfigs[0].recommendedImageRectHeight));
}

FEFramebuffer* FEOpenXRRendering::GetCurrentSwapChain()
{
	if (!bValidSwapChain)
		return nullptr;

	return SwapChainFB;
}

glm::mat4 FEOpenXRRendering::GetCurrentViewMatrix()
{
	if (!bValidSwapChain)
		return glm::mat4(1.0f);

	return CurrentViewMatrix;
}

glm::mat4 FEOpenXRRendering::GetCurrentProjectionMatrix()
{
	if (!bValidSwapChain)
		return glm::mat4(1.0f);

	return CurrentProjectionMatrix;
}