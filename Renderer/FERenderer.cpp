#include "FERenderer.h"
using namespace FocalEngine;

FERenderer* FERenderer::_instance = nullptr;

FERenderer::FERenderer()
{
}

void FERenderer::addToScene(FEEntity* newEntity)
{
	sceneGraph.push_back(newEntity);
}

void FERenderer::render(FEBasicCamera* currentCamera)
{
	for (size_t i = 0; i < sceneGraph.size(); i++)
	{
		sceneGraph[i]->material->bind();

		for (size_t j = 0; j < sceneGraph[i]->material->shaders[0]->params.size(); j++)
		{
			if (sceneGraph[i]->material->shaders[0]->params[j].getParamName() == std::string("FEWorldMatrix"))
				sceneGraph[i]->material->shaders[0]->params[j].updateData(sceneGraph[i]->worldMatrix);

			if (sceneGraph[i]->material->shaders[0]->params[j].getParamName() == std::string("FEViewMatrix"))
				sceneGraph[i]->material->shaders[0]->params[j].updateData(currentCamera->getViewMatrix());

			if (sceneGraph[i]->material->shaders[0]->params[j].getParamName() == std::string("FEProjectionMatrix"))
				sceneGraph[i]->material->shaders[0]->params[j].updateData(currentCamera->getProjectionMatrix());
		}

		sceneGraph[i]->material->shaders[0]->loadDataToGPU();
		sceneGraph[i]->render();

		sceneGraph[i]->material->unBind();
	}
}