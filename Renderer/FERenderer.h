#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../SubSystems/FEScene.h"

namespace FocalEngine
{
	#define FE_MAX_LINES 100000

	class FEngine;
#ifdef USE_DEFERRED_RENDERER
	class FERenderer;

	class FEGBuffer
	{
		friend FERenderer;

		void initializeResources(FEFramebuffer* mainFrameBuffer);
	public:
		FEGBuffer(FEFramebuffer* mainFrameBuffer);
		
		FEFramebuffer* GFrameBuffer = nullptr;

		FETexture* positions = nullptr;
		FETexture* normals = nullptr;
		FETexture* albedo = nullptr;
		FETexture* materialProperties = nullptr;
		FETexture* shaderProperties = nullptr;

		void renderTargetResize(FEFramebuffer* mainFrameBuffer);
	};
	
#endif // USE_DEFERRED_RENDERER
	
	class FERenderer
	{
		friend FEngine;
	public:
		SINGLETON_PUBLIC_PART(FERenderer)

		void render(FEBasicCamera* currentCamera);
		void renderEntity(FEEntity* entity, FEBasicCamera* currentCamera, bool reloadUniformBlocks = false, int componentIndex = -1);
		void renderEntityForward(FEEntity* entity, FEBasicCamera* currentCamera, bool reloadUniformBlocks = false);
		void renderEntityInstanced(FEEntityInstanced* entityInstanced, FEBasicCamera* currentCamera, float** frustum, bool shadowMap = false, bool reloadUniformBlocks = false, int componentIndex = -1);
		void renderTerrain(FETerrain* terrain, FEBasicCamera* currentCamera);
		void addPostProcess(FEPostProcess* newPostProcess, bool noProcessing = false);

		std::vector<std::string> getPostProcessList();
		FEPostProcess* getPostProcessEffect(std::string ID);

		//#fix postProcess
		FEFramebuffer* sceneToTextureFB = nullptr;
		FETexture* finalScene = nullptr;
		std::vector<FEPostProcess*> postProcessEffects;

		// in current version only shadows from one directional light is supported.
		FETexture* CSM0 = nullptr;
		FETexture* CSM1 = nullptr;
		FETexture* CSM2 = nullptr;
		FETexture* CSM3 = nullptr;

		void drawLine(glm::vec3 beginPoint, glm::vec3 endPoint, glm::vec3 color = glm::vec3(1.0f), float width = 0.1f);
		// *********** Anti-Aliasing(FXAA) ***********
		float getFXAASpanMax();
		void setFXAASpanMax(float newValue);

		float getFXAAReduceMin();
		void setFXAAReduceMin(float newValue);

		float getFXAAReduceMul();
		void setFXAAReduceMul(float newValue);

		// *********** Bloom ***********
		float getBloomThreshold();
		void setBloomThreshold(float newValue);

		float getBloomSize();
		void setBloomSize(float newValue);

		// *********** Depth of Field ***********
		float getDOFNearDistance();
		void setDOFNearDistance(float newValue);

		float getDOFFarDistance();
		void setDOFFarDistance(float newValue);

		float getDOFStrength();
		void setDOFStrength(float newValue);

		float getDOFDistanceDependentStrength();
		void setDOFDistanceDependentStrength(float newValue);

		// *********** Chromatic Aberration ***********
		float getChromaticAberrationIntensity();
		void setChromaticAberrationIntensity(float newValue);

		// *********** Distance fog ***********
		bool isDistanceFogEnabled();
		void setDistanceFogEnabled(bool newValue);

		float getDistanceFogDensity();
		void setDistanceFogDensity(float newValue);

		float getDistanceFogGradient();
		void setDistanceFogGradient(float newValue);

		// *********** Sky ***********
		bool isSkyEnabled();
		void setSkyEnabld(bool newValue);

		float getDistanceToSky();
		void setDistanceToSky(float newValue);

		float testTime = 0.0f;
		float lastTestTime = 0.0f;

		bool freezeCulling = false;
		void drawAABB(FEAABB AABB, glm::vec3 color = glm::vec3(0.1f, 0.6f, 0.1f), float lineWidth = 0.2f);

#ifdef USE_DEFERRED_RENDERER
		FEGBuffer* GBuffer = nullptr;
		FEFramebuffer* SSAOFB = nullptr;
#endif // USE_DEFERRED_RENDERER
		
	private:
		SINGLETON_PRIVATE_PART(FERenderer)
		void loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEMaterial* material, FETransformComponent* transform, bool isReceivingShadows = false);
		void loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, bool isReceivingShadows);
		void loadUniformBlocks();

		void standardFBInit(int WindowWidth, int WindowHeight);
		void takeScreenshot(const char* fileName, int width, int height);

		FEMaterial* shadowMapMaterial;
		FEMaterial* shadowMapMaterialInstanced;

		int uniformBufferCount = 0;
		const int UBufferForLightSize = 128;
		GLuint uniformBufferForLights;
		const int UBufferForDirectionalLightSize = 384;
		GLuint uniformBufferForDirectionalLight;

		// Instanced lines
		FEShader* instancedLineShader = nullptr;
		std::vector<FELine> linesBuffer;
		int lineCounter = 0;
		GLuint instancedLineVAO = 0;
		GLenum instancedLineBuffer = 0;

		FEBasicCamera* engineMainCamera = nullptr;
		glm::dvec3 mouseRay = glm::dvec3(0.0);

		void updateTerrainBrush(FETerrain* terrain);

		FEEntity* skyDome = nullptr;

		float distanceFogDensity = 0.007f;
		float distanceFogGradient = 2.5f;
		bool distanceFogEnabled = false;
		void updateFogInShaders();

		FEShader* shaderToForce = nullptr;
		void forceShader(FEShader* shader);

		// *********** GPU Culling ***********
		FEShader* FE_FrustumCullingShader = nullptr;

		GLuint frustumInfoBuffer = 0;
		GLuint cullingLODCountersBuffer = 0;

		void updateGPUCullingFrustum(float** frustum, glm::vec3 cameraPosition);
		void GPUCulling(FEEntityInstanced* entity, int subGameModel);
		// *********** GPU Culling END ***********
	};

	#define RENDERER FERenderer::getInstance()
}

#endif FERENDERER_H