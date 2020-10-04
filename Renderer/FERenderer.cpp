#include "FERenderer.h"
using namespace FocalEngine;

FERenderer* FERenderer::_instance = nullptr;

FERenderer::FERenderer()
{
	glGenBuffers(1, &uniformBufferForLights);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferForLights);
	glBufferData(GL_UNIFORM_BUFFER, FE_MAX_LIGHTS * UBufferForLightSize, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, uniformBufferCount++, uniformBufferForLights, 0, FE_MAX_LIGHTS * UBufferForLightSize);

	glGenBuffers(1, &uniformBufferForDirectionalLight);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferForDirectionalLight);
	glBufferData(GL_UNIFORM_BUFFER, UBufferForDirectionalLightSize, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, uniformBufferCount++, uniformBufferForDirectionalLight, 0, UBufferForDirectionalLightSize);

	// Instanced lines
	linesBuffer.resize(maxLines);

	float quadVertices[] = {
		0.0f,  -0.5f,  0.0f,
		1.0f,  -0.5f,  1.0f,
		1.0f,  0.5f,   1.0f,

		0.0f,  -0.5f,  0.0f,
		1.0f,  0.5f,   1.0f,
		0.0f,  0.5f,   0.0f,
	};
	glGenVertexArrays(1, &instancedLineVAO);
	glBindVertexArray(instancedLineVAO);

	unsigned int quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glGenBuffers(1, &instancedLineBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, instancedLineBuffer);
	glBufferData(GL_ARRAY_BUFFER, linesBuffer.size() * sizeof(FELine), linesBuffer.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)0);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(6 * sizeof(float)));
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(FELine), (void*)(9 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);

	glBindVertexArray(0);
}

void FERenderer::standardFBInit(int WindowWidth, int WindowHeight)
{
	sceneToTextureFB = FEResourceManager::getInstance().createFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, WindowWidth, WindowHeight);
}

void FERenderer::loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEMaterial* material, FETransformComponent* transform, bool isReceivingShadows)
{
	static int FEWorldMatrix_hash = std::hash<std::string>{}("FEWorldMatrix");
	static int FEViewMatrix_hash = std::hash<std::string>{}("FEViewMatrix");
	static int FEProjectionMatrix_hash = std::hash<std::string>{}("FEProjectionMatrix");
	static int FEPVMMatrix_hash = std::hash<std::string>{}("FEPVMMatrix");
	static int FECameraPosition_hash = std::hash<std::string>{}("FECameraPosition");
	static int FEGamma_hash = std::hash<std::string>{}("FEGamma");
	static int FEExposure_hash = std::hash<std::string>{}("FEExposure");
	static int FEReceiveShadows_hash = std::hash<std::string>{}("FEReceiveShadows");
	static int FEAOMapPresent_hash = std::hash<std::string>{}("FEAOMapPresent");
	static int FEAOIntensity_hash = std::hash<std::string>{}("FEAOIntensity");
	static int FEAOMapIntensity_hash = std::hash<std::string>{}("FEAOMapIntensity");
	static int FENormalMapPresent_hash = std::hash<std::string>{}("FENormalMapPresent");
	static int FENormalMapIntensity_hash = std::hash<std::string>{}("FENormalMapIntensity");
	static int FERoughtness_hash = std::hash<std::string>{}("FERoughtness");
	static int FERoughtnessMapPresent_hash = std::hash<std::string>{}("FERoughtnessMapPresent");
	static int FERoughtnessMapIntensity_hash = std::hash<std::string>{}("FERoughtnessMapIntensity");
	static int FEMetalness_hash = std::hash<std::string>{}("FEMetalness");
	static int FEMetalnessMapPresent_hash = std::hash<std::string>{}("FEMetalnessMapPresent");
	static int FEMetalnessMapIntensity_hash = std::hash<std::string>{}("FEMetalnessMapIntensity");
	static int FEMRAOMapPresent_hash = std::hash<std::string>{}("FEMRAOMapPresent");
	FEScene& scene = FEScene::getInstance();

	//auto start = std::chrono::system_clock::now();
	auto iterator = shader->parameters.begin();
	while (iterator != shader->parameters.end())
	{
		if (iterator->second.nameHash == FEWorldMatrix_hash)
			iterator->second.updateData(transform->getTransformMatrix());

		if (iterator->second.nameHash == FEViewMatrix_hash)
			iterator->second.updateData(currentCamera->getViewMatrix());

		if (iterator->second.nameHash == FEProjectionMatrix_hash)
			iterator->second.updateData(currentCamera->getProjectionMatrix());

		if (iterator->second.nameHash == FEPVMMatrix_hash)
			iterator->second.updateData(currentCamera->getProjectionMatrix() * currentCamera->getViewMatrix() * transform->getTransformMatrix());

		if (iterator->second.nameHash == FECameraPosition_hash)
			iterator->second.updateData(currentCamera->getPosition());

		if (iterator->second.nameHash == FEGamma_hash)
			iterator->second.updateData(currentCamera->getGamma());

		if (iterator->second.nameHash == FEExposure_hash)
			iterator->second.updateData(currentCamera->getExposure());

		if (iterator->second.nameHash == FEReceiveShadows_hash)
			iterator->second.updateData(isReceivingShadows);

		if (iterator->second.nameHash == FEAOIntensity_hash)
			iterator->second.updateData(material->getAmbientOcclusionIntensity());

		if (iterator->second.nameHash == FENormalMapIntensity_hash)
			iterator->second.updateData(material->getNormalMapIntensity());

		if (iterator->second.nameHash == FENormalMapPresent_hash)
			iterator->second.updateData(material->normalMap == nullptr ? 0.0f : 1.0f);

		if (iterator->second.nameHash == FEAOMapPresent_hash)
			iterator->second.updateData(material->AOMap == nullptr ? 0.0f : 1.0f);

		if (iterator->second.nameHash == FEAOMapIntensity_hash)
			iterator->second.updateData(material->getAmbientOcclusionMapIntensity());

		if (iterator->second.nameHash == FERoughtnessMapPresent_hash)
			iterator->second.updateData(material->roughtnessMap == nullptr ? 0.0f : 1.0f);
		
		if (iterator->second.nameHash == FERoughtness_hash)
			iterator->second.updateData(material->roughtness);

		if (iterator->second.nameHash == FERoughtnessMapIntensity_hash)
			iterator->second.updateData(material->getRoughtnessMapIntensity());

		if (iterator->second.nameHash == FEMetalness_hash)
			iterator->second.updateData(material->metalness);

		if (iterator->second.nameHash == FEMetalnessMapPresent_hash)
			iterator->second.updateData(material->metalnessMap == nullptr ? 0.0f : 1.0f);

		if (iterator->second.nameHash == FEMetalnessMapIntensity_hash)
			iterator->second.updateData(material->getMetalnessMapIntensity());

		if (iterator->second.nameHash == FEMRAOMapPresent_hash)
			iterator->second.updateData(material->MRAOMap == nullptr ? 0.0f : 1.0f);
		
		iterator++;
	}

	//auto end = std::chrono::system_clock::now();
	////auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
	//double eTime = std::chrono::duration_cast<std::chrono::duration<double>>(end - start).count() * 1000.0;
	//if (eTime > 0.02)
	//{
	//	LOG.logError(std::to_string(eTime));
	//	float time = 1;
	//	time += 1;
	//}
}

void FERenderer::addPostProcess(FEPostProcess* newPostProcess, bool noProcessing)
{
	postProcessEffects.push_back(newPostProcess);

	if (noProcessing)
		return;

	for (size_t i = 0; i < postProcessEffects.back()->stages.size(); i++)
	{
		postProcessEffects.back()->stages[i]->inTexture.resize(postProcessEffects.back()->stages[i]->inTextureSource.size());
		//to-do: change when out texture could be different resolution or/and format.
		//#fix
		if (i == postProcessEffects.back()->stages.size() - 1)
		{
			postProcessEffects.back()->stages[i]->outTexture = FEResourceManager::getInstance().createSameFormatTexture(sceneToTextureFB->getColorAttachment());
		}
		else
		{
			int finalW = postProcessEffects.back()->screenWidth;
			int finalH = postProcessEffects.back()->screenHeight;
			postProcessEffects.back()->stages[i]->outTexture = FEResourceManager::getInstance().createSameFormatTexture(sceneToTextureFB->getColorAttachment(), finalW, finalH);
		}
	}
}

void FERenderer::loadUniformBlocks()
{
	FEScene& scene = FEScene::getInstance();
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	std::vector<FELightShaderInfo> info;
	info.resize(FE_MAX_LIGHTS);

	// direction light information for shaders
	FEDirectionalLightShaderInfo directionalLightInfo;

	auto lightIterator = scene.lightsMap.begin();
	int index = 0;
	while (lightIterator != scene.lightsMap.end())
	{
		if (lightIterator->second->getType() == FE_DIRECTIONAL_LIGHT)
		{
			FEDirectionalLight* light = reinterpret_cast<FEDirectionalLight*>(lightIterator->second);
				
			directionalLightInfo.position = glm::vec4(light->transform.getPosition(), 0.0f);
			directionalLightInfo.color = glm::vec4(light->getColor() * light->getIntensity(), 0.0f);
			directionalLightInfo.direction = glm::vec4(light->getDirection(), 0.0f);
			directionalLightInfo.CSM0 = light->cascadeData[0].projectionMat * light->cascadeData[0].viewMat;
			directionalLightInfo.CSM1 = light->cascadeData[1].projectionMat * light->cascadeData[1].viewMat;
			directionalLightInfo.CSM2 = light->cascadeData[2].projectionMat * light->cascadeData[2].viewMat;
			directionalLightInfo.CSM3 = light->cascadeData[3].projectionMat * light->cascadeData[3].viewMat;
			directionalLightInfo.CSMSizes = glm::vec4(light->cascadeData[0].size, light->cascadeData[1].size, light->cascadeData[2].size, light->cascadeData[3].size);
			directionalLightInfo.activeCascades = light->activeCascades;
			directionalLightInfo.biasFixed = light->shadowBias;
			if (!light->staticShadowBias)
				directionalLightInfo.biasFixed = -1.0f;
			directionalLightInfo.biasVariableIntensity = light->shadowBiasVariableIntensity;
			directionalLightInfo.intensity = light->getIntensity();
		}
		else if (lightIterator->second->getType() == FE_SPOT_LIGHT)
		{
			info[index].typeAndAngles = glm::vec3(lightIterator->second->getType(),
												  glm::cos(glm::radians(reinterpret_cast<FESpotLight*>(lightIterator->second)->getSpotAngle())),
												  glm::cos(glm::radians(reinterpret_cast<FESpotLight*>(lightIterator->second)->getSpotAngleOuter())));
		
			info[index].direction = glm::vec4(reinterpret_cast<FESpotLight*>(lightIterator->second)->getDirection(), 0.0f);
		}
		else if (lightIterator->second->getType() == FE_POINT_LIGHT)
		{
			info[index].typeAndAngles = glm::vec3(lightIterator->second->getType(), 0.0f, 0.0f);
		}

		info[index].position = glm::vec4(lightIterator->second->transform.getPosition(), 0.0f);
		info[index].color = glm::vec4(lightIterator->second->getColor() * lightIterator->second->getIntensity(), 0.0f);

		index++;
		lightIterator++;
	}

	//#fix only standardShaders uniforms buffers are filled.
	static int lightInfo_hash = std::hash<std::string>{}("lightInfo");
	static int directionalLightInfo_hash = std::hash<std::string>{}("directionalLightInfo");
	std::vector<std::string> shaderList = resourceManager.getStandardShadersList();
	for (size_t i = 0; i < shaderList.size(); i++)
	{
		FEShader* shader = resourceManager.getShader(shaderList[i]);
		auto iteratorBlock = shader->blockUniforms.begin();
		while (iteratorBlock != shader->blockUniforms.end())
		{
			if (iteratorBlock->first == lightInfo_hash)
			{
				// if shader uniform block was not asigned yet.
				if (iteratorBlock->second == size_t(-1))
					iteratorBlock->second = uniformBufferForLights;
				// adding 4 because vec3 in shader buffer will occupy 16 bytes not 12.
				size_t sizeOfFELightShaderInfo = sizeof(FELightShaderInfo) + 4;
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, iteratorBlock->second));
				auto lightIterator = scene.lightsMap.begin();
				int index = 0;
				while (lightIterator != scene.lightsMap.end())
				{
					FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, index * sizeOfFELightShaderInfo + 0, 16, &info[index].typeAndAngles));
					FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, index * sizeOfFELightShaderInfo + 16, 16, &info[index].position));
					FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, index * sizeOfFELightShaderInfo + 32, 16, &info[index].color));
					FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, index * sizeOfFELightShaderInfo + 48, 16, &info[index].direction));
					FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, index * sizeOfFELightShaderInfo + 64, 64, &info[index].lightSpace));

					index++;
					lightIterator++;
				}
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			}
			else if (iteratorBlock->first == directionalLightInfo_hash)
			{
				// if shader uniform block was not asigned yet.
				if (iteratorBlock->second == size_t(-1))
					iteratorBlock->second = uniformBufferForDirectionalLight;

				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, iteratorBlock->second));
				
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, &directionalLightInfo.position));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 16, 16, &directionalLightInfo.color));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 32, 16, &directionalLightInfo.direction));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 48, 64, &directionalLightInfo.CSM0));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 112, 64, &directionalLightInfo.CSM1));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 176, 64, &directionalLightInfo.CSM2));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 240, 64, &directionalLightInfo.CSM3));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 304, 64, &directionalLightInfo.CSMSizes));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 368, 4, &directionalLightInfo.activeCascades));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 372, 4, &directionalLightInfo.biasFixed));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 376, 4, &directionalLightInfo.biasVariableIntensity));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 380, 4, &directionalLightInfo.intensity));

				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			}

			iteratorBlock++;
		}
	}
}

void FERenderer::render(FEBasicCamera* currentCamera)
{
	FEScene& scene = FEScene::getInstance();

	// there is only 1 directional light, sun.
	// and we need to set correct light position
	//#fix it should update view matries for each cascade!
	auto itLight = scene.lightsMap.begin();
	while (itLight != scene.lightsMap.end())
	{
		if (itLight->second->isCastShadows())
		{
			if (itLight->second->getType() == FE_DIRECTIONAL_LIGHT)
			{
				FEDirectionalLight* light = reinterpret_cast<FEDirectionalLight*>(itLight->second);
				
				light->updateCascades(currentCamera->fov, currentCamera->aspectRatio,
									  currentCamera->nearPlane, currentCamera->farPlane, currentCamera->viewMatrix);
			}
		}

		itLight++;
	}
	
	loadUniformBlocks();
	
	// ********* GENERATE SHADOW MAPS *********
	itLight = scene.lightsMap.begin();
	while (itLight != scene.lightsMap.end())
	{
		if (itLight->second->isCastShadows())
		{
			if (itLight->second->getType() == FE_DIRECTIONAL_LIGHT)
			{
				FEDirectionalLight* light = reinterpret_cast<FEDirectionalLight*>(itLight->second);

				glm::vec3 oldCameraPosition = currentCamera->getPosition();
				glm::mat4 oldViewMatrix = currentCamera->getViewMatrix();
				glm::mat4 oldProjectionMatrix = currentCamera->getProjectionMatrix();

				for (size_t i = 0; i < size_t(light->activeCascades); i++)
				{
					// put camera to the position of light
					currentCamera->projectionMatrix = light->cascadeData[i].projectionMat;
					currentCamera->viewMatrix = light->cascadeData[i].viewMat;

					FE_GL_ERROR(glViewport(0, 0, light->cascadeData[i].frameBuffer->getWidth(), light->cascadeData[i].frameBuffer->getHeight()));

					light->cascadeData[i].frameBuffer->bind();
					FE_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));

					auto itTerrain = scene.terrainMap.begin();
					while (itTerrain != scene.terrainMap.end())
					{
						auto terrain = itTerrain->second;
						if (!terrain->isCastShadows() || !terrain->isVisible())
						{
							itTerrain++;
							continue;
						}
						
						terrain->shader = FEResourceManager::getInstance().getShader("FESMTerrainShader");
						renderTerrain(terrain, currentCamera);
						terrain->shader = FEResourceManager::getInstance().getShader("FETerrainShader");
						itTerrain++;
					}

					auto it = scene.entityMap.begin();
					while (it != scene.entityMap.end())
					{
						auto entity = it->second;
						if (!entity->isCastShadows() || !entity->isVisible())
						{
							it++;
							continue;
						}

						FEMaterial* originalMaterial = entity->gameModel->material;
						entity->gameModel->material = shadowMapMaterial;
						//#fix do it only if albedoHasAlpha
						shadowMapMaterial->albedoMap = originalMaterial->albedoMap;

						renderEntity(entity, currentCamera);

						shadowMapMaterial->albedoMap = nullptr;
						entity->gameModel->material = originalMaterial;

						it++;
					}

					light->cascadeData[i].frameBuffer->unBind();
					switch (i)
					{
						case 0: CSM0 = light->cascadeData[i].frameBuffer->getDepthAttachment();
							break;
						case 1: CSM1 = light->cascadeData[i].frameBuffer->getDepthAttachment();
							break;
						case 2: CSM2 = light->cascadeData[i].frameBuffer->getDepthAttachment();
							break;
						case 3: CSM3 = light->cascadeData[i].frameBuffer->getDepthAttachment();
							break;
						default:
							break;
					}
					
				}

				currentCamera->setPosition(oldCameraPosition);
				currentCamera->viewMatrix = oldViewMatrix;
				currentCamera->projectionMatrix = oldProjectionMatrix;

				FE_GL_ERROR(glViewport(0, 0, sceneToTextureFB->getWidth(), sceneToTextureFB->getHeight()));
				break;
			}
		}

		itLight++;
	}

	// ********* GENERATE SHADOW MAPS END *********
	
	// in current version only shadows from one directional light is supported.
	if (CSM0) CSM0->bind(FE_CSM_UNIT);
	if (CSM1) CSM1->bind(FE_CSM_UNIT + 1);
	if (CSM2) CSM2->bind(FE_CSM_UNIT + 2);
	if (CSM3) CSM3->bind(FE_CSM_UNIT + 3);

	// ********* RENDER SCENE *********
	sceneToTextureFB->bind();
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// ********* RENDER INSTANCED LINE *********
	FE_GL_ERROR(glDisable(GL_CULL_FACE));

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedLineBuffer));
	FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, maxLines * sizeof(FELine), this->linesBuffer.data()));
	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, 0));

	instancedLineShader->start();
	instancedLineShader->getParameter("FEProjectionMatrix")->updateData(currentCamera->getProjectionMatrix());
	instancedLineShader->getParameter("FEViewMatrix")->updateData(currentCamera->getViewMatrix());
	instancedLineShader->getParameter("resolution")->updateData(glm::vec2(sceneToTextureFB->getWidth(), sceneToTextureFB->getHeight()));
	instancedLineShader->loadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(instancedLineVAO));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glEnableVertexAttribArray(1));
	FE_GL_ERROR(glEnableVertexAttribArray(2));
	FE_GL_ERROR(glEnableVertexAttribArray(3));
	FE_GL_ERROR(glEnableVertexAttribArray(4));
	FE_GL_ERROR(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, lineCounter));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glDisableVertexAttribArray(1));
	FE_GL_ERROR(glDisableVertexAttribArray(2));
	FE_GL_ERROR(glDisableVertexAttribArray(3));
	FE_GL_ERROR(glDisableVertexAttribArray(4));
	FE_GL_ERROR(glBindVertexArray(0));
	instancedLineShader->stop();

	//FE_GL_ERROR(glEnable(GL_CULL_FACE));
	//FE_GL_ERROR(glCullFace(GL_BACK));
	// ********* RENDER INSTANCED LINE END *********

	auto itTerrain = scene.terrainMap.begin();
	while (itTerrain != scene.terrainMap.end())
	{
		auto terrain = itTerrain->second;
		if (terrain->isVisible())
			renderTerrain(terrain, currentCamera);
		
		itTerrain++;
	}

	auto it = scene.entityMap.begin();
	while (it != scene.entityMap.end())
	{
		auto entity = it->second;
		
		if (entity->isVisible())
			renderEntity(entity, currentCamera);

		it++;
	}
	
	sceneToTextureFB->unBind();
	// ********* RENDER SCENE END *********

	//Generate the mipmaps of colorAttachment
	sceneToTextureFB->getColorAttachment()->bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// ********* POST_PROCESS EFFECTS *********
	FETexture* prevStageTex = sceneToTextureFB->getColorAttachment();

	for (size_t i = 0; i < postProcessEffects.size(); i++)
	{
		FEPostProcess& effect = *postProcessEffects[i];
		for (size_t j = 0; j < effect.stages.size(); j++)
		{
			effect.stages[j]->shader->start();
			loadStandardParams(effect.stages[j]->shader, currentCamera, nullptr, nullptr);
			for (size_t k = 0; k < effect.stages[j]->stageSpecificUniforms.size(); k++)
			{
				FEShaderParam* param = effect.stages[j]->shader->getParameter(effect.stages[j]->stageSpecificUniforms[k].getName());
				if (param != nullptr)
				{
					param->updateData(effect.stages[j]->stageSpecificUniforms[k].data);
				}
			}

			effect.stages[j]->shader->loadDataToGPU();

			for (size_t k = 0; k < effect.stages[j]->inTextureSource.size(); k++)
			{
				if (effect.stages[j]->inTextureSource[k] == FEPP_PREVIOUS_STAGE_RESULT0)
				{
					effect.stages[j]->inTexture[k] = prevStageTex;
					effect.stages[j]->inTexture[k]->bind(k);
				}
				else if (effect.stages[j]->inTextureSource[k] == FEPP_SCENE_HDR_COLOR)
				{
					effect.stages[j]->inTexture[k] = sceneToTextureFB->getColorAttachment();
					effect.stages[j]->inTexture[k]->bind(k);
				}
				else if (effect.stages[j]->inTextureSource[k] == FEPP_SCENE_DEPTH)
				{
					effect.stages[j]->inTexture[k] = sceneToTextureFB->getDepthAttachment();
					effect.stages[j]->inTexture[k]->bind(k);
				}
				else if (effect.stages[j]->inTextureSource[k] == FEPP_OWN_TEXTURE)
				{
					effect.stages[j]->inTexture[k]->bind(k);
				}
			}

			effect.intermediateFramebuffer->setColorAttachment(effect.stages[j]->outTexture);
			if (effect.stages[j]->outTexture->width != sceneToTextureFB->getWidth())
			{
				FE_GL_ERROR(glViewport(0, 0, effect.stages[j]->outTexture->width, effect.stages[j]->outTexture->height));
			}
			else
			{
				FE_GL_ERROR(glViewport(0, 0, sceneToTextureFB->getWidth(), sceneToTextureFB->getHeight()));
			}
			
			effect.intermediateFramebuffer->bind();

			FE_GL_ERROR(glBindVertexArray(effect.screenQuad->getVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, effect.screenQuad->getVertexCount(), GL_UNSIGNED_INT, 0));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			effect.intermediateFramebuffer->unBind();

			for (size_t k = 0; k < effect.stages[j]->inTextureSource.size(); k++)
			{
				effect.stages[j]->inTexture[k]->unBind();
			}
			effect.stages[j]->shader->stop();

			prevStageTex = effect.stages[j]->outTexture;
		}
	}

	for (int i = postProcessEffects.size() - 1; i >= 0; i--)
	{
		FEPostProcess& effect = *postProcessEffects[i];
		
		if (effect.active)
		{
			effect.renderResult();
			break;
		}
	}
	// ********* SCREEN SPACE EFFECTS END *********

	// **************************** TERRAIN EDITOR TOOLS ****************************
	itTerrain = scene.terrainMap.begin();
	while (itTerrain != scene.terrainMap.end())
	{
		auto terrain = itTerrain->second;
		if (terrain->isVisible())
			updateTerrainBrush(terrain);

		itTerrain++;
	}
	// **************************** TERRAIN EDITOR TOOLS END ****************************

	lineCounter = 0;
}

FEPostProcess* FERenderer::getPostProcessEffect(std::string name)
{
	for (size_t i = 0; i < postProcessEffects.size(); i++)
	{
		if (postProcessEffects[i]->getName() == name)
			return postProcessEffects[i];
	}

	return nullptr;
}

std::vector<std::string> FERenderer::getPostProcessList()
{
	std::vector<std::string> result;
	for (size_t i = 0; i < postProcessEffects.size(); i++)
		result.push_back(postProcessEffects[i]->getName());
		
	return result;
}

void FERenderer::takeScreenshot(const char* fileName, int width, int height)
{
	char* pixels = new char[4 * width * height];
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, postProcessEffects.back()->stages.back()->outTexture->getTextureID()));
	FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
	
	FETexture* tempTexture = FEResourceManager::getInstance().rawDataToFETexture(pixels, width, height);
	FEResourceManager::getInstance().saveFETexture(tempTexture, fileName);
	FEResourceManager::getInstance().deleteFETexture(tempTexture);

	//FEResourceManager::getInstance().saveFETexture(fileName, pixels, width, height);
	delete[] pixels;
}

void FERenderer::renderEntity(FEEntity* entity, FEBasicCamera* currentCamera, bool reloadUniformBlocks)
{
	if (reloadUniformBlocks)
		loadUniformBlocks();

	entity->gameModel->material->bind();
	loadStandardParams(entity->gameModel->material->shader, currentCamera, entity->gameModel->material, &entity->transform, entity->isReceivingShadows());
	entity->gameModel->material->shader->loadDataToGPU();
	entity->render();
	entity->gameModel->material->unBind();
}

void FERenderer::renderTerrain(FETerrain* terrain, FEBasicCamera* currentCamera)
{
	if (terrain->isWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	terrain->heightMap->bind(0);

	if (terrain->layer0->displacementMap != nullptr)
		terrain->layer0->displacementMap->bind(1);

	if (terrain->layer0->albedoMap != nullptr)
		terrain->layer0->albedoMap->bind(2);
	if (terrain->layer0->normalMap != nullptr)
		terrain->layer0->normalMap->bind(3);
	if (terrain->layer0->AOMap != nullptr)
		terrain->layer0->AOMap->bind(4);
	if (terrain->layer0->roughtnessMap != nullptr)
		terrain->layer0->roughtnessMap->bind(5);
	if (terrain->layer0->metalnessMap != nullptr)
		terrain->layer0->metalnessMap->bind(6);

	if (terrain->projectedMap != nullptr)
		terrain->projectedMap->bind(7);

	terrain->shader->start();

	//loadStandardTerrainParams(terrain->shader, currentCamera, terrain);
	loadStandardParams(terrain->shader, currentCamera, terrain->layer0, &terrain->transform, terrain->isReceivingShadows());

	terrain->shader->getParameter("hightScale")->updateData(terrain->hightScale);
	terrain->shader->getParameter("displacementScale")->updateData(terrain->displacementScale);
	terrain->shader->getParameter("scaleFactor")->updateData(terrain->scaleFactor);
	terrain->shader->getParameter("tileMult")->updateData(terrain->tileMult);
	terrain->shader->getParameter("LODlevel")->updateData(terrain->LODlevel);
	terrain->shader->getParameter("hightMapShift")->updateData(terrain->hightMapShift);

	static glm::vec3 pivotPosition = terrain->transform.getPosition();
	pivotPosition = terrain->transform.getPosition();
	terrain->scaleFactor = 1.0f * terrain->chunkPerSide;

	static int PVMHash = std::hash<std::string>{}("FEPVMMatrix");
	static int WMHash = std::hash<std::string>{}("FEWorldMatrix");
	static int HShiftHash = std::hash<std::string>{}("hightMapShift");

	bool wasDirty = terrain->transform.dirtyFlag;
	terrain->shader->loadDataToGPU();
	for (size_t i = 0; i < terrain->chunkPerSide; i++)
	{
		for (size_t j = 0; j < terrain->chunkPerSide; j++)
		{
			terrain->transform.setPosition(glm::vec3(pivotPosition.x + i * 64.0f * terrain->transform.scale[0], pivotPosition.y, pivotPosition.z + j * 64.0f * terrain->transform.scale[2]));

			terrain->shader->getParameter("FEPVMMatrix")->updateData(currentCamera->getProjectionMatrix() * currentCamera->getViewMatrix() * terrain->transform.getTransformMatrix());
			if (terrain->shader->getParameter("FEWorldMatrix") != nullptr)
				terrain->shader->getParameter("FEWorldMatrix")->updateData(terrain->transform.getTransformMatrix());
			terrain->shader->getParameter("hightMapShift")->updateData(glm::vec2(i * -1.0f, j * -1.0f));

			terrain->shader->loadMatrix(PVMHash, *(glm::mat4*)terrain->shader->getParameter("FEPVMMatrix")->data);
			if (terrain->shader->getParameter("FEWorldMatrix") != nullptr)
				terrain->shader->loadMatrix(WMHash, *(glm::mat4*)terrain->shader->getParameter("FEWorldMatrix")->data);
			terrain->shader->loadVector(HShiftHash, *(glm::vec2*)terrain->shader->getParameter("hightMapShift")->data);
			terrain->render();
		}
	}
	terrain->shader->stop();
	terrain->transform.setPosition(pivotPosition);

	if (!wasDirty)
		terrain->transform.dirtyFlag = false;

	if (terrain->isWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void FERenderer::drawLine(glm::vec3 beginPoint, glm::vec3 endPoint, glm::vec3 color, float width)
{
	if (lineCounter >= maxLines)
		return;

	linesBuffer[lineCounter].begin = beginPoint;
	linesBuffer[lineCounter].end = endPoint;
	linesBuffer[lineCounter].color = color;
	linesBuffer[lineCounter].width = width;

	lineCounter++;
}

void FERenderer::updateTerrainBrush(FETerrain* terrain)
{
	terrain->updateBrush(engineMainCamera->position, mouseRay);
	FE_GL_ERROR(glViewport(0, 0, sceneToTextureFB->getWidth(), sceneToTextureFB->getHeight()));
}