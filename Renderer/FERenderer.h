#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../SubSystems/Scene/Components/Systems/FELightSystem.h"
#include "../SubSystems/Scene/Components/Systems/FEInstancedRenderingSystem.h"
#include "../SubSystems/Scene/Components/Systems/FETerrainSystem.h"
#include "../SubSystems/Scene/Components/Systems/FESkyDomeSystem.h"

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

		bool bActive = true;
		int SampleCount = 16;

		bool bSmallDetails = true;
		bool bBlured = true;

		float Bias = 0.013f;
		float Radius = 10.0f;
		float RadiusSmallDetails = 0.4f;
		float SmallDetailsWeight = 0.2f;
	};
	
	class FERenderer
	{
		friend FEngine;
	public:
		SINGLETON_PUBLIC_PART(FERenderer)

		void Render(FEBasicCamera* CurrentCamera);
		// FIX ME! It is closer to what it should be, but still not perfect.
		// It should be done as system, and only ocassionally throught this function.
		void RenderGameModelComponent(FEGameModelComponent& GameModelComponent, FETransformComponent& TransformComponent, const FEBasicCamera* CurrentCamera, bool bReloadUniformBlocks = false);
		void RenderGameModelComponentForward(FEGameModelComponent& GameModelComponent, FETransformComponent& TransformComponent, const FEBasicCamera* CurrentCamera, bool bReloadUniformBlocks = false);
		void RenderGameModelComponentWithInstanced(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, FEBasicCamera* CurrentCamera, float** Frustum, bool bShadowMap = false, bool bReloadUniformBlocks = false);
		void RenderTerrainComponent(FEEntity* TerrainEntity, const FEBasicCamera* CurrentCamera);
		void RenderTerrainComponent(FETransformComponent& TransformComponent, FETerrainComponent& TerrainComponent, const FEBasicCamera* CurrentCamera);
		void AddPostProcess(FEPostProcess* NewPostProcess, bool NoProcessing = false);

		std::vector<std::string> GetPostProcessList();
		FEPostProcess* GetPostProcessEffect(std::string ID);

		//#fix postProcess
		FEFramebuffer* SceneToTextureFB = nullptr;
		FETexture* FinalScene = nullptr;
		std::vector<FEPostProcess*> PostProcessEffects;

		void DrawLine(glm::vec3 BeginPoint, glm::vec3 EndPoint, glm::vec3 Color = glm::vec3(1.0f), float Width = 0.1f);
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

		// *********** Distance fog ***********
		bool IsDistanceFogEnabled();
		void SetDistanceFogEnabled(bool NewValue);

		float GetDistanceFogDensity();
		void SetDistanceFogDensity(float NewValue);

		float GetDistanceFogGradient();
		void SetDistanceFogGradient(float NewValue);

		float TestTime = 0.0f;
		float LastTestTime = 0.0f;

		bool bFreezeCulling = false;
		void DrawAABB(FEAABB AABB, glm::vec3 Color = glm::vec3(0.1f, 0.6f, 0.1f), float LineWidth = 0.2f);

		bool IsOcclusionCullingEnabled();
		void SetOcclusionCullingEnabled(bool NewValue);

		// *********** SSAO ***********
		bool IsSSAOEnabled();
		void SetSSAOEnabled(bool NewValue);

		int GetSSAOSampleCount();
		void SetSSAOSampleCount(int NewValue);

		bool IsSSAOSmallDetailsEnabled();
		void SetSSAOSmallDetailsEnabled(bool NewValue);

		bool IsSSAOResultBlured();
		void SetSSAOResultBlured(bool NewValue);

		float GetSSAOBias();
		void SetSSAOBias(float NewValue);

		float GetSSAORadius();
		void SetSSAORadius(float NewValue);

		float GetSSAORadiusSmallDetails();
		void SetSSAORadiusSmallDetails(float NewValue);

		float GetSSAOSmallDetailsWeight();
		void SetSSAOSmallDetailsWeight(float NewValue);

		FEGBuffer* GBuffer = nullptr;
		FESSAO* SSAO;
		void UpdateSSAO(const FEBasicCamera* CurrentCamera);

		std::unordered_map<std::string, std::function<FETexture* ()>> GetDebugOutputTextures();
		void SimplifiedRender(FEBasicCamera* CurrentCamera);

		// *********** VR Rendering ***********

		bool bVRActive = false;
		void InitVR(int VRScreenW, int VRScreenH);
		int VRScreenW = 0;
		int VRScreenH = 0;
		FEFramebuffer* SceneToVRTextureFB = nullptr;
		void RenderVR(FEBasicCamera* CurrentCamera/*, uint32_t ColorTexture, uint32_t DepthTexture*/);

		void UpdateVRRenderTargetSize(int VRScreenW, int VRScreenH);

		// *********** VR Rendering END ***********

		void RenderToFrameBuffer(FETexture* SceneTexture, FEFramebuffer* Target);
		void RenderToFrameBuffer(FETexture* SceneTexture, GLuint Target);

		bool CombineFrameBuffers(FEFramebuffer* FirstSource, FEFramebuffer* SecondSource, FEFramebuffer* Target);

		void AddAfterRenderCallback(std::function<void()> Callback);
		FEFramebuffer* GetLastRenderedResult();

		bool IsClearActiveInSimplifiedRendering();
		void SetClearActiveInSimplifiedRendering(bool NewValue);
	private:
		SINGLETON_PRIVATE_PART(FERenderer)

		void LoadStandardParams(FEShader* Shader, const FEBasicCamera* CurrentCamera, FEMaterial* Material, const FETransformComponent* Transform, bool IsReceivingShadows = false, const bool IsUniformLighting = false);
		void LoadStandardParams(FEShader* Shader, const FEBasicCamera* CurrentCamera, bool IsReceivingShadows, const bool IsUniformLighting = false);
		void LoadUniformBlocks();

		void StandardFBInit(int WindowWidth, int WindowHeight);
		void TakeScreenshot(const char* FileName, int Width, int Height);

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

		FEBasicCamera* EngineMainCamera = nullptr;
		glm::dvec3 MouseRay = glm::dvec3(0.0);

		float DistanceFogDensity = 0.007f;
		float DistanceFogGradient = 2.5f;
		bool bDistanceFogEnabled = false;
		void UpdateFogInShaders();

		FEShader* ShaderToForce = nullptr;
		void ForceShader(FEShader* Shader);

		// *********** GPU Culling ***********
		FEShader* FrustumCullingShader = nullptr;
		FEShader* ComputeTextureCopy = nullptr;
		FEShader* ComputeDepthPyramidDownSample = nullptr;

		GLuint FrustumInfoBuffer = 0;
		GLuint CullingLODCountersBuffer = 0;

		void UpdateGPUCullingFrustum(float** Frustum, glm::vec3 CameraPosition);
		void GPUCulling(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const FEBasicCamera* CurrentCamera);

		FETexture* DepthPyramid = nullptr;
		bool bUseOcclusionCulling = true;
		// *********** GPU Culling END ***********

		std::unordered_map<std::string, std::function<FETexture* ()>> DebugOutputTextures;

		void RenderTargetResize(int NewWidth, int NewHeight);

		bool bSimplifiedRendering = false;
		void Init();

		// *********** VR Rendering ***********

		bool bClearActiveInSimplifiedRendering = true;

		// *********** VR Rendering END ***********

		FEFramebuffer* LastRenderedResult = nullptr;
		std::vector<std::function<void()>> AfterRenderCallbacks;
	};

	#define RENDERER FERenderer::getInstance()
}

#endif // FERENDERER_H