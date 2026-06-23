#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../SubSystems/Scene/Components/Systems/FEComponentSystems.h"
#include "../SubSystems/FEInput.h"

namespace FocalEngine
{
	#define FE_MAX_DEBUG_LINES 50000

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
		FETexture* MotionVectors = nullptr;

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

	// TO-DO: This is very non GPU memory efficient way to support multiple cameras.
	struct FECameraRenderingData
	{
		friend class FERenderer;

		FEEntity* CameraEntity = nullptr;
		FEFramebuffer* SceneToTextureFB = nullptr;
		FEGBuffer* GBuffer = nullptr;
		FESSAO* SSAO = nullptr;
		FETexture* DepthPyramid = nullptr;
		std::vector<FEPostProcess*> PostProcessEffects;

		// Variables for compute shader based point cloud rendering.
		FEFramebuffer* PointCloudIntermediateFrameBuffer = nullptr;
		GLuint PointCloud64bitFrameBuffer = -1;
		bool IsAdvancedPointCloudRenderingInitialized();

		FETexture* FinalScene = nullptr;
		bool bTemporaryForceHDROutput = false;

		// Scratch scene color copy for read-while-write passes (Translucent/Additive, Volumetric) to avoid sampling their own render target.
		FETexture* SceneColorScratchTexture = nullptr;
		FETexture* CurrentSceneColorSourceTexture = nullptr;

		~FECameraRenderingData()
		{
			delete SceneToTextureFB;
			delete GBuffer;
			delete SSAO;
			delete DepthPyramid;
			delete SceneColorScratchTexture;
			delete PointCloudIntermediateFrameBuffer;

			if (PointCloud64bitFrameBuffer != GLuint (-1))
				FE_GL_ERROR(glDeleteBuffers(1, &PointCloud64bitFrameBuffer));
		}
	};

	struct FEDebugLineDirtyFlag
	{
		bool bCurrentlyInUse = false;
		bool bSeenStartFrame = false;
		bool bSeenEndFrame = false;
	};

	struct EntityBasedEngineProvidedData
	{
		glm::mat4 WorldMatrix = glm::mat4(1.0f);
	};

	struct CameraBasedEngineProvidedData
	{
		glm::mat4 ViewMatrix = glm::mat4(1.0f);
		glm::mat4 InverseViewMatrix = glm::mat4(1.0f);
		glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 InverseProjectionMatrix = glm::mat4(1.0f);

		float NearPlane = 0.1f;
		float FarPlane = 1000.0f;
		glm::vec3 CameraPosition = glm::vec3(0.0f);
		glm::vec3 CameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);

		float Gamma = 2.2f;
		float Exposure = 1.0f;
	};
	
	class FOCAL_ENGINE_API FERenderer
	{
		friend FEngine;
		friend FECameraSystem;
		friend FEShader;
		friend FENewMaterial;
	public:
		SINGLETON_PUBLIC_PART(FERenderer)

		void Render(FEScene* CurrentScene);
		
		void RenderGameModelComponent(FEEntity* Entity, FEEntity* Camera, bool bReloadUniformBlocks = false);
		void RenderGameModelComponent(FEGameModelComponent& GameModelComponent, FETransformComponent& TransformComponent, FEScene* ParentScene, FEEntity* Camera, bool bReloadUniformBlocks = false);
		void RenderGameModelComponentForward(FEEntity* Entity, FEEntity* Camera, bool bReloadUniformBlocks = false);
		void RenderGameModelComponentWithInstanced(FEEntity* Entity, FEEntity* Camera, bool bShadowMap = false, bool bReloadUniformBlocks = false, size_t PrefabIndex = 0);
		void RenderTerrainComponent(FEEntity* TerrainEntity, FEEntity* Camera);
		
		FETexture* GetCameraResult(FEEntity* CameraEntity);

		std::vector<FELine> GetFrustumLines(FEEntity* Camera, glm::vec3 Color = glm::vec3(0.3f, 0.6f, 0.1f), float LineWidth = 0.2f);

		void DebugDrawLine(glm::vec3 StartPoint, glm::vec3 EndPoint, glm::vec3 Color = glm::vec3(1.0f), float Width = 0.1f);
		void DebugDrawLine(FELine LineToRender);
		void DebugDrawAABB(FEAABB AABB, glm::vec3 Color = glm::vec3(0.1f, 0.6f, 0.1f), float LineWidth = 0.2f);
		void DebugDrawFrustum(FEEntity* Camera, glm::vec3 Color = glm::vec3(0.3f, 0.6f, 0.1f), float LineWidth = 0.2f);

		float TestTime = 0.0f;
		float LastTestTime = 0.0f;

		bool bFreezeCulling = false;

		bool IsOcclusionCullingEnabled();
		void SetOcclusionCullingEnabled(bool NewValue);

		void UpdateSSAO(FEEntity* Camera);

		std::unordered_map<std::string, std::function<FETexture* ()>> GetDebugOutputTextures();
		void SimplifiedRender(FEScene* CurrentScene, FEEntity* MainCameraEntity, FECameraRenderingData* CurrentCameraRenderingData);

		void RenderToFrameBuffer(FETexture* SceneTexture, FEFramebuffer* Target);
		void RenderToFrameBuffer(FETexture* SceneTexture, GLuint Target);

		bool CombineFrameBuffers(FEFramebuffer* FirstSource, FEFramebuffer* SecondSource, FEFramebuffer* Target);

		void AddCameraPostRenderCallback(std::string CameraEntityID, std::function<void(FEEntity* CameraEntity, FETexture* RenderResult)> Callback);

		void SetGLViewport(int X, int Y, int Width, int Height);
		void SetGLViewport(glm::ivec4 ViewPortData);
		glm::ivec4 GetGLViewport();

		FECameraRenderingData* GetCameraRenderingData(FEEntity* CameraEntity);
		void AddPostProcess(FECameraRenderingData* CameraRenderingData, FEPostProcess* NewPostProcess, const bool bNoProcessing = false);

		// TO-DO: This function is not ready yet. The main problem is that fused data would not be fed into postprocess.
		bool FuseSceneRenderings(FEEntity* FirstSceneCamera, FEEntity* SecondSceneCamera, FEFramebuffer* ResultingFrameBuffer);
		bool FuseFrameBufferDataAndCameraData(FETexture* SourceColor, FETexture* SourceDepth, FEEntity* TargetCamera,
											  float SourceNearPlane, float SourceFarPlane,
											  glm::vec3 NormalsToWrite, glm::vec4 MaterialPropertiesToWrite,
											  glm::vec4 ShaderPropertiesToWrite, glm::vec2 MotionVectorsToWrite);
		bool FuseFrameBufferDataAndCameraData(FEFramebuffer* Source, FEEntity* TargetCamera,
											  float SourceNearPlane, float SourceFarPlane,
											  glm::vec3 NormalsToWrite, glm::vec4 MaterialPropertiesToWrite,
											  glm::vec4 ShaderPropertiesToWrite, glm::vec2 MotionVectorsToWrite);
		bool FuseTwoFrameBuffers(FEFramebuffer* FirstSource, float FirstNearPlane, float FirstFarPlane,
								 FEFramebuffer* SecondSource, float SecondNearPlane, float SecondFarPlane, FEFramebuffer* Target);
		bool FuseSceneRenderings(FEEntity* FirstSceneCamera, FEEntity* SecondSceneCamera, FEEntity* CameraToPutResultIn);

		FETexture* CreateScreenshot(FEScene* Scene);
		FETexture* CreateScreenshot(FEEntity* CameraEntity);

		// FIX ME: Is this system good solution?
		void AddBeforeRenderCallback(FEEntity* Entity, std::function<void(FEEntity*)> Callback);
		void RemoveBeforeRenderCallback(FEEntity* Entity, std::function<void(FEEntity*)> Callback);
	private:
		SINGLETON_PRIVATE_PART(FERenderer)

		void RenderInternal(FEScene* CurrentScene, FEEntity* MainCameraEntity, FECameraRenderingData* CurrentCameraRenderingData);

		void LoadStandardUniforms(FEShader* Shader, FEMaterial* Material, FETransformComponent* Transform, FEEntity* Camera, bool bReceivingShadows = false, const bool IsUniformLighting = false);
		void LoadStandardUniforms(FEShader* Shader, bool bReceivingShadows, FEEntity* Camera, const bool bUniformLighting = false);
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

		// Debug lines
		std::vector<FELine> DebugLines;
		std::vector<FEDebugLineDirtyFlag> DebugLinesDirtyFlags;
		void BeginFrameDebugLines();
		void EndFrameDebugLines();
		int DebugLineCounter = 0;
		GLuint DebugLinesVAO = 0;
		GLenum DebugLinesBuffer = 0;
		void RenderDebugLines(FEScene* CurrentScene, FEEntity* MainCameraEntity, FECameraRenderingData* CurrentCameraRenderingData);

		glm::dvec3 MouseRay = glm::dvec3(0.0);

		FEShader* ShaderToForce = nullptr;
		void ForceShader(FEShader* Shader);

		// *********** GPU Culling ***********
		FEShader* FrustumCullingShader = nullptr;
		FEShader* ComputeTextureCopy = nullptr;
		FEShader* ComputeDepthPyramidDownSample = nullptr;

		GLuint FrustumInfoBuffer = 0;
		GLuint CullingLODCountersBuffer = 0;

		void UpdateGPUCullingFrustum(FEEntity* Camera);
		void GPUCulling(FEEntity* EntityWithInstancedComponent, FEGameModelComponent& GameModelComponent, FEEntity* Camera, size_t PrefabIndex = 0);
		void GPUCullingIndividual(FEEntity* EntityWithInstancedComponent, FEGameModelComponent& GameModelComponent, FEEntity* Camera, size_t BufferIndex);

		bool bUseOcclusionCulling = true;
		// *********** GPU Culling END ***********

		std::unordered_map<std::string, std::function<FETexture* ()>> DebugOutputTextures;

		void Init();

		std::unordered_map<std::string, std::vector<std::function<void(FEEntity* CameraEntity, FETexture* RenderResult)>>> CameraPostRenderCallbacks;

		std::unordered_map<std::string, FECameraRenderingData*> CameraRenderingDataMap;
		FECameraRenderingData* CreateCameraRenderingData(FEEntity* CameraEntity);
		void ForceCameraRenderingDataUpdate(FEEntity* CameraEntity);
		
		void UpdateShadersForCamera(FECameraRenderingData* CameraData);

		bool InitializeComputeShaderPointCloudRendering(FEEntity* CameraEntity);

		std::unordered_map<std::string, std::vector<std::function<void(FEEntity*)>>> BeforeRenderCallbacks;

		// *********** New Material System Getters ***********
		FECameraRenderingData* CurrentCameraRenderingData = nullptr;

		void SetEntityForRendering(FEEntity* Entity);
		static const std::unordered_set<std::string>& GetEngineProvidedUniformNames();
		static bool IsEngineProvidedUniform(const std::string& UniformName);

		bool BindEngineProvidedTexture(FEShader* Shader, const std::string& UniformName);

		static EntityBasedEngineProvidedData CurrentEntityBasedEngineProvidedData;
		const EntityBasedEngineProvidedData& GetCurrentEntityBasedEngineProvidedData() const;

		static CameraBasedEngineProvidedData CurrentCameraBasedEngineProvidedData;
		const CameraBasedEngineProvidedData& GetCurrentCameraBasedEngineProvidedData() const;
		// *********** New Material System Getters END ***********
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetRenderer();
	#define RENDERER (*static_cast<FERenderer*>(GetRenderer()))
#else
	#define RENDERER FERenderer::GetInstance()
#endif
}

#endif // FERENDERER_H