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

		for (size_t j = 0; j < sceneGraph[i]->material->shaders[0]->standardDataRequest.size(); j++)
		{
			if (sceneGraph[i]->material->shaders[0]->standardDataRequest[j].uniformName == std::string("FEWorldMatrix"))
				sceneGraph[i]->material->shaders[0]->loadMatrix("FEWorldMatrix", sceneGraph[i]->worldMatrix);

			if (sceneGraph[i]->material->shaders[0]->standardDataRequest[j].uniformName == std::string("FEViewMatrix"))
				sceneGraph[i]->material->shaders[0]->loadMatrix("FEViewMatrix", currentCamera->getViewMatrix());

			if (sceneGraph[i]->material->shaders[0]->standardDataRequest[j].uniformName == std::string("FEProjectionMatrix"))
				sceneGraph[i]->material->shaders[0]->loadMatrix("FEProjectionMatrix", currentCamera->getProjectionMatrix());
		}

		sceneGraph[i]->material->shaders[0]->loadDataToGPU();
		sceneGraph[i]->render();

		sceneGraph[i]->material->unBind();
	}
}