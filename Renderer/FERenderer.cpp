#include "FERenderer.h"
using namespace FocalEngine;

FERenderer* FERenderer::_instance = nullptr;

FERenderer::FERenderer()
{
}

void FERenderer::standardFBInit(int WindowWidth, int WindowHeight)
{
	sceneToTextureFB = new FEFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, WindowWidth, WindowHeight);
}

void FERenderer::loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEEntity* entity)
{
	static char* name = new char[256];
	FEScene& scene = FEScene::getInstance();

	auto iterator = shader->parameters.begin();
	while (iterator != shader->parameters.end())
	{
		auto parameterName = iterator->first.c_str();

		strcpy_s(name, 256, "FEWorldMatrix");
		if (strcmp(parameterName, name) == 0)
			iterator->second.updateData(entity->transform.getTransformMatrix());

		strcpy_s(name, 256, "FEViewMatrix");
		if (strcmp(parameterName, name) == 0)
			iterator->second.updateData(currentCamera->getViewMatrix());

		strcpy_s(name, 256, "FEProjectionMatrix");
		if (strcmp(parameterName, name) == 0)
			iterator->second.updateData(currentCamera->getProjectionMatrix());

		strcpy_s(name, 256, "FECameraPosition");
		if (strcmp(parameterName, name) == 0)
			iterator->second.updateData(currentCamera->getPosition());

		strcpy_s(name, 256, "FEGamma");
		if (strcmp(parameterName, name) == 0)
			iterator->second.updateData(currentCamera->getGamma());

		strcpy_s(name, 256, "FEExposure");
		if (strcmp(parameterName, name) == 0)
			iterator->second.updateData(currentCamera->getExposure());

		strcpy_s(name, 256, "FEReceiveShadows");
		if (strcmp(parameterName, name) == 0)
			iterator->second.updateData(entity->isReceivingShadows());

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
		postProcessEffects.back()->stages[i]->outTexture = sceneToTextureFB->getColorAttachment()->createSameFormatTexture();
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
			directionalLightInfo.activeCascades = light->activeCascades;
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

	std::vector<std::string> materialsList = resourceManager.getMaterialList();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FEMaterial* material = resourceManager.getMaterial(materialsList[i]);

		auto iteratorBlock = material->shader->blockUniforms.begin();
		while (iteratorBlock != material->shader->blockUniforms.end())
		{
			if (iteratorBlock->first.c_str() == std::string("lightInfo"))
			{
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
					FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, index * sizeOfFELightShaderInfo + 128, 64, &info[index].lightSpaceBig));

					index++;
					lightIterator++;
				}
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			}
			else if (iteratorBlock->first.c_str() == std::string("directionalLightInfo"))
			{
				FE_GL_ERROR(glBindBuffer(GL_UNIFORM_BUFFER, iteratorBlock->second));
				
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 0, 16, &directionalLightInfo.position));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 16, 16, &directionalLightInfo.color));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 32, 16, &directionalLightInfo.direction));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 48, 64, &directionalLightInfo.CSM0));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 112, 64, &directionalLightInfo.CSM1));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 176, 64, &directionalLightInfo.CSM2));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 240, 64, &directionalLightInfo.CSM3));
				FE_GL_ERROR(glBufferSubData(GL_UNIFORM_BUFFER, 304, 4, &directionalLightInfo.activeCascades));
				
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
				light->updateCascades(currentCamera->getPosition(), currentCamera->getForward());
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
					// to-do: should put out of scene bounderies in case of direcctional light.
					currentCamera->projectionMatrix = light->cascadeData[i].projectionMat;
					currentCamera->viewMatrix = light->cascadeData[i].viewMat;

					FE_GL_ERROR(glViewport(0, 0, light->cascadeData[i].frameBuffer->getWidth(), light->cascadeData[i].frameBuffer->getHeight()));

					light->cascadeData[i].frameBuffer->bind();
					FE_GL_ERROR(glClear(GL_DEPTH_BUFFER_BIT));

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

	FEEntity* needToDrawAtEndTest = nullptr;

	auto it = scene.entityMap.begin();
	while (it != scene.entityMap.end())
	{
		auto entity = it->second;
		
		if (entity->isVisible())
			renderEntity(entity, currentCamera);

		it++;
	}

	//if (needToDrawAtEndTest != nullptr)
	//{
	//	needToDrawAtEndTest->castShadows = false;
	//	needToDrawAtEndTest->transform.setScale(glm::vec3(0.05f * 0.1f));

	//	needToDrawAtEndTest->gameModel->material->getParameter("baseColor")->updateData(glm::vec3(0.1f, 0.9f, 0.1f));
	//		

	//	FEAABB SphereAABBS = scene.entityMap["entity_1"]->getPureAABB();
	//	glm::vec3 mins = SphereAABBS.getMin();
	//	glm::vec3 maxs = SphereAABBS.getMax();

	//	float xSizes = sqrt((maxs.x - mins.x) * (maxs.x - mins.x));
	//	float ySizes = sqrt((maxs.y - mins.y) * (maxs.y - mins.y));
	//	float zSizes = sqrt((maxs.z - mins.z) * (maxs.z - mins.z));


	//	FEAABB meshAABB = needToDrawAtEndTest->getPureAABB();
	//	glm::vec3 min = meshAABB.getMin();
	//	glm::vec3 max = meshAABB.getMax();

	//	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	//	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	//	ySize *= needToDrawAtEndTest->transform.getScale()[1];
	//	float zSize = sqrt((max.z - min.z) * (max.z - min.z));

	//	//needToDrawAtEndTest->transform.setPosition(scene.entityMap["entity_1"]->transform.getPosition() - glm::vec3(maxs.x - xSizes / 2.0f, maxs.y - ySizes / 2.0f, maxs.z - zSizes / 2.0f));
	//	glm::vec3 sphereCenter = scene.entityMap["entity_1"]->transform.getPosition() - glm::vec3(maxs.x - xSizes / 2.0f, maxs.y - ySizes / 2.0f, maxs.z - zSizes / 2.0f);
	//	//glm::vec3 
	//	//needToDrawAtEndTest->transform.setPosition(sphereCenter);

	//	/*needToDrawAtEndTest->transform.setPosition(glm::vec3(0.0f));
	//	needToDrawAtEndTest->transform.setRotation(glm::vec3(0.0f));
	//	needToDrawAtEndTest->transform.setScale(glm::vec3(1.0f));
	//	FEAABB meshAABB = needToDrawAtEndTest->getAABB();
	//	glm::vec3 min = meshAABB.getMin();
	//	glm::vec3 max = meshAABB.getMax();

	//	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	//	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	//	float zSize = sqrt((max.z - min.z) * (max.z - min.z));*/


	//	FE_GL_ERROR(glEnable(GL_CULL_FACE));
	//	FE_GL_ERROR(glCullFace(GL_BACK));
	//	//FE_GL_ERROR(glDepthFunc(GL_ALWAYS));


	//	//needToDrawAtEndTest->transform.setPosition(-glm::vec3(max.x - xSize / 2.0f, max.y - ySize / 2.0f, max.z - zSize / 2.0f));
	//	glm::vec3 cameraPosition = currentCamera->getPosition();
	//	glm::vec3 toObject = /*needToDrawAtEndTest->transform.getPosition()*/sphereCenter - cameraPosition;
	//	toObject = glm::normalize(toObject);

	//	needToDrawAtEndTest->transform.setPosition((currentCamera->getPosition() + toObject * 0.2f));// + glm::vec3(0.0f, ySize/*0.7f*//*ySize / 2.0f*/, 0.0f));

	//	/*position = currentCamera->projectionMatrix * currentCamera->getViewMatrix() * needToDrawAtEndTest->transform.getTransformMatrix() * position;
	//	position.x = position.x / position.w;
	//	position.y = position.y / position.w;
	//	position.z = position.z / position.w;
	//	position.w = position.w / position.w;*/

	//	renderEntity(needToDrawAtEndTest, currentCamera);

	//	//FE_GL_ERROR(glDepthFunc(GL_LESS));
	//	FE_GL_ERROR(glDisable(GL_CULL_FACE));
	//	FE_GL_ERROR(glCullFace(GL_BACK));

	//	//currentCamera->projectionMatrix = oldProjectionMatrix;

	//	//needToDrawAtEndTest->transform.setPosition(oldPosition);
	//}
	
	sceneToTextureFB->unBind();
	// ********* RENDER SCENE END *********

	// ********* POST_PROCESS EFFECTS *********
	FETexture* prevStageTex = sceneToTextureFB->getColorAttachment();

	for (size_t i = 0; i < postProcessEffects.size(); i++)
	{
		FEPostProcess& effect = *postProcessEffects[i];
		for (size_t j = 0; j < effect.stages.size(); j++)
		{
			effect.stages[j]->shader->start();
			loadStandardParams(effect.stages[j]->shader, currentCamera, nullptr);
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
	
	FEResourceManager::getInstance().saveFETexture(fileName, pixels, width, height);
	delete[] pixels;
}

void FERenderer::renderEntity(FEEntity* entity, FEBasicCamera* currentCamera, bool reloadUniformBlocks)
{
	if (reloadUniformBlocks)
		loadUniformBlocks();

	entity->gameModel->material->bind();
	loadStandardParams(entity->gameModel->material->shader, currentCamera, entity);
	entity->gameModel->material->shader->loadDataToGPU();
	entity->render();
	entity->gameModel->material->unBind();
}