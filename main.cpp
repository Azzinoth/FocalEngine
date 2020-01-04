#include "FEngine.h"

static const char* const MyVS = R"(
#version 400 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* const MyFS = R"(
#version 400 core

in vec2 textureCoords;
uniform sampler2D inputTexture;

void main(void)
{
	gl_FragColor = texture(inputTexture, textureCoords) * 0.5f;
}
)";

static const char* const MyVS2 = R"(
#version 400 core

@In_Position@
out vec2 textureCoords;

void main(void)
{
	gl_Position = vec4(FEPosition, 1.0);
	textureCoords = vec2((FEPosition.x + 1.0) / 2.0, 1 - (-FEPosition.y + 1.0) / 2.0);
}
)";

static const char* const MyFS2 = R"(
#version 400 core

in vec2 textureCoords;
uniform sampler2D inputTexture;

void main(void)
{
	gl_FragColor = texture(inputTexture, textureCoords) * vec4(2.0f, 1.0f, 1.0f, 1.0f);
}
)";

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
	FocalEngine::FETexture* rockTexture = resourceManager.createTexture(RES_FOLDER "slunl_2K_Albedo.png");
	FocalEngine::FEEntity* newEntity = new FocalEngine::FEEntity(resourceManager.loadObjMeshData(RES_FOLDER "rocks1.obj"), new FocalEngine::FEPhongMaterial(rockTexture));
	newEntity->setPosition(glm::vec3(-10.5f, -5.0f, -10.0f));
	newEntity->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	newEntity->setScale(glm::vec3(0.1f, 0.1f, 0.1f));

	scene.add(newEntity);

	FocalEngine::FEEntity* testEntity4 = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("plane"), new FocalEngine::FEPhongMaterial(nullptr));
	testEntity4->setPosition(glm::vec3(0.0f, 2.0f, 0.0f));
	testEntity4->setRotation(glm::vec3(90.0f, 0.0f, 0.0f));
	testEntity4->setScale(glm::vec3(2.1f, 2.1f, 2.1f));

	scene.add(testEntity4);

	FocalEngine::FELight* lightBlob = new FocalEngine::FELight();
	scene.add(lightBlob);

	FocalEngine::FEFramebuffer* fb = new FocalEngine::FEFramebuffer(FocalEngine::FE_COLOR_ATTACHMENT | FocalEngine::FE_DEPTH_ATTACHMENT, engine.getWindowWidth(), engine.getWindowHeight());
	testEntity4->material->addTexture(fb->getColorAttachment());

	FocalEngine::FEScreenSpaceEffect* testEffect = engine.createScreenSpaceEffect();
	testEffect->setFinalTexture(fb->getColorAttachment());


	FocalEngine::FEShader* testshader = new FocalEngine::FEShader(MyVS, MyFS);
	FocalEngine::FEShader* testshader2 = new FocalEngine::FEShader(MyVS2, MyFS2);

	testEffect->addStage(new FocalEngine::FEScreenSpaceEffectStage(fb->getColorAttachment(), testshader));
	testEffect->addStage(new FocalEngine::FEScreenSpaceEffectStage(testEffect->getFinalTexture(), testshader2));

	while (engine.isWindowOpened())
	{
		engine.beginFrame();

		testEntity4->setVisibility(false);
		engine.renderTo(fb);
		testEntity4->setVisibility(true);

		engine.render();

		testEffect->render();

		engine.endFrame();

		// CPU and GPU Time
		std::string cpuMS = std::to_string(engine.getCpuTime());
		cpuMS.erase(cpuMS.begin() + 4, cpuMS.end());

		std::string gpuMS = std::to_string(engine.getGpuTime());
		gpuMS.erase(gpuMS.begin() + 4, gpuMS.end());

		std::string caption = "CPU time : ";
		caption += cpuMS;
		caption += " ms";
		caption += "  Frame time : ";
		caption += gpuMS;
		caption += " ms";
		engine.setWindowCaption(caption.c_str());
	}
	
	return 0;
}