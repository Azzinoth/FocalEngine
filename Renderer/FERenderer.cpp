#include "FERenderer.h"
using namespace FocalEngine;

FERenderer* FERenderer::_instance = nullptr;

FERenderer::FERenderer()
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	FEMaterial* newMat = resourceManager.createMaterial("SolidColorMaterial");
	newMat->shader = new FEShader(FESolidColorVS, FESolidColorFS);
	FocalEngine::FEShaderParam color(glm::vec3(1.0f, 0.4f, 0.6f), "baseColor");
	newMat->addParameter(color);

	newMat = resourceManager.createMaterial("PhongMaterial");
	newMat->shader = new FEShader(FEPhongVS, FEPhongFS);
}

void FERenderer::standardFBInit(int WindowWidth, int WindowHeight)
{
	sceneToTextureFB = new FEFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, WindowWidth, WindowHeight);
}

void FERenderer::loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEEntity* entity)
{
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();

	auto iterator = shader->parameters.begin();
	while (iterator != shader->parameters.end())
	{
		if (iterator->first == std::string("FEWorldMatrix"))
			iterator->second.updateData(entity->worldMatrix);

		if (iterator->first == std::string("FEViewMatrix"))
			iterator->second.updateData(currentCamera->getViewMatrix());

		if (iterator->first == std::string("FEProjectionMatrix"))
			iterator->second.updateData(currentCamera->getProjectionMatrix());

		if (iterator->first == std::string("FECameraPosition"))
			iterator->second.updateData(currentCamera->getPosition());

		if (iterator->first == std::string("FELightPosition"))
			iterator->second.updateData(scene.sceneLights[0]->getPosition());

		if (iterator->first == std::string("FELightColor"))
			iterator->second.updateData(scene.sceneLights[0]->getColor() * scene.sceneLights[0]->getIntensity());

		if (iterator->first == std::string("FEGamma"))
			iterator->second.updateData(currentCamera->getGamma());

		if (iterator->first == std::string("FEExposure"))
			iterator->second.updateData(currentCamera->getExposure());

		iterator++;
	}
}

void FERenderer::addPostProcess(FEPostProcess* newPostProcess)
{
	postProcessEffects.push_back(newPostProcess);

	for (size_t i = 0; i < postProcessEffects.back()->stages.size(); i++)
	{
		postProcessEffects.back()->stages[i]->inTexture.resize(postProcessEffects.back()->stages[i]->inTextureSource.size());
		//to-do: change when out texture could be different resolution or/and format.
		postProcessEffects.back()->stages[i]->outTexture = sceneToTextureFB->getColorAttachment()->createSameFormatTexture();
	}
}

void FERenderer::render(FEBasicCamera* currentCamera)
{
	// ********* RENDER SCENE *********
	sceneToTextureFB->bind();
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();

	auto it = scene.entityMap.begin();
	while (it != scene.entityMap.end())
	{
		auto entity = it->second;

		entity->material->bind();
		loadStandardParams(entity->material->shader, currentCamera, entity);
		entity->material->shader->loadDataToGPU();
		entity->render();
		entity->material->unBind();

		it++;
	}

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

	if (postProcessEffects.size())
	{
		FEPostProcess& effect = *postProcessEffects[postProcessEffects.size() - 1];
		effect.screenQuadShader->start();
		effect.screenQuadShader->loadDataToGPU();
		effect.stages.back()->outTexture->bind(0);

		FE_GL_ERROR(glBindVertexArray(effect.screenQuad->getVaoID()));
		FE_GL_ERROR(glEnableVertexAttribArray(0));
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, effect.screenQuad->getVertexCount(), GL_UNSIGNED_INT, 0));
		FE_GL_ERROR(glDisableVertexAttribArray(0));
		FE_GL_ERROR(glBindVertexArray(0));

		effect.stages.back()->outTexture->unBind();
		effect.screenQuadShader->stop();
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