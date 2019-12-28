#include "FERenderer.h"
using namespace FocalEngine;

FERenderer* FERenderer::_instance = nullptr;

FERenderer::FERenderer()
{
}

void FERenderer::render(FEBasicCamera* currentCamera)
{
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();

	auto it = scene.entityMap.begin();
	while (it != scene.entityMap.end())
	{
		auto entity = it->second;

		entity->material->bind();
		
		for (size_t j = 0; j < entity->material->shaders[0]->params.size(); j++)
		{
			if (entity->material->shaders[0]->params[j].getParamName() == std::string("FEWorldMatrix"))
				entity->material->shaders[0]->params[j].updateData(entity->worldMatrix);

			if (entity->material->shaders[0]->params[j].getParamName() == std::string("FEViewMatrix"))
				entity->material->shaders[0]->params[j].updateData(currentCamera->getViewMatrix());

			if (entity->material->shaders[0]->params[j].getParamName() == std::string("FEProjectionMatrix"))
				entity->material->shaders[0]->params[j].updateData(currentCamera->getProjectionMatrix());

			if (entity->material->shaders[0]->params[j].getParamName() == std::string("FECameraPosition"))
				entity->material->shaders[0]->params[j].updateData(currentCamera->getPosition());

			if (entity->material->shaders[0]->params[j].getParamName() == std::string("FELightPosition"))
				entity->material->shaders[0]->params[j].updateData(scene.sceneLights[0]->getPosition());

			if (entity->material->shaders[0]->params[j].getParamName() == std::string("FELightColor"))
				entity->material->shaders[0]->params[j].updateData(scene.sceneLights[0]->getColor());
		}

		entity->material->shaders[0]->loadDataToGPU();
		entity->render();

		entity->material->unBind();

		it++;
	}
}