#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../SubSystems/Scene/Components/Systems/FEComponentSystems.h"
#include "../SubSystems/FEInput.h"

namespace FocalEngine
{
	#define FE_MAX_LINES 200000

	class FEngine;
	class FERenderer;

	class FEGBuffer
	{
		friend FERenderer;

		void InitializeResources(FEFramebuffer* MainFrameBuffer);
	public:
		FEGBuffer(FEFramebuffer* MainFrameBuffer);
		
		FEFramebuffer* GFrameBuffer = nullptr;

		FETexture* Positions = nullptr;
		FETexture* Normals = nullptr;
		FETexture* Albedo = nullptr;
		FETexture* MaterialProperties = nullptr;
		FETexture* ShaderProperties = nullptr;

		void RenderTargetResize(FEFramebuffer* MainFrameBuffer);
	};

	class FESSAO
	{
		friend FERenderer;

		FESSAO(FEFramebuffer* MainFrameBuffer);
		void InitializeResources(FEFramebuffer* MainFrameBuffer);
		void RenderTargetResize(FEFramebuffer* MainFrameBuffer);

		FEFramebuffer* FB = nullptr;
		FEShader* Shader = nullptr;
	};

	// TO-DO: That is very non GPU memory efficient way to support multiple cameras.
	struct FECameraRenderingData
	{
		friend class FERenderer;
		FEEntity* CameraEntity = nullptr;
		FEFramebuffer* SceneToTextureFB = nullptr;
		FEGBuffer* GBuffer = nullptr;
		FESSAO* SSAO = nullptr;
		FETexture* DepthPyramid = nullptr;
		std::vector<FEPostProcess*> PostProcessEffects;

		FETexture* FinalScene = nullptr;

		~FECameraRenderingData()
		{
			delete SceneToTextureFB;
			delete GBuffer;
			delete SSAO;
			delete DepthPyramid;
		}
	};
	
	class FOCAL_ENGINE_API FERenderer
	{
		friend FEngine;
		friend FECameraSystem;
	public:
		SINGLETON_PUBLIC_PART(FERenderer)

		void Render(FEScene* CurrentScene);
		
		void RenderGameModelComponent(FEEntity* Entity, FEEntity* ForceCamera = nullptr, bool bReloadUniformBlocks = false);
		void RenderGameModelComponent(FEGameModelComponent& GameModelComponent, FETransformComponent& TransformComponent, FEScene* ParentScene, FEEntity* ForceCamera = nullptr, bool bReloadUniformBlocks = false);
		void RenderGameModelComponentForward(FEEntity* Entity, FEEntity* ForceCamera = nullptr, bool bReloadUniformBlocks = false);
		void RenderGameModelComponentWithInstanced(FEEntity* Entity, FEEntity* ForceCamera = nullptr, bool bShadowMap = false, bool bReloadUniformBlocks = false, size_t PrefabIndex = 0);
		void RenderTerrainComponent(FEEntity* TerrainEntity, FEEntity* ForceCamera = nullptr);
		
		FETexture* GetCameraResult(FEEntity* CameraEntity);

		void DrawLine(glm::vec3 BeginPoint, glm::vec3 EndPoint, glm::vec3 Color = glm::vec3(1.0f), float Width = 0.1f);

		float TestTime = 0.0f;
		float LastTestTime = 0.0f;

		bool bFreezeCulling = false;
		void DrawAABB(FEAABB AABB, glm::vec3 Color = glm::vec3(0.1f, 0.6f, 0.1f), float LineWidth = 0.2f);

		bool IsOcclusionCullingEnabled();
		void SetOcclusionCullingEnabled(bool NewValue);

		void UpdateSSAO();

		std::unordered_map<std::string, std::function<FETexture* ()>> GetDebugOutputTextures();
		void SimplifiedRender(FEScene* CurrentScene);

		// *********** VR Rendering ***********

		bool bVRActive = false;
		void InitVR(int VRScreenW, int VRScreenH);
		int VRScreenW = 0;
		int VRScreenH = 0;
		FEFramebuffer* SceneToVRTextureFB = nullptr;
		void RenderVR(FEScene* CurrentScene);

		void UpdateVRRenderTargetSize(int VRScreenW, int VRScreenH);

		// *********** VR Rendering END ***********

		void RenderToFrameBuffer(FETexture* SceneTexture, FEFramebuffer* Target);
		void RenderToFrameBuffer(FETexture* SceneTexture, GLuint Target);

		bool CombineFrameBuffers(FEFramebuffer* FirstSource, FEFramebuffer* SecondSource, FEFramebuffer* Target);

		void AddAfterRenderCallback(std::function<void()> Callback);

		bool IsClearActiveInSimplifiedRendering();
		void SetClearActiveInSimplifiedRendering(bool NewValue);

		void SetGLViewport(int X, int Y, int Width, int Height);
		void SetGLViewport(glm::ivec4 ViewPortData);
		glm::ivec4 GetGLViewport();

		FECameraRenderingData* GetCameraRenderingData(FEEntity* CameraEntity);
		void AddPostProcess(FECameraRenderingData* CameraRenderingData, FEPostProcess* NewPostProcess, const bool NoProcessing = false);
	private:
		SINGLETON_PRIVATE_PART(FERenderer)

		void LoadStandardParams(FEShader* Shader, FEMaterial* Material, FETransformComponent* Transform, FEEntity* ForceCamera = nullptr, bool IsReceivingShadows = false, const bool IsUniformLighting = false);
		void LoadStandardParams(FEShader* Shader, bool IsReceivingShadows, FEEntity* ForceCamera = nullptr, const bool IsUniformLighting = false);
		void LoadUniformBlocks(FEScene* CurrentScene);

		void SaveScreenshot(std::string FileName, FEScene* SceneToWorkWith);

		FEMaterial* ShadowMapMaterial;
		FEMaterial* ShadowMapMaterialInstanced;

		int UniformBufferCount = 0;
		const int UBufferForLightSize = 128;
		GLuint UniformBufferForLights;
		const int UBufferForDirectionalLightSize = 384;
		GLuint UniformBufferForDirectionalLight;

		// in current version only shadows from one directional light is supported.
		FETexture* CSM0 = nullptr;
		FETexture* CSM1 = nullptr;
		FETexture* CSM2 = nullptr;
		FETexture* CSM3 = nullptr;

		// Instanced lines
		FEShader* InstancedLineShader = nullptr;
		std::vector<FELine> LinesBuffer;
		int LineCounter = 0;
		GLuint InstancedLineVAO = 0;
		GLenum InstancedLineBuffer = 0;

		glm::dvec3 MouseRay = glm::dvec3(0.0);

		FEShader* ShaderToForce = nullptr;
		void ForceShader(FEShader* Shader);

		// *********** GPU Culling ***********
		FEShader* FrustumCullingShader = nullptr;
		FEShader* ComputeTextureCopy = nullptr;
		FEShader* ComputeDepthPyramidDownSample = nullptr;

		GLuint FrustumInfoBuffer = 0;
		GLuint CullingLODCountersBuffer = 0;

		void UpdateGPUCullingFrustum();
		void GPUCulling(FEEntity* EntityWithInstancedComponent, FEGameModelComponent& GameModelComponent, size_t PrefabIndex = 0);
		void GPUCullingIndividual(FEEntity* EntityWithInstancedComponent, FEGameModelComponent& GameModelComponent, size_t BufferIndex);

		bool bUseOcclusionCulling = true;
		// *********** GPU Culling END ***********

		std::unordered_map<std::string, std::function<FETexture* ()>> DebugOutputTextures;

		bool bSimplifiedRendering = false;
		void Init();

		// *********** VR Rendering ***********

		bool bClearActiveInSimplifiedRendering = true;

		// *********** VR Rendering END ***********

		std::vector<std::function<void()>> AfterRenderCallbacks;

		// FIX ME! Temporary
		std::string LastRenderedSceneID;
		std::string LastRenderedCameraID;
		FEEntity* TryToGetLastUsedCameraEntity();

		std::unordered_map<std::string, FECameraRenderingData*> CameraRenderingDataMap;
		FECameraRenderingData* CreateCameraRenderingData(FEEntity* CameraEntity);
		void OnResizeCameraRenderingDataUpdate(FEEntity* CameraEntity);
		
		FECameraRenderingData* CurrentCameraRenderingData = nullptr;
		void UpdateShadersForCamera(FECameraRenderingData* CameraData);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetRenderer();
	#define RENDERER (*static_cast<FERenderer*>(GetRenderer()))
#else
	#define RENDERER FERenderer::GetInstance()
#endif
}

#endif // FERENDERER_H