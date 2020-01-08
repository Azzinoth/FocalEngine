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
@Texture@ sceneTexture;
@Texture@ depthTexture;
uniform float depthThreshold;

void main(void)
{
	float depthValue = texture(depthTexture, textureCoords).r;
	depthValue = (1 - depthValue) * 10;
	//gl_FragColor = vec4(vec3((1 - depthValue) * 10), 1.0); // only for perspective projection

	if (depthValue > depthThreshold)
	{
		//gl_FragColor = texture(depthTexture, textureCoords) * vec4(0.5);
		vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords; // * 0.5 + 0.5

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + vec2(0.0, tex_offset.y * i * 2);
		}

		gl_FragColor = vec4(0.0);
		gl_FragColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else
	{
		gl_FragColor = texture(sceneTexture, textureCoords);
	}

}
)";

static const char* const MyFS2 = R"(
#version 400 core

in vec2 textureCoords;
@Texture@ sceneTexture;
@Texture@ depthTexture;
uniform float depthThreshold;

void main(void)
{
	float depthValue = texture(depthTexture, textureCoords).r;
	depthValue = (1 - depthValue) * 10;
	//gl_FragColor = vec4(vec3((1 - depthValue) * 10), 1.0); // only for perspective projection

	if (depthValue > depthThreshold)
	{
		//gl_FragColor = texture(depthTexture, textureCoords) * vec4(0.5);
		vec2 tex_offset = 1.0 / textureSize(sceneTexture, 0);

		vec2 blurTextureCoords[11];
		vec2 centerTexCoords = textureCoords; // * 0.5 + 0.5

		for (int i = -5; i <= 5; i++)
		{
			blurTextureCoords[i + 5] = centerTexCoords + vec2(tex_offset.x * i * 2, 0.0);
		}

		gl_FragColor = vec4(0.0);
		gl_FragColor += texture(sceneTexture, blurTextureCoords[0]) * 0.0093;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[1]) * 0.028002;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[2]) * 0.065984;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[3]) * 0.121703;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[4]) * 0.175713;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[5]) * 0.198596;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[6]) * 0.175713;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[7]) * 0.121703;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[8]) * 0.065984;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[9]) * 0.028002;
		gl_FragColor += texture(sceneTexture, blurTextureCoords[10]) * 0.0093;
	}
	else
	{
		gl_FragColor = texture(sceneTexture, textureCoords);
	}

}
)";

FocalEngine::FEEntity* testEntity2;
bool isCameraInputActive = true;

void mouseButtonCallback(int button, int action, int mods)
{
	//dynamic_cast<FocalEngine::FEStandardMaterial*>(testEntity2->material)->setBaseColor(glm::vec3(0.1f, 0.6f, 0.1f));
	testEntity2->material->setParam("baseColor", glm::vec3(0.1f, 0.6f, 0.1f));

	//FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	//engine.getCamera()->setExposure(engine.getCamera()->getExposure() - 0.01f);
}

void mouseMoveCallback(double xpos, double ypos)
{
	int y = 0;
	y++;
}

void keyButtonCallback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		isCameraInputActive = !isCameraInputActive;
		FocalEngine::FEngine::getInstance().getCamera()->setIsInputActive(isCameraInputActive);
	}
}

void displayMaterialPrameter(FocalEngine::FEShaderParam* param)
{
	switch (param->type)
	{
		case FocalEngine::FE_INT_SCALAR_UNIFORM:
		{
			int iData = *(int*)param->data;
			ImGui::SliderInt(param->paramName.c_str(), &iData, 0, 10);
			param->updateData(iData);
			break;
		}

		case FocalEngine::FE_FLOAT_SCALAR_UNIFORM:
		{
			float fData = *(float*)param->data;
			ImGui::SliderFloat(param->paramName.c_str(), &fData, 0.0f, 10.0f);
			param->updateData(fData);
			break;
		}

		case FocalEngine::FE_VECTOR2_UNIFORM:
		{
			glm::vec2 color = *(glm::vec2*)param->data;
			ImGui::ColorEdit3(param->paramName.c_str(), &color.x);
			param->updateData(color);
			break;
		}

		case FocalEngine::FE_VECTOR3_UNIFORM:
		{
			glm::vec3 color = *(glm::vec3*)param->data;
			ImGui::ColorEdit3(param->paramName.c_str(), &color.x);
			param->updateData(color);
			break;
		}

		case FocalEngine::FE_VECTOR4_UNIFORM:
		{
			glm::vec4 color = *(glm::vec4*)param->data;
			ImGui::ColorEdit3(param->paramName.c_str(), &color.x);
			param->updateData(color);
			break;
		}

		case FocalEngine::FE_MAT4_UNIFORM:
		{
			//loadMatrix(iterator->second.getParamName().c_str(), *(glm::mat4*)iterator->second.data);
			break;
		}

		default:
			break;
	}
}

void displayMaterialPrameters(FocalEngine::FEMaterial* material)
{
	std::string text = "Parameters of " + material->getName() + " :";
	if (ImGui::CollapsingHeader(text.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::PushID(0); // to create scopes and avoid ID conflicts within the same Window.
		std::vector<std::string> params = material->getParameterList();
		FocalEngine::FEShaderParam* param;
		for (size_t i = 0; i < params.size(); i++)
		{
			param = material->getParametr(params[i]);

			displayMaterialPrameter(param);
		}
		ImGui::PopID();
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	engine.createWindow();
	engine.setKeyCallback(keyButtonCallback);
	engine.setMouseButtonCallback(mouseButtonCallback);
	engine.setMouseMoveCallback(mouseMoveCallback);

	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();
	FocalEngine::FERenderer& renderer = FocalEngine::FERenderer::getInstance();
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();


	FocalEngine::FESolidColorMaterial* testMat = new FocalEngine::FESolidColorMaterial();
	testMat->setName("cubeMaterial");
	
	FocalEngine::FEEntity* testEntity = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"), testMat);
	testEntity->setPosition(glm::vec3(-1.5f, 0.0f, -10.0f));
	testEntity->setRotation(glm::vec3(0.0f, 0.0f, 30.0f));

	testEntity2 = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"), testMat);
	testEntity2->setPosition(glm::vec3(1.5f, 0.0f, 0.0f));

	scene.add(testEntity);
	scene.add(testEntity2);

	

	FocalEngine::FEEntity* testEntity3 = new FocalEngine::FEEntity(resourceManager.getSimpleMesh("cube"));
	testEntity3->setPosition(glm::vec3(-1.5f, -7.0f, -10.0f));
	testEntity3->setRotation(glm::vec3(0.0f, 45.0f, 45.0f));

	scene.add(testEntity3);

	#define RES_FOLDER "C:/Users/kandr/Downloads/OpenGL test/resources/megascanRock/"
	FocalEngine::FETexture* rockTexture = resourceManager.createTexture(RES_FOLDER "slunl_4K_Albedo.png");
	

	FocalEngine::FEEntity* newEntity = new FocalEngine::FEEntity(resourceManager.loadObjMeshData(RES_FOLDER "rocks1.obj"), /*newMat*/ new FocalEngine::FEPhongMaterial(rockTexture));



	
	newEntity->setPosition(glm::vec3(-10.5f, -5.0f, -10.0f));
	newEntity->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	newEntity->setScale(glm::vec3(0.1f, 0.1f, 0.1f));
	scene.add(newEntity);

	FocalEngine::FELight* lightBlob = new FocalEngine::FELight();
	lightBlob->setColor(glm::vec3(5.0f, 5.0f, 5.0f));
	scene.add(lightBlob);

	// to-do: there is no ability to add light :)
	FocalEngine::FELight* lightBlob2 = new FocalEngine::FELight();
	lightBlob2->setColor(glm::vec3(1.0f, 0.0f, 0.0f));
	lightBlob2->setPosition(glm::vec3(1.0f, 1.0f, 1.0f));
	scene.add(lightBlob2);

	FocalEngine::FEPostProcess* testEffect = engine.createPostProcess();
	FocalEngine::FEShader* testshader = new FocalEngine::FEShader(MyVS, MyFS);
	FocalEngine::FEShader* testshader2 = new FocalEngine::FEShader(MyVS, MyFS2);
	testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader));
	testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader2));
	testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader));
	testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader2));
	renderer.addPostProcess(testEffect);

	FocalEngine::FEShaderParam testParam(1.0f, "FESpecularStrength");
	testMat->addParameter(testParam);

	while (engine.isWindowOpened())
	{
		engine.beginFrame();

		engine.render();

		FocalEngine::FEPostProcess* bloom = renderer.getPostProcessEffect(0);

		float exposure = engine.getCamera()->getExposure();
		ImGui::SliderFloat("cameraExposure", &exposure, 0.0f, 4.0f);
		engine.getCamera()->setExposure(exposure);

		float thresholdBrightness = *(float*)bloom->stages[0]->shader->getParam("thresholdBrightness").data;
		ImGui::SliderFloat("thresholdBrightness", &thresholdBrightness, 0.0f, 4.0f);
		bloom->stages[0]->shader->getParam("thresholdBrightness").updateData(thresholdBrightness);

		float depthThreshold = *(float*)testEffect->stages[1]->shader->getParam("depthThreshold").data;
		ImGui::SliderFloat("depthThreshold", &depthThreshold, 0.0f, 2.0f);
		testEffect->stages[0]->shader->getParam("depthThreshold").updateData(depthThreshold);
		testEffect->stages[1]->shader->getParam("depthThreshold").updateData(depthThreshold);

		displayMaterialPrameters(testMat);
		/*FocalEngine::FEShaderParam* param = testMat->getParametr("FESpecularStrength");
		float specularStrength = *(float*)param->data;
		ImGui::SliderFloat("SpecularStrength", &specularStrength, 0.0f, 4.0f);
		param->updateData(specularStrength);

		param = testMat->getParametr("baseColor");
		glm::vec3 color = *(glm::vec3*)param->data;
		ImGui::ColorEdit3("baseColor", &color.x);
		param->updateData(color);*/

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