#include "FEngine.h"

FocalEngine::FEEntity* testEntity2;

static const char* const StrangeMatVS = R"(
#version 400 core

@In_Position@
@In_Normal@

@WorldMatrix@
@ViewMatrix@
@ProjectionMatrix@

out vec3 Norm;

void main(void)
{
	Norm = FENormal;
	gl_Position = FEProjectionMatrix * FEViewMatrix * FEWorldMatrix * vec4(FEPosition, 1.0);
}
)";

static const char* const StrangeMatFS = R"(
#version 400 core

in vec3 Norm;

void main(void)
{
	gl_FragColor = vec4(Norm, 0.0f); // 1.0f, 0.0f, 0.0f
}
)";

class StrangeShader : public FocalEngine::FEShader
{
public:
	StrangeShader() : FEShader(StrangeMatVS, StrangeMatFS) {};
	~StrangeShader() {};

private:

};

class StrangeMat : public FocalEngine::FEMaterial
{
public:
	StrangeMat()
	{
		shaders.push_back(new StrangeShader());
	};
	~StrangeMat() {};
private:
};

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
	
	FocalEngine::FEEntity* testEntity = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"));
	testEntity->setPosition(glm::vec3(-1.5f, 0.0f, -10.0f));
	testEntity->setRotation(glm::vec3(0.0f, 0.0f, 30.0f));

	testEntity2 = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"));
	testEntity2->setPosition(glm::vec3(1.5f, 0.0f, 0.0f));

	renderer.addToScene(testEntity);
	renderer.addToScene(testEntity2);

	FocalEngine::FERenderer& renderer1 = FocalEngine::FERenderer::getInstance();

	engine.setKeyCallback(keyButtonCallback);
	engine.setMouseButtonCallback(mouseButtonCallback);
	engine.setMouseMoveCallback(mouseMoveCallback);

	FocalEngine::FEEntity* testEntity3 = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"), new StrangeMat());
	testEntity3->setPosition(glm::vec3(-1.5f, -5.0f, -10.0f));
	testEntity3->setRotation(glm::vec3(0.0f, 0.0f, 45.0f));

	renderer.addToScene(testEntity3);

	int storage = 0;

	
	storage |= 1; // FE_POSITION
	storage <<= 1;
	storage |= 1;  // FE_COLOR
	storage <<= 1;
	storage |= 1;  // FE_NORMAL
	storage <<= 1;
	storage |= 1;  // FE_TANGENTS

	int test = FocalEngine::FE_POSITION | FocalEngine::FE_NORMAL;

	bool ifFE_POSITION = test & FocalEngine::FE_POSITION;
	bool ifFE_COLOR = test & FocalEngine::FE_COLOR;
	bool ifFE_NORMAL = test & FocalEngine::FE_NORMAL;
	bool ifFE_TANGENTS = test & FocalEngine::FE_TANGENTS;

	while (engine.isWindowOpened())
	{
		engine.beginFrame();

		

		engine.endFrame();
	}
	
	return 0;
}