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

		if (sceneGraph[i]->material->shaders[0]->macroWorldMatrix)
			sceneGraph[i]->material->shaders[0]->loadWorldMatrix(sceneGraph[i]->worldMatrix);
		if (sceneGraph[i]->material->shaders[0]->macroViewMatrix)
			sceneGraph[i]->material->shaders[0]->loadViewMatrix(currentCamera->getViewMatrix());
		if (sceneGraph[i]->material->shaders[0]->macroProjectionMatrix)
			sceneGraph[i]->material->shaders[0]->loadProjectionMatrix(currentCamera->getProjectionMatrix());

		sceneGraph[i]->material->shaders[0]->loadData();
		sceneGraph[i]->render();

		sceneGraph[i]->material->unBind();
	}
}