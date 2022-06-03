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
	linesBuffer.resize(FE_MAX_LINES);

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

	skyDome = RESOURCE_MANAGER.createEntity(RESOURCE_MANAGER.getGameModel("17271E603508013IO77931TY"/*"skyDomeGameModel"*/), "skyDomeEntity");
	RESOURCE_MANAGER.makePrefabStandard(skyDome->prefab);
	skyDome->visible = false;
	skyDome->transform.setScale(glm::vec3(50.0f));

	FE_FrustumCullingShader = RESOURCE_MANAGER.createShader("FE_FrustumCullingShader", nullptr, nullptr,
																									   nullptr, nullptr,
																									   nullptr, RESOURCE_MANAGER.loadGLSL("CoreExtensions//ComputeShaders//FE_FrustumCulling_CS.glsl").c_str());

	FE_GL_ERROR(glGenBuffers(1, &frustumInfoBuffer));
	FE_GL_ERROR(glGenBuffers(1, &cullingLODCountersBuffer));

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cullingLODCountersBuffer));
	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * 40, nullptr, GL_DYNAMIC_DRAW));

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, frustumInfoBuffer));

	std::vector<float> frustumData;
	for (size_t i = 0; i < 32; i++)
	{
		frustumData.push_back(0.0);
	}

	FE_GL_ERROR(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float) * (32), frustumData.data(), GL_DYNAMIC_DRAW));
}

void FERenderer::standardFBInit(int WindowWidth, int WindowHeight)
{
	sceneToTextureFB = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, WindowWidth, WindowHeight);

#ifdef USE_DEFERRED_RENDERER
	GBuffer = new FEGBuffer(sceneToTextureFB);
	SSAOFB = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT, WindowWidth, WindowHeight, false);
#endif // USE_DEFERRED_RENDERER

	debugOutputTextures["albedoRenderTarget"] = []() { return RENDERER.GBuffer->albedo; };
	debugOutputTextures["positionsRenderTarget"] = []() { return RENDERER.GBuffer->positions; };
	debugOutputTextures["normalsRenderTarget"] = []() { return RENDERER.GBuffer->normals; };
	debugOutputTextures["materialPropertiesRenderTarget"] = []() { return RENDERER.GBuffer->materialProperties; };
	debugOutputTextures["shaderPropertiesRenderTarget"] = []() { return RENDERER.GBuffer->shaderProperties; };

	debugOutputTextures["CSM0"] = []() { return RENDERER.CSM0; };
	debugOutputTextures["CSM1"] = []() { return RENDERER.CSM1; };
	debugOutputTextures["CSM2"] = []() { return RENDERER.CSM2; };
	debugOutputTextures["CSM3"] = []() { return RENDERER.CSM3; };
}

void FERenderer::loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEMaterial* material, FETransformComponent* transform, bool isReceivingShadows)
{
	static int FEWorldMatrix_hash = int(std::hash<std::string>{}("FEWorldMatrix"));
	static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));
	static int FEPVMMatrix_hash = int(std::hash<std::string>{}("FEPVMMatrix"));
	static int FECameraPosition_hash = int(std::hash<std::string>{}("FECameraPosition"));
	static int FEGamma_hash = int(std::hash<std::string>{}("FEGamma"));
	static int FEExposure_hash = int(std::hash<std::string>{}("FEExposure"));
	static int FEReceiveShadows_hash = int(std::hash<std::string>{}("FEReceiveShadows"));
	static int FEAOIntensity_hash = int(std::hash<std::string>{}("FEAOIntensity"));
	static int FEAOMapIntensity_hash = int(std::hash<std::string>{}("FEAOMapIntensity"));
	static int FENormalMapIntensity_hash = int(std::hash<std::string>{}("FENormalMapIntensity"));
	static int FERoughtness_hash = int(std::hash<std::string>{}("FERoughtness"));
	static int FERoughtnessMapIntensity_hash = int(std::hash<std::string>{}("FERoughtnessMapIntensity"));
	static int FEMetalness_hash = int(std::hash<std::string>{}("FEMetalness"));
	static int FEMetalnessMapIntensity_hash = int(std::hash<std::string>{}("FEMetalnessMapIntensity"));
	static int FETiling_hash = int(std::hash<std::string>{}("FETiling"));

	static int FETextureBindingsUniformLocations_hash = int(std::hash<std::string>{}("textureBindings[0]"));
	static int FETextureChannelsBindingsUniformLocations_hash = int(std::hash<std::string>{}("textureChannels[0]"));

	static int FEcompactMaterialPacking_hash = int(std::hash<std::string>{}("compactMaterialPacking"));
	
	if (material != nullptr)
	{
		if (shader->materialTexturesList)
		{
			shader->loadIntArray(FETextureBindingsUniformLocations_hash, material->textureBindings.data(), material->textureBindings.size());
			shader->loadIntArray(FETextureChannelsBindingsUniformLocations_hash, material->textureChannels.data(), material->textureChannels.size());
		}
	}

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

		if (material != nullptr)
		{
			if (iterator->second.nameHash == FEAOIntensity_hash)
				iterator->second.updateData(material->getAmbientOcclusionIntensity());

			if (iterator->second.nameHash == FENormalMapIntensity_hash)
				iterator->second.updateData(material->getNormalMapIntensity());

			if (iterator->second.nameHash == FEAOMapIntensity_hash)
				iterator->second.updateData(material->getAmbientOcclusionMapIntensity());

			if (iterator->second.nameHash == FERoughtness_hash)
				iterator->second.updateData(material->roughtness);

			if (iterator->second.nameHash == FERoughtnessMapIntensity_hash)
				iterator->second.updateData(material->getRoughtnessMapIntensity());

			if (iterator->second.nameHash == FEMetalness_hash)
				iterator->second.updateData(material->metalness);

			if (iterator->second.nameHash == FEMetalnessMapIntensity_hash)
				iterator->second.updateData(material->getMetalnessMapIntensity());

			if (iterator->second.nameHash == FETiling_hash)
				iterator->second.updateData(material->getTiling());

			if (iterator->second.nameHash == FEcompactMaterialPacking_hash)
				iterator->second.updateData(material->isCompackPacking());
		}
		
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

void FERenderer::loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, bool isReceivingShadows)
{
	static int FEViewMatrix_hash = int(std::hash<std::string>{}("FEViewMatrix"));
	static int FEProjectionMatrix_hash = int(std::hash<std::string>{}("FEProjectionMatrix"));
	static int FECameraPosition_hash = int(std::hash<std::string>{}("FECameraPosition"));
	static int FEGamma_hash = int(std::hash<std::string>{}("FEGamma"));
	static int FEExposure_hash = int(std::hash<std::string>{}("FEExposure"));
	static int FEReceiveShadows_hash = int(std::hash<std::string>{}("FEReceiveShadows"));

	auto iterator = shader->parameters.begin();
	while (iterator != shader->parameters.end())
	{
		if (iterator->second.nameHash == FEViewMatrix_hash)
			iterator->second.updateData(currentCamera->getViewMatrix());

		if (iterator->second.nameHash == FEProjectionMatrix_hash)
			iterator->second.updateData(currentCamera->getProjectionMatrix());

		if (iterator->second.nameHash == FECameraPosition_hash)
			iterator->second.updateData(currentCamera->getPosition());

		if (iterator->second.nameHash == FEGamma_hash)
			iterator->second.updateData(currentCamera->getGamma());

		if (iterator->second.nameHash == FEExposure_hash)
			iterator->second.updateData(currentCamera->getExposure());

		if (iterator->second.nameHash == FEReceiveShadows_hash)
			iterator->second.updateData(isReceivingShadows);

		iterator++;
	}
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
			postProcessEffects.back()->stages[i]->outTexture = RESOURCE_MANAGER.createSameFormatTexture(sceneToTextureFB->getColorAttachment());
		}
		else
		{
			int finalW = postProcessEffects.back()->screenWidth;
			int finalH = postProcessEffects.back()->screenHeight;
			postProcessEffects.back()->stages[i]->outTexture = RESOURCE_MANAGER.createSameFormatTexture(sceneToTextureFB->getColorAttachment(), finalW, finalH);
		}

		postProcessEffects.back()->texturesToDelete.push_back(postProcessEffects.back()->stages[i]->outTexture);
	}
}

void FERenderer::loadUniformBlocks()
{
	FEScene& scene = SCENE;
	FEResourceManager& resourceManager = RESOURCE_MANAGER;

	std::vector<FELightShaderInfo> info;
	info.resize(FE_MAX_LIGHTS);

	// direction light information for shaders
	FEDirectionalLightShaderInfo directionalLightInfo;

	int index = 0;
	auto lightIterator = OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (lightIterator != OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].end())
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

		info[index].position = glm::vec4(light->transform.getPosition(), 0.0f);
		info[index].color = glm::vec4(light->getColor() * light->getIntensity(), 0.0f);

		index++;
		lightIterator++;
	}

	lightIterator = OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].begin();
	while (lightIterator != OBJECT_MANAGER.objectsByType[FE_SPOT_LIGHT].end())
	{
		FESpotLight* light = reinterpret_cast<FESpotLight*>(lightIterator->second);

		info[index].typeAndAngles = glm::vec3(light->getType(),
											  glm::cos(glm::radians(light->getSpotAngle())),
											  glm::cos(glm::radians(light->getSpotAngleOuter())));

		info[index].direction = glm::vec4(light->getDirection(), 0.0f);

		info[index].position = glm::vec4(light->transform.getPosition(), 0.0f);
		info[index].color = glm::vec4(light->getColor() * light->getIntensity(), 0.0f);

		index++;
		lightIterator++;
	}

	lightIterator = OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].begin();
	while (lightIterator != OBJECT_MANAGER.objectsByType[FE_POINT_LIGHT].end())
	{
		FEPointLight* light = reinterpret_cast<FEPointLight*>(lightIterator->second);
		info[index].typeAndAngles = glm::vec3(light->getType(), 0.0f, 0.0f);

		info[index].position = glm::vec4(light->transform.getPosition(), 0.0f);
		info[index].color = glm::vec4(light->getColor() * light->getIntensity(), 0.0f);

		index++;
		lightIterator++;
	}

	//#fix only standardShaders uniforms buffers are filled.
	static int lightInfo_hash = int(std::hash<std::string>{}("lightInfo"));
	static int directionalLightInfo_hash = int(std::hash<std::string>{}("directionalLightInfo"));
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
				if (iteratorBlock->second == GL_INVALID_INDEX)
					iteratorBlock->second = uniformBufferForLights;
				// adding 4 because vec3 in shader buffer will occupy 16 bytes not 12.
				size_t sizeOfFELightShaderInfo = sizeof(FELightShaderInfo) + 4;
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, iteratorBlock->second));

				//FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeOfFELightShaderInfo * info.size(), &info));
				for (size_t j = 0; j < info.size(); j++)
				{
					FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, j * sizeOfFELightShaderInfo, sizeOfFELightShaderInfo, &info[j]));
				}

				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			}
			else if (iteratorBlock->first == directionalLightInfo_hash)
			{
				// if shader uniform block was not asigned yet.
				if (iteratorBlock->second == GL_INVALID_INDEX)
					iteratorBlock->second = uniformBufferForDirectionalLight;

				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, iteratorBlock->second));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, 384, &directionalLightInfo));
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			}

			iteratorBlock++;
		}
	}
}

void FERenderer::renderEntityInstanced(FEEntityInstanced* entityInstanced, FEBasicCamera* currentCamera, float** frustum, bool shadowMap, bool reloadUniformBlocks, int componentIndex)
{
	if (reloadUniformBlocks)
		loadUniformBlocks();

	if (componentIndex != -1)
	{
		GPUCulling(entityInstanced, int(componentIndex));

		FEGameModel* currentGameModel = entityInstanced->prefab->components[componentIndex]->gameModel;

		FEShader* originalShader = currentGameModel->getMaterial()->shader;
		if (originalShader->getName() == "FEPBRShader")
		{
			if (shaderToForce)
			{
				currentGameModel->getMaterial()->shader = shaderToForce;
			}
			else
			{
				currentGameModel->getMaterial()->shader = RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
			}
		}

		currentGameModel->getMaterial()->bind();
		FETransformComponent tempTransform = entityInstanced->transform.combine(entityInstanced->prefab->components[componentIndex]->transform);
		loadStandardParams(currentGameModel->getMaterial()->shader, currentCamera, currentGameModel->material, &tempTransform, entityInstanced->isReceivingShadows());
		currentGameModel->getMaterial()->shader->loadDataToGPU();

		entityInstanced->render(int(componentIndex));

		currentGameModel->getMaterial()->unBind();
		if (originalShader->getName() == "FEPBRShader")
		{
			currentGameModel->getMaterial()->shader = originalShader;
			if (currentGameModel->getBillboardMaterial() != nullptr)
				currentGameModel->getBillboardMaterial()->shader = originalShader;
		}

		// Billboards part
		if (currentGameModel->getBillboardMaterial() != nullptr)
		{
			FEMaterial* regularBillboardMaterial = currentGameModel->getBillboardMaterial();
			if (shadowMap)
			{
				shadowMapMaterialInstanced->setAlbedoMap(regularBillboardMaterial->getAlbedoMap());
				currentGameModel->setBillboardMaterial(shadowMapMaterialInstanced);
			}

			FEShader* originalShader = currentGameModel->getMaterial()->shader;
			if (originalShader->getName() == "FEPBRShader")
			{
				if (shaderToForce)
				{
					currentGameModel->getBillboardMaterial()->shader = shaderToForce;
				}
				else
				{
					currentGameModel->getBillboardMaterial()->shader = RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
				}
			}

			currentGameModel->getBillboardMaterial()->bind();
			loadStandardParams(currentGameModel->getBillboardMaterial()->shader, currentCamera, currentGameModel->getBillboardMaterial(), &entityInstanced->transform, entityInstanced->isReceivingShadows());
			currentGameModel->getBillboardMaterial()->shader->loadDataToGPU();

			entityInstanced->renderOnlyBillbords(currentCamera->getPosition());

			currentGameModel->getBillboardMaterial()->unBind();
			if (originalShader->getName() == "FEPBRShader")
				currentGameModel->getBillboardMaterial()->shader = originalShader;

			if (shadowMap)
			{
				currentGameModel->setBillboardMaterial(regularBillboardMaterial);
			}
		}

		return;
	}

	for (size_t i = 0; i < entityInstanced->prefab->components.size(); i++)
	{
		GPUCulling(entityInstanced, int(i));

		FEGameModel* currentGameModel = entityInstanced->prefab->components[i]->gameModel;

		FEShader* originalShader = currentGameModel->getMaterial()->shader;
		if (originalShader->getName() == "FEPBRShader")
		{
			if (shaderToForce)
			{
				currentGameModel->getMaterial()->shader = shaderToForce;
			}
			else
			{
				currentGameModel->getMaterial()->shader = RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
			}
		}

		currentGameModel->getMaterial()->bind();
		FETransformComponent tempTransform = entityInstanced->transform.combine(entityInstanced->prefab->components[i]->transform);
		loadStandardParams(currentGameModel->getMaterial()->shader, currentCamera, currentGameModel->material, &tempTransform, entityInstanced->isReceivingShadows());
		currentGameModel->getMaterial()->shader->loadDataToGPU();

		entityInstanced->render(int(i));

		currentGameModel->getMaterial()->unBind();
		if (originalShader->getName() == "FEPBRShader")
		{
			currentGameModel->getMaterial()->shader = originalShader;
			if (currentGameModel->getBillboardMaterial() != nullptr)
				currentGameModel->getBillboardMaterial()->shader = originalShader;
		}

		// Billboards part
		if (currentGameModel->getBillboardMaterial() != nullptr)
		{
			FEMaterial* regularBillboardMaterial = currentGameModel->getBillboardMaterial();
			if (shadowMap)
			{
				shadowMapMaterialInstanced->setAlbedoMap(regularBillboardMaterial->getAlbedoMap());
				currentGameModel->setBillboardMaterial(shadowMapMaterialInstanced);
			}

			FEShader* originalShader = currentGameModel->getMaterial()->shader;
			if (originalShader->getName() == "FEPBRShader")
			{
				if (shaderToForce)
				{
					currentGameModel->getBillboardMaterial()->shader = shaderToForce;
				}
				else
				{
					currentGameModel->getBillboardMaterial()->shader = RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
				}
			}

			currentGameModel->getBillboardMaterial()->bind();
			loadStandardParams(currentGameModel->getBillboardMaterial()->shader, currentCamera, currentGameModel->getBillboardMaterial(), &entityInstanced->transform, entityInstanced->isReceivingShadows());
			currentGameModel->getBillboardMaterial()->shader->loadDataToGPU();

			entityInstanced->renderOnlyBillbords(currentCamera->getPosition());

			currentGameModel->getBillboardMaterial()->unBind();
			if (originalShader->getName() == "FEPBRShader")
				currentGameModel->getBillboardMaterial()->shader = originalShader;

			if (shadowMap)
			{
				currentGameModel->setBillboardMaterial(regularBillboardMaterial);
			}
		}
	}
}

void FERenderer::render(FEBasicCamera* currentCamera)
{
	currentCamera->updateFrustumPlanes();

	lastTestTime = testTime;
	testTime = 0.0f;
	FEScene& scene = SCENE;

	// there is only 1 directional light, sun.
	// and we need to set correct light position
	//#fix it should update view matries for each cascade!

	auto lightIterator = OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (lightIterator != OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		FEDirectionalLight* light = reinterpret_cast<FEDirectionalLight*>(lightIterator->second);
		if (light->isCastShadows())
		{
			light->updateCascades(currentCamera->fov, currentCamera->aspectRatio,
								  currentCamera->nearPlane, currentCamera->farPlane, 
								  currentCamera->viewMatrix, currentCamera->getForward(),
								  currentCamera->getRight(), currentCamera->getUp());
		}
		lightIterator++;
	}

	loadUniformBlocks();

	// ********* GENERATE SHADOW MAPS *********
	CSM0 = nullptr;
	CSM1 = nullptr;
	CSM2 = nullptr;
	CSM3 = nullptr;

	FEShader* shaderPBR = RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	FEShader* shaderInstancedPBR = RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
	FEShader* shaderTerrain = RESOURCE_MANAGER.getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);

	auto itLight = OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].begin();
	while (itLight != OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].end())
	{
		FEDirectionalLight* light = reinterpret_cast<FEDirectionalLight*>(itLight->second);
		if (light->isCastShadows())
		{
			float shadowsBlurFactor = light->getShadowBlurFactor();
			shaderPBR->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
			shaderInstancedPBR->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
#ifndef USE_DEFERRED_RENDERER
			shaderTerrain->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
#endif // USE_DEFERRED_RENDERER

			glm::vec3 oldCameraPosition = currentCamera->getPosition();
			glm::mat4 oldViewMatrix = currentCamera->getViewMatrix();
			glm::mat4 oldProjectionMatrix = currentCamera->getProjectionMatrix();

			for (size_t i = 0; i < size_t(light->activeCascades); i++)
			{
				// put camera to the position of light
				currentCamera->projectionMatrix = light->cascadeData[i].projectionMat;
				currentCamera->viewMatrix = light->cascadeData[i].viewMat;

				FE_GL_ERROR(glViewport(0, 0, light->cascadeData[i].frameBuffer->getWidth(), light->cascadeData[i].frameBuffer->getHeight()));

				updateGPUCullingFrustum(light->cascadeData[i].frustum, currentCamera->getPosition());

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
						
					terrain->shader = RESOURCE_MANAGER.getShader("50064D3C4D0B537F0846274F"/*"FESMTerrainShader"*/);
					renderTerrain(terrain, currentCamera);
					terrain->shader = RESOURCE_MANAGER.getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);
					itTerrain++;
				}

				auto it = scene.entityMap.begin();
				while (it != scene.entityMap.end())
				{
					auto entity = it->second;
					if (!entity->isCastShadows() || !entity->isVisible() || entity->prefab == nullptr)
					{
						it++;
						continue;
					}

					if (entity->getType() == FE_ENTITY)
					{
						for (size_t j = 0; j < entity->prefab->components.size(); j++)
						{
							FEMaterial* originalMaterial = entity->prefab->components[j]->gameModel->material;
							entity->prefab->components[j]->gameModel->material = shadowMapMaterial;
							shadowMapMaterial->setAlbedoMap(originalMaterial->getAlbedoMap());
							// if material have submaterial
							if (originalMaterial->getAlbedoMap(1) != nullptr)
							{
								shadowMapMaterial->setAlbedoMap(originalMaterial->getAlbedoMap(1), 1);
								shadowMapMaterial->getAlbedoMap(1)->bind(1);
							}

							renderEntity(entity, currentCamera, false, int(j));

							entity->prefab->components[j]->gameModel->material = originalMaterial;
							for (size_t k = 0; k < shadowMapMaterial->textures.size(); k++)
							{
								shadowMapMaterial->textures[k] = nullptr;
								shadowMapMaterial->textureBindings[k] = -1;

								shadowMapMaterialInstanced->textures[k] = nullptr;
								shadowMapMaterialInstanced->textureBindings[k] = -1;
							}
						}
					}
					else if (entity->getType() == FE_ENTITY_INSTANCED)
					{
						std::vector<FEMaterial*> originalMaterials;
						FEEntityInstanced* currentEntity = reinterpret_cast<FEEntityInstanced*>(entity);
						for (size_t j = 0; j < currentEntity->prefab->components.size(); j++)
						{
							originalMaterials.push_back(currentEntity->prefab->components[j]->gameModel->material);

							currentEntity->prefab->components[j]->gameModel->material = shadowMapMaterialInstanced;
							shadowMapMaterialInstanced->setAlbedoMap(originalMaterials.back()->getAlbedoMap());
							// if material have submaterial
							if (originalMaterials.back()->getAlbedoMap(1) != nullptr)
							{
								shadowMapMaterialInstanced->setAlbedoMap(originalMaterials.back()->getAlbedoMap(1), 1);
								shadowMapMaterialInstanced->getAlbedoMap(1)->bind(1);
							}
							
							renderEntityInstanced(currentEntity, currentCamera, light->cascadeData[i].frustum, true, false, int(j));
							
							entity->prefab->components[j]->gameModel->material = originalMaterials[j];
							for (size_t k = 0; k < shadowMapMaterial->textures.size(); k++)
							{
								shadowMapMaterial->textures[k] = nullptr;
								shadowMapMaterial->textureBindings[k] = -1;

								shadowMapMaterialInstanced->textures[k] = nullptr;
								shadowMapMaterialInstanced->textureBindings[k] = -1;
							}
						}
					}
						
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

		itLight++;
	}
	// ********* GENERATE SHADOW MAPS END *********
	
	// in current version only shadows from one directional light is supported.
	if (OBJECT_MANAGER.objectsByType[FE_DIRECTIONAL_LIGHT].size() != 0)
	{
		if (CSM0) CSM0->bind(FE_CSM_UNIT);
		if (CSM1) CSM1->bind(FE_CSM_UNIT + 1);
		if (CSM2) CSM2->bind(FE_CSM_UNIT + 2);
		if (CSM3) CSM3->bind(FE_CSM_UNIT + 3);
	}

	// ********* RENDER SCENE *********
#ifdef USE_DEFERRED_RENDERER

	GBuffer->GFrameBuffer->bind();

	unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
	glDrawBuffers(6, attachments);

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	updateGPUCullingFrustum(currentCamera->frustum, currentCamera->getPosition());

	auto entityIterator = scene.entityMap.begin();
	while (entityIterator != scene.entityMap.end())
	{
		auto entity = entityIterator->second;

		if (entity->isVisible() && entity->isPostprocessApplied())
		{
			if (entity->getType() == FE_ENTITY)
			{
				forceShader(RESOURCE_MANAGER.getShader("670B01496E202658377A4576"/*"FEPBRGBufferShader"*/));
				renderEntity(entity, currentCamera);
			}
			else if (entity->getType() == FE_ENTITY_INSTANCED)
			{
				forceShader(RESOURCE_MANAGER.getShader("613830232E12602D6A1D2C17"/*"FEPBRInstancedGBufferShader"*/));
				renderEntityInstanced(reinterpret_cast<FEEntityInstanced*>(entity), currentCamera, currentCamera->getFrustumPlanes(), false);
			}
		}

		entityIterator++;
	}

	auto itTerrain = scene.terrainMap.begin();
	while (itTerrain != scene.terrainMap.end())
	{
		auto terrain = itTerrain->second;
		if (terrain->isVisible())
			renderTerrain(terrain, currentCamera);

		itTerrain++;
	}

	GBuffer->GFrameBuffer->unBind();
	forceShader(nullptr);
#else
	sceneToTextureFB->bind();

	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// ********* RENDER SKY *********
	if (isSkyEnabled())
		renderEntity(skyDome, currentCamera);
#endif // USE_DEFERRED_RENDERER
	


#ifdef USE_DEFERRED_RENDERER
	
	// ************************************ SSAO ************************************
#ifdef USE_SSAO
	SSAOFB->bind();
	FEShader* SSAOShader = RESOURCE_MANAGER.getShader("1037115B676E383E36345079"/*"FESSAOShader"*/);
	SSAOShader->start();
	loadStandardParams(SSAOShader, currentCamera, true);
	SSAOShader->loadDataToGPU();
#endif // USE_SSAO
	GBuffer->albedo->bind(0);
	GBuffer->normals->bind(1);
	GBuffer->materialProperties->bind(2);
	GBuffer->positions->bind(3);
	GBuffer->shaderProperties->bind(4);
#ifdef USE_SSAO
	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/)->getVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/)->getVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	SSAOShader->stop();

	// First blur stage
	FEShader* BlurShader = RESOURCE_MANAGER.getShader("7F3E4F5C130B537F0846274F"/*"FEBloomBlur"*/);
	BlurShader->start();
	BlurShader->getParameter("FEBlurDirection")->updateData(glm::vec2(0.0f, 1.0f));
	BlurShader->getParameter("BloomSize")->updateData(1.5f);

	BlurShader->loadDataToGPU();

	SSAOFB->getColorAttachment()->bind(0);
	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/)->getVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/)->getVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	// Second blur stage
	BlurShader->getParameter("FEBlurDirection")->updateData(glm::vec2(1.0f, 0.0f));
	BlurShader->getParameter("BloomSize")->updateData(1.5f);

	BlurShader->loadDataToGPU();

	SSAOFB->getColorAttachment()->bind(0);
	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/)->getVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/)->getVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	BlurShader->stop();

	SSAOFB->getColorAttachment()->unBind();
	SSAOFB->unBind();

	GBuffer->albedo->bind(0);
	GBuffer->normals->bind(1);
	GBuffer->materialProperties->bind(2);
	GBuffer->positions->bind(3);
	GBuffer->shaderProperties->bind(4);
	SSAOFB->getColorAttachment()->bind(5);
#endif // USE_SSAO
	// ************************************ SSAO END ************************************

	// ************************************ COPYING DEPTH BUFFER ************************************
	glBindFramebuffer(GL_READ_FRAMEBUFFER, GBuffer->GFrameBuffer->fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sceneToTextureFB->fbo); // write to default framebuffer
	glBlitFramebuffer(0, 0, sceneToTextureFB->getWidth(), sceneToTextureFB->getHeight(),
		0, 0, sceneToTextureFB->getWidth(), sceneToTextureFB->getHeight(),
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	// ************************************ COPYING DEPTH BUFFER END ************************************

	sceneToTextureFB->bind();

	unsigned int attachments_[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments_);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);

	FEShader* screenQuadShader = RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
	screenQuadShader->start();
	loadStandardParams(screenQuadShader, currentCamera, true);
	screenQuadShader->loadDataToGPU();

	FE_GL_ERROR(glBindVertexArray(RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/)->getVaoID()));
	FE_GL_ERROR(glEnableVertexAttribArray(0));
	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/)->getVertexCount(), GL_UNSIGNED_INT, 0));
	FE_GL_ERROR(glDisableVertexAttribArray(0));
	FE_GL_ERROR(glBindVertexArray(0));

	GBuffer->albedo->unBind();
	GBuffer->normals->unBind();
	GBuffer->materialProperties->unBind();
	GBuffer->positions->unBind();
	GBuffer->shaderProperties->unBind();

	screenQuadShader->stop();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	
#else
	glDepthMask(GL_TRUE);

	auto itTerrain = scene.terrainMap.begin();
	while (itTerrain != scene.terrainMap.end())
	{
		auto terrain = itTerrain->second;
		if (terrain->isVisible())
			renderTerrain(terrain, currentCamera);

		itTerrain++;
	}

	FE_GL_ERROR(glDisable(GL_CULL_FACE));

	auto entityIterator = scene.entityMap.begin();
	while (entityIterator != scene.entityMap.end())
	{
		auto entity = entityIterator->second;

		if (entity->isVisible())
		{
			if (entity->getType() == FE_ENTITY)
			{
				renderEntity(entity, currentCamera);
			}
			else if (entity->getType() == FE_ENTITY_INSTANCED)
			{
				renderEntityInstanced(reinterpret_cast<FEEntityInstanced*>(entity), currentCamera, currentCamera->getFrustumPlanes(), false);
			}
		}

		entityIterator++;
	}

#endif // USE_DEFERRED_RENDERER

	// ********* RENDER INSTANCED LINE *********
	//FE_GL_ERROR(glDisable(GL_CULL_FACE));

	FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, instancedLineBuffer));
	FE_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, 0, FE_MAX_LINES * sizeof(FELine), this->linesBuffer.data()));
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

	/*FE_GL_ERROR(glEnable(GL_CULL_FACE));
	FE_GL_ERROR(glCullFace(GL_BACK));*/
	// ********* RENDER INSTANCED LINE END *********

	// ********* RENDER SKY *********
	if (isSkyEnabled())
		renderEntity(skyDome, currentCamera);

	sceneToTextureFB->unBind();
	// ********* RENDER SCENE END *********

	//Generate the mipmaps of colorAttachment
	sceneToTextureFB->getColorAttachment()->bind();
	glGenerateMipmap(GL_TEXTURE_2D);
	
	// ********* POST_PROCESS EFFECTS *********
	// Because we render post process effects with screen quad
	// we will turn off write to depth buffer in order to get clear DB to be able to render additional objects
	glDepthMask(GL_FALSE);
	finalScene = sceneToTextureFB->getColorAttachment();
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
					effect.stages[j]->inTexture[k]->bind(int(k));
				}
				else if (effect.stages[j]->inTextureSource[k] == FEPP_SCENE_HDR_COLOR)
				{
					effect.stages[j]->inTexture[k] = sceneToTextureFB->getColorAttachment();
					effect.stages[j]->inTexture[k]->bind(int(k));
				}
				else if (effect.stages[j]->inTextureSource[k] == FEPP_SCENE_DEPTH)
				{
					effect.stages[j]->inTexture[k] = sceneToTextureFB->getDepthAttachment();
					effect.stages[j]->inTexture[k]->bind(int(k));
				}
				else if (effect.stages[j]->inTextureSource[k] == FEPP_OWN_TEXTURE)
				{
					effect.stages[j]->inTexture[k]->bind(int(k));
				}
			}

			FETexture* ordinaryColorAttachment = effect.intermediateFramebuffer->getColorAttachment();
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
			// this was added because of postProcesses and how they "manage" colorAttachment of FB
			effect.intermediateFramebuffer->setColorAttachment(ordinaryColorAttachment);

			for (size_t k = 0; k < effect.stages[j]->inTextureSource.size(); k++)
			{
				effect.stages[j]->inTexture[k]->unBind();
			}
			effect.stages[j]->shader->stop();

			prevStageTex = effect.stages[j]->outTexture;
		}
	}

	for (int i = int(postProcessEffects.size() - 1); i >= 0; i--)
	{
		FEPostProcess& effect = *postProcessEffects[i];
		
		if (effect.active)
		{
			effect.renderResult();
			finalScene = effect.stages.back()->outTexture;
			break;
		}
	}

	glDepthMask(GL_TRUE);
	// ********* SCREEN SPACE EFFECTS END *********

	// ********* ENTITIES THAT WILL NOT BE IMPACTED BY POST PROCESS. MAINLY FOR UI *********
	FETexture* originalColorAttachment = sceneToTextureFB->getColorAttachment();
	sceneToTextureFB->setColorAttachment(finalScene);

	sceneToTextureFB->bind();

	entityIterator = scene.entityMap.begin();
	while (entityIterator != scene.entityMap.end())
	{
		auto entity = entityIterator->second;

		if (entity->isVisible() && !entity->isPostprocessApplied())
		{
			if (entity->getType() == FE_ENTITY)
			{
				renderEntity(entity, currentCamera);
			}
			else if (entity->getType() == FE_ENTITY_INSTANCED)
			{
			}
		}

		entityIterator++;
	}
	
	sceneToTextureFB->unBind();
	sceneToTextureFB->setColorAttachment(originalColorAttachment);
	// ********* ENTITIES THAT WILL NOT BE IMPACTED BY POST PROCESS. MAINLY FOR UI END *********

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

FEPostProcess* FERenderer::getPostProcessEffect(std::string ID)
{
	for (size_t i = 0; i < postProcessEffects.size(); i++)
	{
		if (postProcessEffects[i]->getObjectID() == ID)
			return postProcessEffects[i];
	}

	return nullptr;
}

std::vector<std::string> FERenderer::getPostProcessList()
{
	std::vector<std::string> result;
	for (size_t i = 0; i < postProcessEffects.size(); i++)
		result.push_back(postProcessEffects[i]->getObjectID());
		
	return result;
}

void FERenderer::takeScreenshot(const char* fileName, int width, int height)
{
	unsigned char* pixels = new unsigned char[4 * width * height];
	FE_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, postProcessEffects.back()->stages.back()->outTexture->getTextureID()));
	FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels));
	
	FETexture* tempTexture = RESOURCE_MANAGER.rawDataToFETexture(pixels, width, height);
	RESOURCE_MANAGER.saveFETexture(tempTexture, fileName);
	RESOURCE_MANAGER.deleteFETexture(tempTexture);

	//RESOURCE_MANAGER.saveFETexture(fileName, pixels, width, height);
	delete[] pixels;
}

void FERenderer::renderEntity(FEEntity* entity, FEBasicCamera* currentCamera, bool reloadUniformBlocks, int componentIndex)
{
	if (reloadUniformBlocks)
		loadUniformBlocks();

	if (componentIndex == -1)
	{
		for (size_t i = 0; i < entity->prefab->components.size(); i++)
		{
			FEShader* originalShader = entity->prefab->components[i]->gameModel->material->shader;
			if (shaderToForce)
			{
				if (originalShader->getName() == "FEPBRShader")
					entity->prefab->components[i]->gameModel->material->shader = shaderToForce;
			}

			entity->prefab->components[i]->gameModel->material->bind();
			FETransformComponent tempTransform = entity->transform.combine(entity->prefab->components[i]->transform);
			loadStandardParams(entity->prefab->components[i]->gameModel->material->shader, currentCamera, entity->prefab->components[i]->gameModel->material, &tempTransform, entity->isReceivingShadows());
			entity->prefab->components[i]->gameModel->material->shader->loadDataToGPU();

			FE_GL_ERROR(glBindVertexArray(entity->prefab->components[i]->gameModel->mesh->getVaoID()));
			if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
			if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
			if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
			if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
			if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
			if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

			if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
				FE_GL_ERROR(glDrawElements(GL_TRIANGLES, entity->prefab->components[i]->gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
			if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
				FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, entity->prefab->components[i]->gameModel->mesh->getVertexCount()));

			FE_GL_ERROR(glBindVertexArray(0));

			entity->prefab->components[i]->gameModel->material->unBind();

			if (shaderToForce)
			{
				if (originalShader->getName() == "FEPBRShader")
					entity->prefab->components[i]->gameModel->material->shader = originalShader;
			}
		}
	}
	else
	{
		FEShader* originalShader = entity->prefab->components[componentIndex]->gameModel->material->shader;
		if (shaderToForce)
		{
			if (originalShader->getName() == "FEPBRShader")
				entity->prefab->components[componentIndex]->gameModel->material->shader = shaderToForce;
		}

		entity->prefab->components[componentIndex]->gameModel->material->bind();
		FETransformComponent tempTransform = entity->transform.combine(entity->prefab->components[componentIndex]->transform);
		loadStandardParams(entity->prefab->components[componentIndex]->gameModel->material->shader, currentCamera, entity->prefab->components[componentIndex]->gameModel->material, &tempTransform, entity->isReceivingShadows());
		entity->prefab->components[componentIndex]->gameModel->material->shader->loadDataToGPU();

		FE_GL_ERROR(glBindVertexArray(entity->prefab->components[componentIndex]->gameModel->mesh->getVaoID()));
		if ((entity->prefab->components[componentIndex]->gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((entity->prefab->components[componentIndex]->gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((entity->prefab->components[componentIndex]->gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((entity->prefab->components[componentIndex]->gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((entity->prefab->components[componentIndex]->gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((entity->prefab->components[componentIndex]->gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		if ((entity->prefab->components[componentIndex]->gameModel->mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, entity->prefab->components[componentIndex]->gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
		if ((entity->prefab->components[componentIndex]->gameModel->mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
			FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, entity->prefab->components[componentIndex]->gameModel->mesh->getVertexCount()));

		FE_GL_ERROR(glBindVertexArray(0));

		entity->prefab->components[componentIndex]->gameModel->material->unBind();

		if (shaderToForce)
		{
			if (originalShader->getName() == "FEPBRShader")
				entity->prefab->components[componentIndex]->gameModel->material->shader = originalShader;
		}
	}
}

void FERenderer::renderEntityForward(FEEntity* entity, FEBasicCamera* currentCamera, bool reloadUniformBlocks)
{
	if (reloadUniformBlocks)
		loadUniformBlocks();

	std::vector<FEShader*> originalShaders;

	for (size_t i = 0; i < entity->prefab->components.size(); i++)
	{
		FEShader* originalShader = entity->prefab->components[i]->gameModel->material->shader;
		entity->prefab->components[i]->gameModel->material->shader = RESOURCE_MANAGER.getShader("5E45017E664A62273E191500"/*"FEPBRShaderForward"*/);
			

		entity->prefab->components[i]->gameModel->material->bind();
		loadStandardParams(entity->prefab->components[i]->gameModel->material->shader, currentCamera, entity->prefab->components[i]->gameModel->material, &entity->transform, entity->isReceivingShadows());
		entity->prefab->components[i]->gameModel->material->shader->loadDataToGPU();

		FE_GL_ERROR(glBindVertexArray(entity->prefab->components[i]->gameModel->mesh->getVaoID()));
		if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
		if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
		if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
		if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
		if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
		if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

		if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, entity->prefab->components[i]->gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
		if ((entity->prefab->components[i]->gameModel->mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
			FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, entity->prefab->components[i]->gameModel->mesh->getVertexCount()));

		FE_GL_ERROR(glBindVertexArray(0));

		entity->prefab->components[i]->gameModel->material->unBind();

		entity->prefab->components[i]->gameModel->material->shader = originalShader;
	}
}

void FERenderer::renderTerrain(FETerrain* terrain, FEBasicCamera* currentCamera)
{
	if (terrain->isWireframeMode())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	if (terrain->shader->getName() == "FESMTerrainShader")
	{
		terrain->heightMap->bind(0);
	}
	else
	{
		for (size_t i = 0; i < terrain->layers.size(); i++)
		{
			if (terrain->layers[i] != nullptr && terrain->layers[i]->getMaterial()->isCompackPacking())
			{
				if (terrain->layers[i]->getMaterial()->getAlbedoMap() != nullptr)
					terrain->layers[i]->getMaterial()->getAlbedoMap()->bind(int(i * 3));

				if (terrain->layers[i]->getMaterial()->getNormalMap() != nullptr)
					terrain->layers[i]->getMaterial()->getNormalMap()->bind(int(i * 3 + 1));

				if (terrain->layers[i]->getMaterial()->getAOMap() != nullptr)
					terrain->layers[i]->getMaterial()->getAOMap()->bind(int(i * 3 + 2));
			}
		}

		terrain->heightMap->bind(24);
		if (terrain->projectedMap != nullptr)
			terrain->projectedMap->bind(25);

		for (size_t i = 0; i < FE_TERRAIN_MAX_LAYERS / FE_TERRAIN_LAYER_PER_TEXTURE; i++)
		{
			if (terrain->layerMaps[i] != nullptr)
				terrain->layerMaps[i]->bind(int(26 + i));
		}
	}

	terrain->shader->start();
	loadStandardParams(terrain->shader, currentCamera, nullptr, &terrain->transform, terrain->isReceivingShadows());
	// ************ Load materials data for all terrain layers ************

	int layersUsed = terrain->layersUsed();
	if (layersUsed == 0)
	{
		// 0 index is for hightMap.
		RESOURCE_MANAGER.noTexture->bind(1);
	}

	terrain->loadLayersDataToGPU();
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, terrain->GPULayersDataBuffer));

	// Shadow map shader does not have this parameter.
	if (terrain->shader->getParameter("usedLayersCount") != nullptr)
		terrain->shader->getParameter("usedLayersCount")->updateData(float(layersUsed));
	// ************ Load materials data for all terrain layers END ************

	terrain->shader->getParameter("hightScale")->updateData(terrain->hightScale);
	terrain->shader->getParameter("scaleFactor")->updateData(terrain->scaleFactor);
	if (terrain->shader->getName() != "FESMTerrainShader")
		terrain->shader->getParameter("tileMult")->updateData(terrain->tileMult);
	terrain->shader->getParameter("LODlevel")->updateData(terrain->LODlevel);
	terrain->shader->getParameter("hightMapShift")->updateData(terrain->hightMapShift);

	static glm::vec3 pivotPosition = terrain->transform.getPosition();
	pivotPosition = terrain->transform.getPosition();
	terrain->scaleFactor = 1.0f * terrain->chunkPerSide;

	static int PVMHash = int(std::hash<std::string>{}("FEPVMMatrix"));
	static int WMHash = int(std::hash<std::string>{}("FEWorldMatrix"));
	static int HShiftHash = int(std::hash<std::string>{}("hightMapShift"));

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

			if (terrain->shader->getParameter("hightMapShift") != nullptr)
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
	if (lineCounter >= FE_MAX_LINES)
	{
		LOG.add("Tring to draw more than maxLines", FE_LOG_ERROR, FE_LOG_RENDERING);
		return;
	}

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

bool FERenderer::isSkyEnabled()
{
	return skyDome->isVisible();
}

void FERenderer::setSkyEnabld(bool newValue)
{
	skyDome->setVisibility(newValue);
}

float FERenderer::getDistanceToSky()
{
	return skyDome->transform.scale[0];
}

void FERenderer::setDistanceToSky(float newValue)
{
	skyDome->transform.setScale(glm::vec3(newValue));
}

bool FERenderer::isDistanceFogEnabled()
{
	return distanceFogEnabled;
}

void FERenderer::setDistanceFogEnabled(bool newValue)
{
	if (distanceFogEnabled == false && newValue == true)
	{
		if (distanceFogDensity <= 0.0f)
			distanceFogDensity = 0.007f;
		if (distanceFogGradient <= 0.0f)
			distanceFogGradient = 2.5f;
	}
	distanceFogEnabled = newValue;
	updateFogInShaders();
}

float FERenderer::getDistanceFogDensity()
{
	return distanceFogDensity;
}

void FERenderer::setDistanceFogDensity(float newValue)
{
	distanceFogDensity = newValue;
	updateFogInShaders();
}

float FERenderer::getDistanceFogGradient()
{
	return distanceFogGradient;
}

void FERenderer::setDistanceFogGradient(float newValue)
{
	distanceFogGradient = newValue;
	updateFogInShaders();
}

void FERenderer::updateFogInShaders()
{
	if (distanceFogEnabled)
	{
		RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("fogDensity")->updateData(distanceFogDensity);
		RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("fogGradient")->updateData(distanceFogGradient);

		RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("fogDensity")->updateData(distanceFogDensity);
		RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("fogGradient")->updateData(distanceFogGradient);

#ifdef USE_DEFERRED_RENDERER

#else
		RESOURCE_MANAGER.getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("fogDensity")->updateData(distanceFogDensity);
		RESOURCE_MANAGER.getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("fogGradient")->updateData(distanceFogGradient);
#endif // USE_DEFERRED_RENDERER
	}
	else
	{
		RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("fogDensity")->updateData(-1.0f);
		RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("fogGradient")->updateData(-1.0f);

		RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("fogDensity")->updateData(-1.0f);
		RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/)->getParameter("fogGradient")->updateData(-1.0f);
#ifdef USE_DEFERRED_RENDERER

#else
		RESOURCE_MANAGER.getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("fogDensity")->updateData(-1.0f);
		RESOURCE_MANAGER.getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/)->getParameter("fogGradient")->updateData(-1.0f);
#endif // USE_DEFERRED_RENDERER
	}
}

float FERenderer::getChromaticAberrationIntensity()
{
	if (getPostProcessEffect("506D804162647749060C3E68"/*"chromaticAberration"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("506D804162647749060C3E68"/*"chromaticAberration"*/)->stages[0]->shader->getParameter("intensity")->data;
}

void FERenderer::setChromaticAberrationIntensity(float newValue)
{
	if (getPostProcessEffect("506D804162647749060C3E68"/*"chromaticAberration"*/) == nullptr)
		return;
	getPostProcessEffect("506D804162647749060C3E68"/*"chromaticAberration"*/)->stages[0]->shader->getParameter("intensity")->updateData(newValue);
}

float FERenderer::getDOFNearDistance()
{
	if (getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[0]->shader->getParameter("depthThreshold")->data;
}

void FERenderer::setDOFNearDistance(float newValue)
{
	if (getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return;
	getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[0]->shader->getParameter("depthThreshold")->updateData(newValue);
	getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[1]->shader->getParameter("depthThreshold")->updateData(newValue);
}

float FERenderer::getDOFFarDistance()
{
	if (getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[0]->shader->getParameter("depthThresholdFar")->data;
}

void FERenderer::setDOFFarDistance(float newValue)
{
	if (getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return;
	getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[0]->shader->getParameter("depthThresholdFar")->updateData(newValue);
	getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[1]->shader->getParameter("depthThresholdFar")->updateData(newValue);
}

float FERenderer::getDOFStrength()
{
	if (getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[0]->shader->getParameter("blurSize")->data;
}

void FERenderer::setDOFStrength(float newValue)
{
	if (getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return;
	getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[0]->shader->getParameter("blurSize")->updateData(newValue);
	getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[1]->shader->getParameter("blurSize")->updateData(newValue);
}

float FERenderer::getDOFDistanceDependentStrength()
{
	if (getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[0]->shader->getParameter("intMult")->data;
}

void FERenderer::setDOFDistanceDependentStrength(float newValue)
{
	if (getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/) == nullptr)
		return;
	getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[0]->shader->getParameter("intMult")->updateData(newValue);
	getPostProcessEffect("217C4E80482B6C650D7B492F"/*"DOF"*/)->stages[1]->shader->getParameter("intMult")->updateData(newValue);
}

float FERenderer::getBloomThreshold()
{
	if (getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->stages[0]->shader->getParameter("thresholdBrightness")->data;
}

void FERenderer::setBloomThreshold(float newValue)
{
	if (getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/) == nullptr)
		return;
	getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->stages[0]->shader->getParameter("thresholdBrightness")->updateData(newValue);
}

float FERenderer::getBloomSize()
{
	if (getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->stages[1]->stageSpecificUniforms[1].data;
}

void FERenderer::setBloomSize(float newValue)
{
	if (getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/) == nullptr)
		return;
	getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->stages[1]->stageSpecificUniforms[1].updateData(newValue);
	getPostProcessEffect("451C48791871283D372C5938"/*"bloom"*/)->stages[2]->stageSpecificUniforms[1].updateData(newValue);
}

float FERenderer::getFXAASpanMax()
{
	if (getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->stages[0]->shader->getParameter("FXAASpanMax")->data;
}

void FERenderer::setFXAASpanMax(float newValue)
{
	if (getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return;
	getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->stages[0]->shader->getParameter("FXAASpanMax")->updateData(newValue);
}

float FERenderer::getFXAAReduceMin()
{
	if (getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->stages[0]->shader->getParameter("FXAAReduceMin")->data;
}

void FERenderer::setFXAAReduceMin(float newValue)
{
	if (getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return;
	getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->stages[0]->shader->getParameter("FXAAReduceMin")->updateData(newValue);
}

float FERenderer::getFXAAReduceMul()
{
	if (getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return 0.0f;
	return *(float*)getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->stages[0]->shader->getParameter("FXAAReduceMul")->data;
}

void FERenderer::setFXAAReduceMul(float newValue)
{
	if (getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/) == nullptr)
		return;
	getPostProcessEffect("0A3F10643F06525D70016070"/*"FE_FXAA"*/)->stages[0]->shader->getParameter("FXAAReduceMul")->updateData(newValue);
}

void FERenderer::drawAABB(FEAABB AABB, glm::vec3 color, float lineWidth)
{
	// bottom plane
	drawLine(glm::vec3(AABB.getMin()), glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMin()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMin()), glm::vec3(AABB.getMin()[0], AABB.getMin()[1], AABB.getMax()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMin()[2]), glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMax()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMax()[2]), glm::vec3(AABB.getMin()[0], AABB.getMin()[1], AABB.getMax()[2]), color, lineWidth);

	// upper plane
	drawLine(glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMin()[2]), glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMin()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMin()[2]), glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMax()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMin()[2]), glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMax()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMax()[2]), glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMax()[2]), color, lineWidth);

	// conect two planes
	drawLine(glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMin()[2]), glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMin()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMin()[0], AABB.getMin()[1], AABB.getMax()[2]), glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMax()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMax()[0], AABB.getMin()[1], AABB.getMax()[2]), glm::vec3(AABB.getMax()[0], AABB.getMax()[1], AABB.getMax()[2]), color, lineWidth);
	drawLine(glm::vec3(AABB.getMin()[0], AABB.getMin()[1], AABB.getMin()[2]), glm::vec3(AABB.getMin()[0], AABB.getMax()[1], AABB.getMin()[2]), color, lineWidth);
}

void FERenderer::forceShader(FEShader* shader)
{
	shaderToForce = shader;
}

void FERenderer::updateGPUCullingFrustum(float** frustum, glm::vec3 cameraPosition)
{
	float* frustumBufferData = (float*)glMapNamedBufferRange(frustumInfoBuffer, 0, sizeof(float) * (32),
		GL_MAP_WRITE_BIT |
		GL_MAP_INVALIDATE_BUFFER_BIT |
		GL_MAP_UNSYNCHRONIZED_BIT);

	for (size_t i = 0; i < 6; i++)
	{
		frustumBufferData[i * 4] = frustum[i][0];
		frustumBufferData[i * 4 + 1] = frustum[i][1];
		frustumBufferData[i * 4 + 2] = frustum[i][2];
		frustumBufferData[i * 4 + 3] = frustum[i][3];
	}

	frustumBufferData[24] = cameraPosition[0];
	frustumBufferData[25] = cameraPosition[1];
	frustumBufferData[26] = cameraPosition[2];

	FE_GL_ERROR(glUnmapNamedBuffer(frustumInfoBuffer));
}

void FERenderer::GPUCulling(FEEntityInstanced* entity, int subGameModel)
{
	if (freezeCulling)
		return;

	entity->checkDirtyFlag(subGameModel);

	FE_FrustumCullingShader->start();

	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, entity->renderers[subGameModel]->sourceDataBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, entity->renderers[subGameModel]->positionsBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, frustumInfoBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, entity->renderers[subGameModel]->LODBuffers[0]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, entity->renderers[subGameModel]->AABBSizesBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, cullingLODCountersBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, entity->renderers[subGameModel]->LODInfoBuffer));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, entity->renderers[subGameModel]->LODBuffers[1]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, entity->renderers[subGameModel]->LODBuffers[2]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, entity->renderers[subGameModel]->LODBuffers[3]));
	FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, entity->renderers[subGameModel]->indirectDrawInfoBuffer));

	FE_FrustumCullingShader->dispatch(GLuint(ceil(entity->instanceCount / 64.0f)), 1, 1);
	FE_GL_ERROR(glMemoryBarrier(GL_ALL_BARRIER_BITS));

	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, 0));
	//FE_GL_ERROR(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, 0));
}

#ifdef USE_DEFERRED_RENDERER

void FEGBuffer::initializeResources(FEFramebuffer* mainFrameBuffer)
{
	GFrameBuffer = RESOURCE_MANAGER.createFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, mainFrameBuffer->getColorAttachment()->getWidth(), mainFrameBuffer->getColorAttachment()->getHeight());

	positions = RESOURCE_MANAGER.createTexture(GL_RGB32F, GL_RGB, mainFrameBuffer->getColorAttachment()->getWidth(), mainFrameBuffer->getColorAttachment()->getHeight());
	GFrameBuffer->setColorAttachment(positions, 1);

	normals = RESOURCE_MANAGER.createTexture(GL_RGB16F, GL_RGB, mainFrameBuffer->getColorAttachment()->getWidth(), mainFrameBuffer->getColorAttachment()->getHeight());
	GFrameBuffer->setColorAttachment(normals, 2);

	albedo = RESOURCE_MANAGER.createTexture(GL_RGB, GL_RGB, mainFrameBuffer->getColorAttachment()->getWidth(), mainFrameBuffer->getColorAttachment()->getHeight());
	GFrameBuffer->setColorAttachment(albedo, 3);

	materialProperties = RESOURCE_MANAGER.createTexture(GL_RGBA16F, GL_RGBA, mainFrameBuffer->getColorAttachment()->getWidth(), mainFrameBuffer->getColorAttachment()->getHeight());
	GFrameBuffer->setColorAttachment(materialProperties, 4);

	shaderProperties = RESOURCE_MANAGER.createTexture(GL_RGBA, GL_RGBA, mainFrameBuffer->getColorAttachment()->getWidth(), mainFrameBuffer->getColorAttachment()->getHeight());
	GFrameBuffer->setColorAttachment(shaderProperties, 5);
}

FEGBuffer::FEGBuffer(FEFramebuffer* mainFrameBuffer)
{
	initializeResources(mainFrameBuffer);
}

void FEGBuffer::renderTargetResize(FEFramebuffer* mainFrameBuffer)
{
	delete GFrameBuffer;
	initializeResources(mainFrameBuffer);
}

#endif // USE_DEFERRED_RENDERER

std::unordered_map<std::string, std::function<FETexture* ()>> FERenderer::getDebugOutputTextures()
{
	return debugOutputTextures;
}