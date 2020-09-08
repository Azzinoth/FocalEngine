#pragma once

#ifndef FERENDERER_H
#define FERENDERER_H

#include "../SubSystems/FEScene.h"

namespace FocalEngine
{
	class FEngine;

	class FERenderer
	{
		friend FEngine;
	public:
		SINGLETON_PUBLIC_PART(FERenderer)

		void render(FEBasicCamera* currentCamera);
		void renderEntity(FEEntity* entity, FEBasicCamera* currentCamera, bool reloadUniformBlocks = false);
		void renderTerrain(FETerrain* terrain, FEBasicCamera* currentCamera);
		void addPostProcess(FEPostProcess* newPostProcess, bool noProcessing = false);

		std::vector<std::string> getPostProcessList();
		FEPostProcess* getPostProcessEffect(std::string name);

		//#fix postProcess
		FEFramebuffer* sceneToTextureFB = nullptr;
		std::vector<FEPostProcess*> postProcessEffects;

		// in current version only shadows from one directional light is supported.
		FETexture* CSM0 = nullptr;
		FETexture* CSM1 = nullptr;
		FETexture* CSM2 = nullptr;
		FETexture* CSM3 = nullptr;

		void drawLine(glm::vec3 beginPoint, glm::vec3 endPoint, glm::vec3 color = glm::vec3(1.0f), float width = 0.1f);
	private:
		SINGLETON_PRIVATE_PART(FERenderer)
		void loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEMaterial* material, FETransformComponent* transform, bool isReceivingShadows = false);
		void loadUniformBlocks();

		void standardFBInit(int WindowWidth, int WindowHeight);
		void takeScreenshot(const char* fileName, int width, int height);

		FEMaterial* shadowMapMaterial;

		int uniformBufferCount = 0;
		//const int UBufferForLightSize = 192;
		const int UBufferForLightSize = 128;
		GLuint uniformBufferForLights;
		const int UBufferForDirectionalLightSize = 384;
		GLuint uniformBufferForDirectionalLight;

		// Instanced lines
		FEShader* instancedLineShader = nullptr;
		std::vector<FELine> linesBuffer;
		int maxLines = 10000;
		int lineCounter = 0;
		GLuint instancedLineVAO = 0;
		GLenum instancedLineBuffer = 0;

		FEBasicCamera* engineMainCamera = nullptr;
		glm::dvec3 mouseRay = glm::dvec3(0.0);

		void updateTerrainBrush(FETerrain* terrain);
	};
}

#endif FERENDERER_H