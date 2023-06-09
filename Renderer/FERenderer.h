#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../SubSystems/FEScene.h"

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
		void RenderEntity(const FEEntity* Entity, const FEBasicCamera* CurrentCamera, bool bReloadUniformBlocks = false, int ComponentIndex = -1);
		void RenderEntityForward(const FEEntity* Entity, const FEBasicCamera* CurrentCamera, bool bReloadUniformBlocks = false);
		void RenderEntityInstanced(FEEntityInstanced* EntityInstanced, FEBasicCamera* CurrentCamera, float** Frustum, bool bShadowMap = false, bool bReloadUniformBlocks = false, int ComponentIndex = -1);
		void RenderTerrain(FETerrain* Terrain, const FEBasicCamera* CurrentCamera);
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

		// *********** Sky ***********
		bool IsSkyEnabled();
		void SetSkyEnabld(bool NewValue);

		float GetDistanceToSky();
		void SetDistanceToSky(float NewValue);

		float TestTime = 0.0f;
		float LastTestTime = 0.0f;

		bool bFreezeCulling = false;
		void DrawAABB(FEAABB AABB, glm::vec3 Color = glm::vec3(0.1f, 0.6f, 0.1f), float LineWidth = 0.2f);

		bool IsOccusionCullingEnabled();
		void SetOccusionCullingEnabled(bool NewValue);

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
	private:
		SINGLETON_PRIVATE_PART(FERenderer)
		// Temporary DELETE !
	public:
		void LoadStandardParams(FEShader* Shader, const FEBasicCamera* CurrentCamera, FEMaterial* Material, const FETransformComponent* Transform, bool IsReceivingShadows = false);
		void LoadStandardParams(FEShader* Shader, const FEBasicCamera* CurrentCamera, bool IsReceivingShadows);
	private:
		// Temporary DELETE !
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

		void UpdateTerrainBrush(FETerrain* Terrain);

		FEEntity* SkyDome = nullptr;

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
		void GPUCulling(FEEntityInstanced* Entity, int SubGameModel, const FEBasicCamera* CurrentCamera);

		FETexture* DepthPyramid = nullptr;
		bool bUseOccusionCulling = true;
		// *********** GPU Culling END ***********

		std::unordered_map<std::string, std::function<FETexture* ()>> DebugOutputTextures;

		void RenderTargetResize(int NewWidth, int NewHeight);

		bool bSimplifiedRendering = false;
		void Init();
	};

	#define RENDERER FERenderer::getInstance()
}

#endif FERENDERER_H