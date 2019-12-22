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
		//glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), currentCamera->getPosition(), glm::vec3(0.0f, 1.0f, 0.0f));

		sceneGraph[i]->material->shaders[0]->loadWorldMatrix(sceneGraph[i]->worldMatrix);
		sceneGraph[i]->material->shaders[0]->loadViewMatrix(view);
		sceneGraph[i]->material->shaders[0]->loadProjectionMatrix(currentCamera->getProjectionMatrix());
		sceneGraph[i]->render();
		sceneGraph[i]->material->unBind();
	}
}