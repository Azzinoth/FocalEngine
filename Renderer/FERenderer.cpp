#include "FERenderer.h"
using namespace FocalEngine;

FERenderer* FERenderer::_instance = nullptr;

FERenderer::FERenderer()
{
}

void FERenderer::standardFBInit(int WindowWidth, int WindowHeight)
{
	sceneToTextureFB = new FEFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, WindowWidth, WindowHeight);
	sceneToTextureFB_DELETE = new FEFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, WindowWidth, WindowHeight);
}

void FERenderer::loadStandardParams(FEShader* shader, FEBasicCamera* currentCamera, FEEntity* entity)
{
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();

	for (size_t j = 0; j < shader->params.size(); j++)
	{
		if (shader->params[j].getParamName() == std::string("FEWorldMatrix"))
			shader->params[j].updateData(entity->worldMatrix);

		if (shader->params[j].getParamName() == std::string("FEViewMatrix"))
			shader->params[j].updateData(currentCamera->getViewMatrix());

		if (shader->params[j].getParamName() == std::string("FEProjectionMatrix"))
			shader->params[j].updateData(currentCamera->getProjectionMatrix());

		if (shader->params[j].getParamName() == std::string("FECameraPosition"))
			shader->params[j].updateData(currentCamera->getPosition());

		if (shader->params[j].getParamName() == std::string("FELightPosition"))
			shader->params[j].updateData(scene.sceneLights[0]->getPosition());

		if (shader->params[j].getParamName() == std::string("FELightColor"))
			shader->params[j].updateData(scene.sceneLights[0]->getColor());

		if (shader->params[j].getParamName() == std::string("FEGamma"))
			shader->params[j].updateData(currentCamera->getGamma());

		if (shader->params[j].getParamName() == std::string("FEExposure"))
			shader->params[j].updateData(currentCamera->getExposure());
	}
}

void FERenderer::render(FEBasicCamera* currentCamera)
{
	// ********* RENDER SCENE *********
	sceneToTextureFB->bind();
	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();

	auto it = scene.entityMap.begin();
	while (it != scene.entityMap.end())
	{
		auto entity = it->second;

		entity->material->bind();
		loadStandardParams(entity->material->shaders[0], currentCamera, entity);
		entity->material->shaders[0]->loadDataToGPU();
		entity->render();
		entity->material->unBind();

		it++;
	}

	sceneToTextureFB->unBind();

	sceneToTextureFB_DELETE->bind();
	FE_GL_ERROR(glEnable(GL_DEPTH_TEST));
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	it = scene.entityMap.begin();
	while (it != scene.entityMap.end())
	{
		auto entity = it->second;

		entity->material->bind();
		loadStandardParams(entity->material->shaders[0], currentCamera, entity);
		entity->material->shaders[0]->loadDataToGPU();
		entity->render();
		entity->material->unBind();

		it++;
	}

	sceneToTextureFB_DELETE->unBind();
	// ********* RENDER SCENE END *********

	for (size_t i = 0; i < scene.sceneScreenSpaceEffects.size(); i++)
	{
		FEScreenSpaceEffect& effect = *scene.sceneScreenSpaceEffects[i];

		if (i == 0)
		{
			if (!effect.inTexture)
			{
				effect.inTexture = sceneToTextureFB->getColorAttachment();
			}
			else if (effect.inTexture->getTextureID() != sceneToTextureFB->getColorAttachment()->getTextureID())
			{
				std::swap(effect.inTexture, effect.finalTexture);
			}
		}
		else
		{
			effect.inTexture = scene.sceneScreenSpaceEffects[i - 1]->finalTexture;
			effect.finalTexture = scene.sceneScreenSpaceEffects[i - 1]->inTexture;
		}

		for (size_t j = 0; j < effect.stages.size(); j++)
		{
			effect.stages[j]->shader->start();
			loadStandardParams(effect.stages[j]->shader, currentCamera, nullptr);
			effect.stages[j]->shader->loadDataToGPU();

			if (j > 0)
				std::swap(effect.inTexture, effect.finalTexture);
			
			effect.inTexture->bind(0);
			if (j == 3)
			{
				//FE_GL_ERROR(glActiveTexture(GL_TEXTURE1));
				//FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, sceneToTextureFB_DELETE->getColorAttachment()->getTextureID()));
				sceneToTextureFB_DELETE->getColorAttachment()->bind(1);
			}
				
			effect.intermediateFramebuffer->setColorAttachment(effect.finalTexture);

			effect.intermediateFramebuffer->bind();

			FE_GL_ERROR(glBindVertexArray(effect.screenQuad->getVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, effect.screenQuad->getVertexCount(), GL_UNSIGNED_INT, 0));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			effect.intermediateFramebuffer->unBind();

			effect.inTexture->unBind();
			effect.stages[j]->shader->stop();
		}

		if (i == scene.sceneScreenSpaceEffects.size() - 1)
		{
			effect.screenQuadShader->start();
			effect.screenQuadShader->loadDataToGPU();
			effect.finalTexture->bind(0);

			FE_GL_ERROR(glBindVertexArray(effect.screenQuad->getVaoID()));
			FE_GL_ERROR(glEnableVertexAttribArray(0));
			FE_GL_ERROR(glDrawElements(GL_TRIANGLES, effect.screenQuad->getVertexCount(), GL_UNSIGNED_INT, 0));
			FE_GL_ERROR(glDisableVertexAttribArray(0));
			FE_GL_ERROR(glBindVertexArray(0));

			effect.finalTexture->unBind();
			effect.screenQuadShader->stop();
		}
	}
}