#include "FEngine.h"

FocalEngine::FEEntity* testEntity2;

void mouseButtonCallback(int button, int action, int mods)
{
	//dynamic_cast<FocalEngine::FEStandardMaterial*>(testEntity2->material)->setBaseColor(glm::vec3(0.1f, 0.6f, 0.1f));
	testEntity2->material->setParam("baseColor", glm::vec3(0.1f, 0.6f, 0.1f));
}

void mouseMoveCallback(double xpos, double ypos)
{
	int y = 0;
	y++;
}

void keyButtonCallback(int key, int scancode, int action, int mods)
{
	int y = 0;
	y++;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	engine.createWindow();
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();
	FocalEngine::FERenderer& renderer = FocalEngine::FERenderer::getInstance();
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();
	
	FocalEngine::FEEntity* testEntity = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"));
	testEntity->setPosition(glm::vec3(-1.5f, 0.0f, -10.0f));
	testEntity->setRotation(glm::vec3(0.0f, 0.0f, 30.0f));

	testEntity2 = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"));
	testEntity2->setPosition(glm::vec3(1.5f, 0.0f, 0.0f));

	scene.add(testEntity);
	scene.add(testEntity2);

	FocalEngine::FERenderer& renderer1 = FocalEngine::FERenderer::getInstance();

	engine.setKeyCallback(keyButtonCallback);
	engine.setMouseButtonCallback(mouseButtonCallback);
	engine.setMouseMoveCallback(mouseMoveCallback);

	FocalEngine::FEEntity* testEntity3 = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"));
	testEntity3->setPosition(glm::vec3(-1.5f, -7.0f, -10.0f));
	testEntity3->setRotation(glm::vec3(0.0f, 45.0f, 45.0f));

	scene.add(testEntity3);

	#define RES_FOLDER "C:/Users/kandr/Downloads/OpenGL test/resources/megascanRock/"
	FocalEngine::FEEntity* newEntity = new FocalEngine::FEEntity(resourceManager.loadObjMeshData(RES_FOLDER "rocks1.obj"));
	newEntity->setPosition(glm::vec3(-10.5f, -5.0f, -10.0f));
	newEntity->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	newEntity->setScale(glm::vec3(0.1f, 0.1f, 0.1f));

	scene.add(newEntity);

	FocalEngine::FELight* lightBlob = new FocalEngine::FELight();
	scene.add(lightBlob);

	while (engine.isWindowOpened())
	{
		engine.beginFrame();

		

		engine.endFrame();
	}
	
	return 0;
}