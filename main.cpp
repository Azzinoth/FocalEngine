#include "FEngine.h"

static const char* const sTestVS = R"(
#version 400 core

in vec3 vPos;
uniform mat4 FEWorldMatrix;
uniform mat4 FEViewMatrix;
uniform mat4 FEProjectionMatrix;

void main(void)
{
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(vPos, 1.0);
}
)";

static const char* const sTestFS = R"(
#version 400 core

//out vec4 out_Color;

void main(void)
{
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

void mouseButtonCallback(int button, int action, int mods)
{
	int y = 0;
	y++;
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


	FocalEngine::FEShader* testShader = new FocalEngine::FEShader(sTestVS, sTestFS, std::vector<std::string> {"vPos"});
	FocalEngine::FEMaterial* testMaterial = new FocalEngine::FEMaterial();
	testMaterial->shaders.push_back(testShader);

	
	FocalEngine::FEEntity* testEntity = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"), testMaterial);
	testEntity->setPosition(glm::vec3(-1.5f, 0.0f, -10.0f));
	testEntity->setRotation(glm::vec3(0.0f, 0.0f, 30.0f));

	FocalEngine::FEEntity* testEntity2 = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"), testMaterial);
	testEntity2->setPosition(glm::vec3(1.5f, 0.0f, 0.0f));

	renderer.addToScene(testEntity);
	renderer.addToScene(testEntity2);

	FocalEngine::FERenderer& renderer1 = FocalEngine::FERenderer::getInstance();


	engine.setKeyCallback(keyButtonCallback);
	engine.setMouseButtonCallback(mouseButtonCallback);
	engine.setMouseMoveCallback(mouseMoveCallback);

	while (engine.isWindowOpened())
	{
		engine.beginFrame();

		

		engine.endFrame();
	}
	
	return 0;
}