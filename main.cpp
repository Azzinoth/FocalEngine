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
uniform float blurSize;

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
			blurTextureCoords[i + 5] = centerTexCoords + vec2(0.0, tex_offset.y * i * blurSize);
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
uniform float blurSize;

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
			blurTextureCoords[i + 5] = centerTexCoords + vec2(tex_offset.x * i * blurSize, 0.0);
		}

		gl_FragColor = vec4(0.0);
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
FocalEngine::FELight* lightBlob;
bool isCameraInputActive = true;

void mouseButtonCallback(int button, int action, int mods)
{
	//FocalEngine::FEScene::getInstance().getEntity("testCube2")->material->setParam("baseColor", glm::vec3(0.1f, 0.6f, 0.1f));
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
			ImGui::SliderInt(param->name.c_str(), &iData, 0, 10);
			param->updateData(iData);
			break;
		}

		case FocalEngine::FE_FLOAT_SCALAR_UNIFORM:
		{
			float fData = *(float*)param->data;
			ImGui::SliderFloat(param->name.c_str(), &fData, 0.0f, 10.0f);
			param->updateData(fData);
			break;
		}

		case FocalEngine::FE_VECTOR2_UNIFORM:
		{
			glm::vec2 color = *(glm::vec2*)param->data;
			ImGui::ColorEdit3(param->name.c_str(), &color.x);
			param->updateData(color);
			break;
		}

		case FocalEngine::FE_VECTOR3_UNIFORM:
		{
			glm::vec3 color = *(glm::vec3*)param->data;
			ImGui::ColorEdit3(param->name.c_str(), &color.x);
			param->updateData(color);
			break;
		}

		case FocalEngine::FE_VECTOR4_UNIFORM:
		{
			glm::vec4 color = *(glm::vec4*)param->data;
			ImGui::ColorEdit3(param->name.c_str(), &color.x);
			param->updateData(color);
			break;
		}

		case FocalEngine::FE_MAT4_UNIFORM:
		{
			//loadMatrix(iterator->second.getName().c_str(), *(glm::mat4*)iterator->second.data);
			break;
		}

		default:
			break;
	}
}

std::string item_current = "";

void displayMaterialPrameters(FocalEngine::FEMaterial* material)
{
	std::vector<std::string> materialList = FocalEngine::FEResourceManager::getInstance().getMaterialList();
	materialList.push_back("testMat");
	materialList.push_back("testMatgargr");

	if (ImGui::BeginCombo("Materials", item_current.c_str(), ImGuiWindowFlags_None))
	{
		for (size_t n = 0; n < materialList.size(); n++)
		{
			bool is_selected = (item_current == materialList[n]);
			if (ImGui::Selectable(materialList[n].c_str(), is_selected))
				item_current = materialList[n].c_str();
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	std::string text = "Parameters of " + material->getName() + " :";
	if (ImGui::CollapsingHeader(text.c_str(), ImGuiWindowFlags_None)) {
		ImGui::PushID(0); // to create scopes and avoid ID conflicts within the same Window.
		std::vector<std::string> params = material->getParameterList();
		FocalEngine::FEShaderParam* param;
		for (size_t i = 0; i < params.size(); i++)
		{
			param = material->getParameter(params[i]);
			if (param->loadedFromEngine)
				continue;
			displayMaterialPrameter(param);
		}
		ImGui::PopID();
	}
}

void displaySceneEntities()
{
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();
	std::vector<std::string> entityList = scene.getEntityList();
	std::vector<std::string> materialList = FocalEngine::FEResourceManager::getInstance().getMaterialList();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(FocalEngine::FEngine::getInstance().getWindowWidth() / 3.7f, float(FocalEngine::FEngine::getInstance().getWindowHeight())));
	ImGui::Begin("Scene Entities", nullptr, ImGuiWindowFlags_NoCollapse);
		for (size_t i = 0; i < entityList.size(); i++)
		{
			FocalEngine::FEEntity* entity = scene.getEntity(entityList[i]);
			if (ImGui::TreeNode(entity->getName().c_str()))
			{
				ImGui::PushID(i); // to create scopes and avoid ID conflicts within the same Window.
				glm::vec3 pos = entity->getPosition();
				ImGui::Text("X pos :   ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##X pos : ", &pos[0], -100.0f, 100.0f);
				
				ImGui::Text("Y pos :   ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##Y pos : ", &pos[1], -100.0f, 100.0f);

				ImGui::Text("Z pos :   ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##Z pos : ", &pos[2], -100.0f, 100.0f);

				entity->setPosition(pos);

				// ROTATION
				glm::vec3 rot = entity->getRotation();

				ImGui::Text("X rot :   ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##X rot : ", &rot[0], 0.0f, 360.0f);

				ImGui::Text("Y rot :   ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##Y rot : ", &rot[1], 0.0f, 360.0f);

				ImGui::Text("Z rot :   ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##Z rot : ", &rot[2], 0.0f, 360.0f);

				entity->setRotation(rot);

				// SCALE
				glm::vec3 scale = entity->getScale();

				ImGui::SetNextItemWidth(40);
				ImGui::Text("X scale : ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##X scale : ", &scale[0], 0.01f, 10.0f);

				ImGui::Text("Y scale : ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##Y scale : ", &scale[1], 0.01f, 10.0f);

				ImGui::Text("Z scale : ");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(100);
				ImGui::SliderFloat("##Z scale : ", &scale[2], 0.01f, 10.0f);

				entity->setScale(scale);

				if (ImGui::CollapsingHeader("Mesh", ImGuiWindowFlags_None))
				{
					std::string meshText = "Name : ";
					meshText += entity->mesh->getName();
					ImGui::Text(meshText.c_str());

					ImGui::SameLine();
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
					if (ImGui::Button("Change Mesh"))
					{
						ImGui::OpenPopup("ChangeMesh");
					}

					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();

					if (ImGui::BeginPopupModal("ChangeMesh", NULL, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ImGui::Text("Insert mesh file path :");
						static char filePath[256] = "";

						ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
						ImGui::Separator();

						if (ImGui::Button("Load", ImVec2(120, 0)))
						{
							entity->mesh = FocalEngine::FEResourceManager::getInstance().loadObjMeshData(filePath);
							ImGui::CloseCurrentPopup();
							strcpy_s(filePath, "");
						}
						ImGui::SetItemDefaultFocus();
						ImGui::SameLine();
						if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
						ImGui::EndPopup();
					}
				}
				

				item_current = entity->material->getName();
				if (ImGui::BeginCombo("Materials", item_current.c_str(), ImGuiWindowFlags_None))
				{
					for (size_t n = 0; n < materialList.size(); n++)
					{
						bool is_selected = (item_current == materialList[n]);
						if (ImGui::Selectable(materialList[n].c_str(), is_selected))
						{
							item_current = materialList[n].c_str();
							entity->material = FocalEngine::FEResourceManager::getInstance().getMaterial(materialList[n]);
						}
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				std::string text = "Parameters of " + entity->material->getName() + " :";
				if (ImGui::CollapsingHeader(text.c_str(), 0))
				{
					std::vector<std::string> params = entity->material->getParameterList();
					FocalEngine::FEShaderParam* param;
					for (size_t j = 0; j < params.size(); j++)
					{
						param = entity->material->getParameter(params[j]);
						if (param->loadedFromEngine)
							continue;
						displayMaterialPrameter(param);
					}

					std::vector<std::string> textures = entity->material->getTextureList();
					for (size_t j = 0; j < textures.size(); j++)
					{
						ImGui::Text(textures[j].c_str());
						ImGui::Image((void*)(intptr_t)entity->material->getTexture(textures[j])->getTextureID(), ImVec2(32, 32));

						ImGui::PushID(std::to_string(j).c_str());
						ImGui::SameLine();
						ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
						if (ImGui::Button("Change"))
						{
							ImGui::OpenPopup("ChangeTexture");
						}

						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						ImGui::PopStyleColor();
						
						if (ImGui::BeginPopupModal("ChangeTexture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							ImGui::Text("Insert texture file path :");
							static char filePath[256] = "";
							
							ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
							ImGui::Separator();

							if (ImGui::Button("Load", ImVec2(120, 0)))
							{
								entity->material->setTexture(FocalEngine::FEResourceManager::getInstance().createTexture(filePath), textures[j]);
								ImGui::CloseCurrentPopup();
								strcpy_s(filePath, "");
							}
							ImGui::SetItemDefaultFocus();
							ImGui::SameLine();
							if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
							ImGui::EndPopup();
						}

						ImGui::PopID();
					}
				}

				ImGui::PopID();
				ImGui::TreePop();
			}
		}

		ImGui::Text("============================================");
		glm::vec3 pos = lightBlob->getPosition();
		ImGui::Text("Light X pos :   ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("##Light X pos : ", &pos[0], -50.0f, 50.0f);

		ImGui::Text("Light Y pos :   ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("##Light Y pos : ", &pos[1], -50.0f, 50.0f);

		ImGui::Text("Light Z pos :   ");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		ImGui::SliderFloat("##Light Z pos : ", &pos[2], -50.0f, 50.0f);

		lightBlob->setPosition(pos);

		float FEExposure = FocalEngine::FEngine::getInstance().getCamera()->getExposure();
		ImGui::SliderFloat("Camera Exposure", &FEExposure, 0.001f, 4.0f);
		FocalEngine::FEngine::getInstance().getCamera()->setExposure(FEExposure);

	ImGui::End();
}

void displayPostProcess()
{
	FocalEngine::FERenderer& renderer = FocalEngine::FERenderer::getInstance();
	std::vector<std::string> postProcessList = renderer.getPostProcessList();

	bool my_tool_active = true;
	ImGui::Begin("PostProcess Effects", &my_tool_active, ImGuiWindowFlags_None);

		for (size_t i = 0; i < postProcessList.size(); i++)
		{
			FocalEngine::FEPostProcess* PPEffect = renderer.getPostProcessEffect(postProcessList[i]);
			if (ImGui::CollapsingHeader(PPEffect->getName().c_str(), 0)) //ImGuiTreeNodeFlags_DefaultOpen
			{
				for (size_t j = 0; j < PPEffect->stages.size(); j++)
				{
					ImGui::PushID(j); // to create scopes and avoid ID conflicts within the same Window.
					std::vector<std::string> params = PPEffect->stages[j]->shader->getParameterList();
					FocalEngine::FEShaderParam* param;
					for (size_t i = 0; i < params.size(); i++)
					{
						param = PPEffect->stages[j]->shader->getParameter(params[i]);
						if (param->loadedFromEngine)
							continue;
						displayMaterialPrameter(param);
					}
					ImGui::PopID();
				}
			}
		}

	ImGui::End();
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

	FocalEngine::FEMaterial* testMat = resourceManager.createMaterial("TestMaterial");
	testMat->shader = new FocalEngine::FEShader(FESolidColorVS, FESolidColorFS);
	FocalEngine::FEShaderParam color(glm::vec3(0.0f, 0.4f, 0.6f), "baseColor");
	testMat->addParameter(color);

	scene.addEntity(resourceManager.getSimpleMesh("cube"), testMat, "testCube1");
	scene.getEntity("testCube1")->setPosition(glm::vec3(-1.5f, 0.0f, -10.0f));
	scene.getEntity("testCube1")->setRotation(glm::vec3(0.0f, 0.0f, 30.0f));

	scene.addEntity(resourceManager.getSimpleMesh("cube"), testMat, "testCube2");
	scene.getEntity("testCube2")->setPosition(glm::vec3(1.5f, 0.0f, 0.0f));

	
	scene.addEntity(resourceManager.getSimpleMesh("cube"), resourceManager.getMaterial("SolidColorMaterial"), "testCube3");
	scene.getEntity("testCube3")->setPosition(glm::vec3(-1.5f, -7.0f, -10.0f));
	scene.getEntity("testCube3")->setRotation(glm::vec3(0.0f, 45.0f, 45.0f));

	#define RES_FOLDER "C:/Users/kandr/Downloads/OpenGL test/resources/megascanRock/"
	FocalEngine::FETexture* rockTexture = resourceManager.createTexture(RES_FOLDER "slunl_4K_Albedo.png");
	rockTexture->setName("color map");
	FocalEngine::FETexture* rockNormalTexture = resourceManager.createTexture(RES_FOLDER "slunl_4K_Normal_LOD0.png");
	rockNormalTexture->setName("normal map");

	scene.addEntity(resourceManager.loadObjMeshData("C:/Users/kandr/Downloads/OpenGL test_12.08.2019/OpenGL test_16.01.2018/OpenGL test/OpenGL test/resources/cutTree/cutTree.obj"), resourceManager.getMaterial("PhongMaterial"), "brik");
	//scene.addEntity(resourceManager.loadObjMeshData(RES_FOLDER "rocks1.obj"), resourceManager.getMaterial("PhongMaterial"), "brik");
	resourceManager.getMaterial("PhongMaterial")->addTexture(rockTexture);
	resourceManager.getMaterial("PhongMaterial")->addTexture(rockNormalTexture);
	scene.getEntity("brik")->setPosition(glm::vec3(-10.5f, -5.0f, -10.0f));
	scene.getEntity("brik")->setRotation(glm::vec3(0.0f, 0.0f, 0.0f));
	scene.getEntity("brik")->setScale(glm::vec3(0.1f, 0.1f, 0.1f));

	lightBlob = new FocalEngine::FELight();
	lightBlob->setColor(glm::vec3(5.0f, 5.0f, 5.0f));
	scene.add(lightBlob);

	// to-do: there is no ability to add light :)
	FocalEngine::FELight* lightBlob2 = new FocalEngine::FELight();
	lightBlob2->setColor(glm::vec3(1.0f, 0.0f, 0.0f));
	lightBlob2->setPosition(glm::vec3(1.0f, 1.0f, 1.0f));
	scene.add(lightBlob2);

	FocalEngine::FEPostProcess* testEffect = engine.createPostProcess("DOF");
	FocalEngine::FEShader* testshader = new FocalEngine::FEShader(MyVS, MyFS);
	FocalEngine::FEShader* testshader2 = new FocalEngine::FEShader(MyVS, MyFS2);
	for (size_t i = 0; i < 1; i++)
	{
		testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader));
		testEffect->stages.back()->shader->getParameter("blurSize")->updateData(1.0f);
		testEffect->addStage(new FocalEngine::FEPostProcessStage(std::vector<int> { FocalEngine::FEPP_PREVIOUS_STAGE_RESULT0, FocalEngine::FEPP_SCENE_DEPTH}, testshader2));
		testEffect->stages.back()->shader->getParameter("blurSize")->updateData(1.0f);
	}
	renderer.addPostProcess(testEffect);

	FocalEngine::FEShaderParam testParam(1.0f, "FESpecularStrength");
	testMat->addParameter(testParam);

	item_current = FocalEngine::FEResourceManager::getInstance().getMaterialList()[0];

	while (engine.isWindowOpened())
	{
		engine.beginFrame();

		engine.render();

		//ImGui::ShowDemoWindow();
		//displayMaterialPrameters(testMat);
		displaySceneEntities();
		displayPostProcess();

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