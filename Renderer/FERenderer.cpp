#include "FERenderer.h"

FocalEngine::FERenderer* Renderer = nullptr;

FocalEngine::FERenderer::FERenderer()
{
}

glm::mat4 FocalEngine::FERenderer::getProjectionMatrix()
{
	#define WIN_W 1280
	#define WIN_H 720

	return glm::perspective(FOV, float(WIN_W) / float(WIN_H), NEAR_PLANE, FAR_PLANE);
}

void FocalEngine::FERenderer::addToScene(FEEntity* newEntity)
{
	sceneGraph.push_back(newEntity);
}

void FocalEngine::FERenderer::render()
{
	for (size_t i = 0; i < sceneGraph.size(); i++)
	{
		glm::mat4 trans = glm::mat4(1.0);
		trans = glm::translate(trans, sceneGraph[i]->position);
		sceneGraph[i]->material->bind();
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//sceneGraph[i]->material->shaders[0]->loadProjectionMatrix(view * trans);
		sceneGraph[i]->material->shaders[0]->loadProjectionMatrix(getProjectionMatrix() * view * trans);
		sceneGraph[i]->render();
		sceneGraph[i]->material->unBind();
	}
}