#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	struct FECameraRenderingData;

	class FOCAL_ENGINE_API FEPointCloudSystem
	{
		friend class FEPointCloud;
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEPointCloudSystem)

		bool bInternalAdd = false;
		FEShader* StandardPointCloudShader = nullptr;

		FEShader* ComputePointCloudShader = nullptr;
		FEShader* ComputePointCloudAfterRenderingShader = nullptr;

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		static void DuplicatePointCloudComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);

		static Json::Value PointCloudComponentToJson(FEEntity* Entity);
		static void PointCloudComponentFromJson(FEEntity* Entity, Json::Value Root);

		bool SetAdvancedRendering(FEPointCloud* PointCloud, bool bUseAdvancedRendering);
	public:
		SINGLETON_PUBLIC_PART(FEPointCloudSystem)

		/**
			* Renders a point cloud using standard rendering pipeline (non-compute shader approach).
		*/
		void RenderStandard(FEEntity* Entity, FEEntity* Camera);
		bool RenderPointCloudComponent(FETransformComponent& TransformComponent, FEPointCloudComponent& PointCloudComponent, FEEntity* Camera);

		/**
			* Renders a point cloud to an internal 64-bit custom framebuffer using compute shaders.
			* This function can be called multiple times per frame to render
			* different point clouds to the same internal buffer.
			* 
			* Implementation based on:
			* Schütz, M., Kerbl, B., & Wimmer, M. (2021). Rendering point clouds with compute 
			* shaders and vertex order optimization. In Computer Graphics Forum (Vol. 40, No. 4, pp. 115-126).
			* https://doi.org/10.1111/cgf.14345
		*/
		bool RenderWithComputeShaders(FETransformComponent& TransformComponent, FEPointCloudComponent& PointCloudComponent, FEEntity* Camera);
		bool FuseComputeRenderedToFramebuffer(FEEntity* Camera);
		bool FuseComputeRenderedToFramebuffer(FEEntity* CameraUsed, FEFramebuffer* TargetFrameBuffer);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPointCloudSystem();
	#define POINT_CLOUD_SYSTEM (*static_cast<FEPointCloudSystem*>(GetPointCloudSystem()))
#else
	#define POINT_CLOUD_SYSTEM FEPointCloudSystem::GetInstance()
#endif
}