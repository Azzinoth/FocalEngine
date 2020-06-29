#include "../Editor/FEEditor.h"

void mouseButtonCallback(int button, int action, int mods)
{
	if ((!ImGui::GetIO().WantCaptureMouse) && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		checkPixelAccurateSelection = true;
	}
}

std::string getSelectedEntity()
{
	return selectedEntity;
}

void keyButtonCallback(int key, int scancode, int action, int mods)
{
	if (!ImGui::GetIO().WantCaptureMouse && key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		isCameraInputActive = !isCameraInputActive;
		FEngine::getInstance().getCamera()->setIsInputActive(isCameraInputActive);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		FEngine::getInstance().terminate();
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && key == GLFW_KEY_DELETE)
	{
		if (selectedEntity != "")
			FEScene::getInstance().deleteEntity(selectedEntity);
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_C)
	{
		clipboardEntity = selectedEntity;
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_V)
	{
		if (clipboardEntity != "")
		{
			FEScene& Scene = FEScene::getInstance();
			FEEntity* newEntity = Scene.addEntity(Scene.getEntity(clipboardEntity)->gameModel, "");
			newEntity->transform = Scene.getEntity(clipboardEntity)->transform;
			newEntity->transform.setPosition(newEntity->transform.getPosition() * 1.1f);
			selectedEntity = newEntity->getName();
		}
	}
}

void toolTip(const char* text)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(text);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void showTransformConfiguration(std::string objectName, FETransformComponent* transform)
{
	// ********************* POSITION *********************
	glm::vec3 position = transform->getPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + objectName).c_str(), &position[0], 0.1f);
	toolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + objectName).c_str(), &position[1], 0.1f);
	toolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + objectName).c_str(), &position[2], 0.1f);
	toolTip("Z position");
	transform->setPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = transform->getRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + objectName).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	toolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + objectName).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	toolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + objectName).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	toolTip("Z rotation");
	transform->setRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &transform->uniformScaling);
	glm::vec3 scale = transform->getScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + objectName).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	toolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + objectName).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	toolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + objectName).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	toolTip("Z scale");

	glm::vec3 oldScale = transform->getScale();
	transform->changeXScaleBy(scale[0] - oldScale[0]);
	transform->changeYScaleBy(scale[1] - oldScale[1]);
	transform->changeZScaleBy(scale[2] - oldScale[2]);
}

void showPosition(std::string objectName, glm::vec3& position)
{
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + objectName).c_str(), &position[0], 0.1f);
	toolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + objectName).c_str(), &position[1], 0.1f);
	toolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + objectName).c_str(), &position[2], 0.1f);
	toolTip("Z position");
}

void showRotation(std::string objectName, glm::vec3& rotation)
{
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + objectName).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	toolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + objectName).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	toolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + objectName).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	toolTip("Z rotation");
}

void showScale(std::string objectName, glm::vec3& scale)
{
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + objectName).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	toolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + objectName).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	toolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + objectName).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	toolTip("Z scale");
}

void displayMaterialPrameter(FEShaderParam* param)
{
	switch (param->type)
	{
	case FE_INT_SCALAR_UNIFORM:
	{
		int iData = *(int*)param->data;
		ImGui::SliderInt(param->name.c_str(), &iData, 0, 10);
		param->updateData(iData);
		break;
	}

	case FE_FLOAT_SCALAR_UNIFORM:
	{
		float fData = *(float*)param->data;
		ImGui::DragFloat(param->name.c_str(), &fData, 0.1f, 0.0f, 100.0f);
		param->updateData(fData);
		break;
	}

	case FE_VECTOR2_UNIFORM:
	{
		glm::vec2 color = *(glm::vec2*)param->data;
		ImGui::ColorEdit3(param->name.c_str(), &color.x);
		param->updateData(color);
		break;
	}

	case FE_VECTOR3_UNIFORM:
	{
		glm::vec3 color = *(glm::vec3*)param->data;
		ImGui::ColorEdit3(param->name.c_str(), &color.x);
		param->updateData(color);
		break;
	}

	case FE_VECTOR4_UNIFORM:
	{
		glm::vec4 color = *(glm::vec4*)param->data;
		ImGui::ColorEdit3(param->name.c_str(), &color.x);
		param->updateData(color);
		break;
	}

	case FE_MAT4_UNIFORM:
	{
		//loadMatrix(iterator->second.getName().c_str(), *(glm::mat4*)iterator->second.data);
		break;
	}

	default:
		break;
	}
}

void displayMaterialPrameters(FEMaterial* material)
{
	static std::string currentMaterial = "";
	std::vector<std::string> materialList = FEResourceManager::getInstance().getMaterialList();
	if (ImGui::BeginCombo("Materials", material->getName().c_str(), ImGuiWindowFlags_None))
	{
		for (size_t n = 0; n < materialList.size(); n++)
		{
			bool is_selected = (currentMaterial == materialList[n]);
			if (ImGui::Selectable(materialList[n].c_str(), is_selected))
				currentMaterial = materialList[n].c_str();
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	std::string text = "Parameters of " + material->getName() + " :";
	if (ImGui::CollapsingHeader(text.c_str(), ImGuiWindowFlags_None)) {
		ImGui::PushID(0);
		std::vector<std::string> params = material->getParameterList();
		FEShaderParam* param;
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

void displayLightProperties(FELight* light)
{
	showTransformConfiguration(light->getName(), &light->transform);

	if (light->getType() == FE_DIRECTIONAL_LIGHT)
	{
		FEDirectionalLight* directionalLight = reinterpret_cast<FEDirectionalLight*>(light);
		ImGui::Separator();
		ImGui::Text("-------------Shadow settings--------------");

		ImGui::Text("Number of cascades :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		int cascades = directionalLight->getActiveCascades();
		ImGui::SliderInt("##cascades", &cascades, 1, 4);
		directionalLight->setActiveCascades(cascades);
		toolTip("How much steps of shadow quality will be used.");

		ImGui::Text("Cascade exponent :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float cascadeDistributionExponent = directionalLight->getCascadeDistributionExponent();
		ImGui::DragFloat("##cascade exponent", &cascadeDistributionExponent, 0.1f, 0.1f, 10.0f);
		directionalLight->setCascadeDistributionExponent(cascadeDistributionExponent);
		toolTip("How much size of each next cascade is bigger than previous.");

		ImGui::Text("First cascade Size :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float firstCascadeSize = directionalLight->getFirstCascadeSize();
		ImGui::DragFloat("##firstCascadeSize", &firstCascadeSize, 0.1f, 0.1f, 500.0f);
		directionalLight->setFirstCascadeSize(firstCascadeSize);
		toolTip("Size of first cascade.");
	}
	else if (light->getType() == FE_POINT_LIGHT)
	{
	}
	else if (light->getType() == FE_SPOT_LIGHT)
	{
		FESpotLight* spotLight = reinterpret_cast<FESpotLight*>(light);
		glm::vec3 d = spotLight->getDirection();
		ImGui::DragFloat("##x", &d[0], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##y", &d[1], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##z", &d[2], 0.01f, 0.0f, 1.0f);

		float spotAngle = spotLight->getSpotAngle();
		ImGui::SliderFloat((std::string("Inner angle##") + spotLight->getName()).c_str(), &spotAngle, 0.0f, 90.0f);
		spotLight->setSpotAngle(spotAngle);

		float spotAngleOuter = spotLight->getSpotAngleOuter();
		ImGui::SliderFloat((std::string("Outer angle ##") + spotLight->getName()).c_str(), &spotAngleOuter, 0.0f, 90.0f);
		spotLight->setSpotAngleOuter(spotAngleOuter);
	}

	glm::vec3 color = light->getColor();
	ImGui::ColorEdit3((std::string("Color##") + light->getName()).c_str(), &color.x);
	light->setColor(color);

	float intensity = light->getIntensity();
	ImGui::SliderFloat((std::string("Intensity##") + light->getName()).c_str(), &intensity, 0.0f, 100.0f);
	light->setIntensity(intensity);
}

void displayLightsProperties()
{
	FEScene& scene = FEScene::getInstance();
	std::vector<std::string> lightList = scene.getLightsList();

	for (size_t i = 0; i < lightList.size(); i++)
	{
		if (ImGui::TreeNode(lightList[i].c_str()))
		{
			displayLightProperties(scene.getLight(lightList[i]));
			ImGui::TreePop();
		}
	}
}

void addEntityButton()
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Add new entity", ImVec2(220, 0)))
	{
		selectGameModelWindow.show(nullptr, true);
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	selectGameModelWindow.render();
}

void displaySceneEntities()
{
	FEScene& scene = FEScene::getInstance();
	std::vector<std::string> entityList = scene.getEntityList();
	std::vector<std::string> materialList = FEResourceManager::getInstance().getMaterialList();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(FEngine::getInstance().getWindowWidth() / 3.7f, float(FEngine::getInstance().getWindowHeight())));
	ImGui::Begin("Scene Entities", nullptr, ImGuiWindowFlags_None);
	addEntityButton();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Save project", ImVec2(220, 0)))
	{
		currentProject->saveScene();
		FEngine::getInstance().takeScreenshot((currentProject->getProjectFolder() + "projectScreenShot.FETexture").c_str());
	}

	if (ImGui::Button("Close project", ImVec2(220, 0)))
	{
		if (currentProject->modified)
		{
			// ask should we save project.
		}

		for (size_t i = 0; i < projectList.size(); i++)
		{
			delete projectList[i];
		}
		projectList.clear();
		currentProject = nullptr;

		loadProjectList();

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::End();

		return;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	if (selectedEntityWasChanged)
	{
		for (size_t i = 0; i < entityList.size(); i++)
		{
			ImGui::GetStateStorage()->SetInt(ImGui::GetID(entityList[i].c_str()), 0);
		}
		selectedEntityWasChanged = false;
		ImGui::GetStateStorage()->SetInt(ImGui::GetID(selectedEntity.c_str()), 1);
	}

	for (size_t i = 0; i < entityList.size(); i++)
	{
		FEEntity* entity = scene.getEntity(entityList[i]);
		if (ImGui::TreeNode(entity->getName().c_str()))
		{
			ImGui::PushID(entity->getName().c_str());
			showTransformConfiguration(entity->getName(), &entity->transform);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));

			if (ImGui::Button("Change name"))
			{
				renameEntityWindow.show(entity);
			}

			std::string text = "Game model name : ";
			text += entity->gameModel->getName();
			ImGui::Text(text.c_str());
			ImGui::SameLine();

			if (ImGui::Button("Change Game model"))
			{
				selectGameModelWindow.show(&entity->gameModel);
			}

			if (ImGui::Button("Delete entity"))
			{
				scene.deleteEntity(entity->getName());
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();

				ImGui::PopID();
				ImGui::TreePop();
				break;
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			
			ImGui::PopID();
			ImGui::TreePop();
		}
	}
	ImGui::Text("============================================");

	displayLightsProperties();

	float FEExposure = FEngine::getInstance().getCamera()->getExposure();
	ImGui::DragFloat("Camera Exposure", &FEExposure, 0.01f, 0.001f, 100.0f);
	FEngine::getInstance().getCamera()->setExposure(FEExposure);

	ImGui::End();

	selectMeshWindow.render();
	selectMaterialWindow.render();
	selectGameModelWindow.render();
	renameEntityWindow.render();
}

void displayMaterialContentBrowser()
{
	std::vector<std::string> materialList = FEResourceManager::getInstance().getMaterialList();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Add new material", ImVec2(220, 0)))
		ImGui::OpenPopup("New material");

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupModal("New material", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Insert name of new material :");
		static char filePath[512] = "";

		ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
		ImGui::Separator();

		if (ImGui::Button("Create", ImVec2(120, 0)))
		{
			FEMaterial* newMat = FEResourceManager::getInstance().createMaterial(filePath);
			if (newMat)
			{
				newMat->shader = new FEShader(FEPhongVS, FEPhongFS);

				newMat->albedoMap = FEResourceManager::getInstance().noTexture;
				newMat->normalMap = FEResourceManager::getInstance().noTexture;
			}

			ImGui::CloseCurrentPopup();
			strcpy_s(filePath, "");
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	for (size_t i = 0; i < materialList.size(); i++)
	{
		ImGui::PushID(i);
		FEMaterial* material = FEResourceManager::getInstance().getMaterial(materialList[i]);

		if (ImGui::CollapsingHeader(materialList[i].c_str(), 0))
		{
			std::vector<std::string> params = material->getParameterList();
			FEShaderParam* param;
			for (size_t j = 0; j < params.size(); j++)
			{
				param = material->getParameter(params[j]);
				if (param->loadedFromEngine)
					continue;
				displayMaterialPrameter(param);
			}

			ImGui::PushID("albedoMap_texture");
			ImGui::Text("albedoMap:");
			displayTextureInMaterialEditor(material->albedoMap);
			ImGui::PopID();

			ImGui::PushID("normalMap_texture");
			ImGui::Text("normalMap:");
			displayTextureInMaterialEditor(material->normalMap);
			ImGui::PopID();

			ImGui::PushID("roughtnessMap_texture");
			ImGui::Text("roughtnessMap:");
			displayTextureInMaterialEditor(material->roughtnessMap);
			ImGui::PopID();

			ImGui::PushID("metalnessMap_texture");
			ImGui::Text("metalnessMap:");
			displayTextureInMaterialEditor(material->metalnessMap);
			ImGui::PopID();

			ImGui::PushID("AOMap_texture");
			ImGui::Text("AOMap:");
			ImGui::PopID();
			displayTextureInMaterialEditor(material->AOMap);

			//# fix I need to create shaderMap or something because this is a waste of memory and time.
			std::vector<std::string> textureList = material->shader->getTextureList();
			bool hasAO = false;
			for (size_t j = 0; j < textureList.size(); j++)
			{
				if (textureList[j] == "AOTexture")
				{
					hasAO = true;
					break;
				}
			}

			if (!hasAO && material->AOMap != nullptr)
			{
				delete material->shader;
				material->shader = new FEShader(FEPhongVS, FEPhongAOFS);
			}

			ImGui::PushID("displacementMap_texture");
			ImGui::Text("displacementMap:");
			displayTextureInMaterialEditor(material->displacementMap);
			ImGui::PopID();
		}
		ImGui::PopID();
	}

	selectTextureWindow.render();
}

void displayPostProcessContentBrowser()
{
	FERenderer& renderer = FERenderer::getInstance();
	std::vector<std::string> postProcessList = renderer.getPostProcessList();

	for (size_t i = 0; i < postProcessList.size(); i++)
	{
		FEPostProcess* PPEffect = renderer.getPostProcessEffect(postProcessList[i]);
		if (ImGui::CollapsingHeader(PPEffect->getName().c_str(), 0)) //ImGuiTreeNodeFlags_DefaultOpen
		{
			for (size_t j = 0; j < PPEffect->stages.size(); j++)
			{
				ImGui::PushID(j);
				std::vector<std::string> params = PPEffect->stages[j]->shader->getParameterList();
				FEShaderParam* param;
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
}

int timesTextureUsed(FETexture* texture)
{
	int result = 0;
	std::vector<std::string> materiasList = FEResourceManager::getInstance().getMaterialList();

	for (size_t i = 0; i < materiasList.size(); i++)
	{
		FEMaterial* currentMaterial = FEResourceManager::getInstance().getMaterial(materiasList[i]);

		if (currentMaterial->albedoMap != nullptr && currentMaterial->albedoMap->getName() == texture->getName()) result++;
		if (currentMaterial->normalMap != nullptr && currentMaterial->normalMap->getName() == texture->getName()) result++;
		if (currentMaterial->roughtnessMap != nullptr && currentMaterial->roughtnessMap->getName() == texture->getName()) result++;
		if (currentMaterial->metalnessMap != nullptr && currentMaterial->metalnessMap->getName() == texture->getName()) result++;
		if (currentMaterial->AOMap != nullptr && currentMaterial->AOMap->getName() == texture->getName()) result++;
		if (currentMaterial->displacementMap != nullptr && currentMaterial->displacementMap->getName() == texture->getName()) result++;
	}

	return result;
}

int timesMeshUsed(FEMesh* mesh)
{
	int result = 0;
	std::vector<std::string> gameModelList = FEResourceManager::getInstance().getGameModelList();

	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* currentGameModel = FEResourceManager::getInstance().getGameModel(gameModelList[i]);

		if (currentGameModel->mesh == mesh)
			result++;
	}

	return result;
}

int timesGameModelUsed(FEGameModel* gameModel)
{
	int result = 0;
	std::vector<std::string> entitiesList = FEScene::getInstance().getEntityList();

	for (size_t i = 0; i < entitiesList.size(); i++)
	{
		FEEntity* currentEntity = FEScene::getInstance().getEntity(entitiesList[i]);

		if (currentEntity->gameModel == gameModel)
			result++;
	}

	return result;
}

int textureUnderMouse = -1;
bool isOpenContextMenuInContentBrowser = false;
int meshUnderMouse = -1;
static int activeTabContentBrowser = 0;

int gameModelUnderMouse = -1;

void displayContentBrowser()
{
	float mainWindowW = float(FEngine::getInstance().getWindowWidth());
	float mainWindowH = float(FEngine::getInstance().getWindowHeight());
	float windowW = mainWindowW / 3.7f;
	float windowH = mainWindowH;

	ImGui::SetNextWindowPos(ImVec2(mainWindowW - windowW, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(windowW, windowH));
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_None);
		ImGui::PushStyleColor(ImGuiCol_TabActive, (ImVec4)ImColor::ImColor(0.4f, 0.9f, 0.4f, 1.0f));
		if (ImGui::BeginTabBar("##Content Browser", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Meshes"))
			{
				activeTabContentBrowser = 0;
				displayMeshesContentBrowser();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Textures"))
			{
				activeTabContentBrowser = 1;
				displayTexturesContentBrowser();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Materials"))
			{
				activeTabContentBrowser = 2;
				displayMaterialContentBrowser();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Game Models"))
			{
				activeTabContentBrowser = 3;
				displayGameModelContentBrowser();
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("PostProcess"))
			{
				activeTabContentBrowser = 4;
				displayPostProcessContentBrowser();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
			ImGui::PopStyleColor();
		}

		bool open_context_menu = false;
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
			open_context_menu = true;

		if (open_context_menu)
			ImGui::OpenPopup("##context_menu");

		isOpenContextMenuInContentBrowser = false;
		if (ImGui::BeginPopup("##context_menu"))
		{
			isOpenContextMenuInContentBrowser = true;
			switch (activeTabContentBrowser)
			{
				case 0:
				{
					if (ImGui::MenuItem("Load new mesh..."))
					{
						std::string filePath = "";
						openDialog(filePath, meshLoadFilter, 1);
						if (filePath != "")
						{
							FEResourceManager::getInstance().LoadOBJMesh(filePath.c_str(), "", currentProject->getProjectFolder().c_str());
							// add asset list saving....
							currentProject->saveScene();
						}
					}

					if (meshUnderMouse != -1)
					{
						if (ImGui::MenuItem("Delete"))
						{
							deleteMeshWindow.show(FEResourceManager::getInstance().getMesh(FEResourceManager::getInstance().getMeshList()[meshUnderMouse]));
						}

						if (ImGui::MenuItem("Rename"))
						{
							renameMeshWindow.show(FEResourceManager::getInstance().getMesh(FEResourceManager::getInstance().getMeshList()[meshUnderMouse]));
						}
					}

					break;
				}
				case 1:
				{
					if (ImGui::MenuItem("Load texture..."))
					{
						std::string filePath = "";
						openDialog(filePath, textureLoadFilter, 1);
						if (filePath != "")
						{
							loadTextureWindow.show(filePath);
						}
					}

					if (ImGui::MenuItem("Load texture and combine with opacity mask..."))
					{
						std::string filePath = "";
						openDialog(filePath, textureLoadFilter, 1);

						std::string maskFilePath = "";
						openDialog(maskFilePath, textureLoadFilter, 1);

						if (filePath != "" && maskFilePath != "")
						{
							FETexture* newTexture = FEResourceManager::getInstance().LoadPngTextureWithTransparencyMaskAndCompress(filePath.c_str(), maskFilePath.c_str(), "");
							FEResourceManager::getInstance().saveFETexture((currentProject->getProjectFolder() + newTexture->getName() + ".FETexture").c_str(), newTexture);
						}
					}

					if (textureUnderMouse != -1)
					{
						if (ImGui::MenuItem("Delete"))
						{
							deleteTextureWindow.show(FEResourceManager::getInstance().getTexture(FEResourceManager::getInstance().getTextureList()[textureUnderMouse]));
						}

						if (ImGui::MenuItem("Rename"))
						{
							renameTextureWindow.show(FEResourceManager::getInstance().getTexture(FEResourceManager::getInstance().getTextureList()[textureUnderMouse]));
						}
					}
					break;
				}
				case 2:
				{
					if (ImGui::MenuItem("Create new material..."))
					{
					}
					break;
				}
				case 3:
				{
					if (ImGui::MenuItem("Create new game model"))
					{
						FEResourceManager::getInstance().createGameModel();
					}

					if (gameModelUnderMouse != -1)
					{
						if (ImGui::MenuItem("Edit"))
						{
							editGameModelWindow.show(FEResourceManager::getInstance().getGameModel(FEResourceManager::getInstance().getGameModelList()[gameModelUnderMouse]));
						}
						
						if (ImGui::MenuItem("Delete"))
						{
							deleteGameModelWindow.show(FEResourceManager::getInstance().getGameModel(FEResourceManager::getInstance().getGameModelList()[gameModelUnderMouse]));
						}

						if (ImGui::MenuItem("Rename"))
						{
							renameGameModelWindow.show(FEResourceManager::getInstance().getGameModel(FEResourceManager::getInstance().getGameModelList()[gameModelUnderMouse]));
						}
					}
					break;
				}
				case 4:
				{
					break;
				}
				default:
					break;
			}
			ImGui::EndPopup();
		}
	ImGui::End();
	
	loadTextureWindow.render();
	renameMeshWindow.render();
	renameTextureWindow.render();
	renameFailedWindow.render();
	deleteTextureWindow.render();
	deleteMeshWindow.render();
	renameGameModelWindow.render();
	deleteGameModelWindow.render();
	editGameModelWindow.render();
}

glm::dvec3 mouseRay()
{
	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / FEngine::getInstance().getWindowWidth() - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * (mouseY)) / FEngine::getInstance().getWindowHeight();

	glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(FEngine::getInstance().getCamera()->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(FEngine::getInstance().getCamera()->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

void determineEntityUnderMouse()
{
	entitiesUnderMouse.clear();

	std::vector<std::string> entityList = FEScene::getInstance().getEntityList();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		float dis = 0;
		FEAABB box = FEScene::getInstance().getEntity(entityList[i])->getAABB();

		if (box.rayIntersect(FEngine::getInstance().getCamera()->getPosition(), mouseRay(), dis))
		{
			entitiesUnderMouse.push_back(entityList[i]);
		}
	}
}

void setSelectedEntity(std::string newEntity)
{
	selectedEntity = newEntity;
	selectedEntityWasChanged = true;
}

void mouseMoveCallback(double xpos, double ypos)
{
	mouseX = xpos;
	//#fix magic number, I think I need 20 pixels because of window caption but I am not sure...
	mouseY = ypos + 20;

	determineEntityUnderMouse();
}

bool checkFolder(const char* dirPath)
{
	DWORD dwAttrib = GetFileAttributesA(dirPath);
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		   (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool createFolder(const char* dirPath)
{
	return (_mkdir(dirPath) != 0);
}

bool deleteFolder(const char* dirPath)
{
	return (_rmdir(dirPath) == 0);
}

void loadProjectList()
{
	if (!checkFolder(PROJECTS_FOLDER))
		createFolder(PROJECTS_FOLDER);

	std::vector<std::string> projectNameList = getFolderList(PROJECTS_FOLDER);

	for (size_t i = 0; i < projectNameList.size(); i++)
	{
		projectList.push_back(new FEProject(projectNameList[i].c_str(), std::string(PROJECTS_FOLDER) + std::string("/") + projectNameList[i].c_str() + "/"));
	}
}

void loadEditor()
{
	projectChosen = -1;
	FEngine::getInstance().getCamera()->setIsInputActive(isCameraInputActive);
	loadProjectList();

	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();
	FocalEngine::FERenderer& renderer = FocalEngine::FERenderer::getInstance();

	pixelAccurateSelectionFB = new FEFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, engine.getWindowWidth(), engine.getWindowHeight());
	delete pixelAccurateSelectionFB->getColorAttachment();
	pixelAccurateSelectionFB->setColorAttachment(new FETexture(GL_RGB, GL_RGB, engine.getWindowWidth(), engine.getWindowHeight()));
	
	pixelAccurateSelectionMaterial = resourceManager.createMaterial("pixelAccurateSelectionMaterial");
	resourceManager.makeMaterialStandard(pixelAccurateSelectionMaterial);
	pixelAccurateSelectionMaterial->shader = new FocalEngine::FEShader(FEPixelAccurateSelectionVS, FEPixelAccurateSelectionFS);
	FEShaderParam colorParam(glm::vec3(0.0f, 0.0f, 0.0f), "baseColor");
	pixelAccurateSelectionMaterial->addParameter(colorParam);

	// **************************** Meshes Content Browser ****************************
	previewFB = new FEFramebuffer(FE_COLOR_ATTACHMENT | FE_DEPTH_ATTACHMENT, 128, 128);
	previewGameModel = new FEGameModel(nullptr, nullptr, "editorPreviewGameModel");
	previewEntity = new FEEntity(previewGameModel, "editorPreviewEntity");
	meshPreviewMaterial = resourceManager.createMaterial("meshPreviewMaterial");
	resourceManager.makeMaterialStandard(meshPreviewMaterial);
	meshPreviewMaterial->shader = new FocalEngine::FEShader(FEMeshPreviewVS, FEMeshPreviewFS);
}

std::vector<std::string> getFolderList(const char* dirPath)
{
	std::vector<std::string> result;
	std::string pattern(dirPath);
	pattern.append("\\*");
	WIN32_FIND_DATAA data;
	HANDLE hFind;
	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (std::string(data.cFileName) != std::string(".") && std::string(data.cFileName) != std::string(".."))
				result.push_back(data.cFileName);
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}
	
	return result;
}

void renderEditor()
{
	if (currentProject)
	{
		displaySceneEntities();
		displayContentBrowser();

		if (checkPixelAccurateSelection)
		{
			checkPixelAccurateSelection = false;
			FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();

			pixelAccurateSelectionFB->bind();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

			for (size_t i = 0; i < entitiesUnderMouse.size(); i++)
			{
				potentiallySelectedEntity = FEScene::getInstance().getEntity(entitiesUnderMouse[i]);
				// sometimes we can delete entity before entitiesUnderMouse update
				if (potentiallySelectedEntity == nullptr)
					continue;

				FEMaterial* regularMaterial = potentiallySelectedEntity->gameModel->material;
				potentiallySelectedEntity->gameModel->material = pixelAccurateSelectionMaterial;

				int r = 0;
				int g = 0;
				int b = 0;

				r = (i + 1) & 255;
				g = ((i + 1) >> 8) & 255;
				b = ((i + 1) >> 16) & 255;
				pixelAccurateSelectionMaterial->getParameter("baseColor")->updateData(glm::vec3(float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f));
				pixelAccurateSelectionMaterial->albedoMap = regularMaterial->albedoMap;
				FERenderer::getInstance().renderEntity(potentiallySelectedEntity, engine.getCamera());
				potentiallySelectedEntity->gameModel->material = regularMaterial;
				pixelAccurateSelectionMaterial->albedoMap = nullptr;
			}

			glReadPixels(GLint(mouseX), GLint(engine.getWindowHeight() - mouseY), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, colorUnderMouse);
			pixelAccurateSelectionFB->unBind();
			glClearColor(0.55f, 0.73f, 0.87f, 1.0f);

			setSelectedEntity("");
			if (entitiesUnderMouse.size() > 0)
			{
				int index = 0;
				index |= int(colorUnderMouse[2]);
				index <<= 8;
				index |= int(colorUnderMouse[1]);
				index <<= 8;
				index |= int(colorUnderMouse[0]);

				index -= 1;

				if (index >= 0 && entitiesUnderMouse.size() > size_t(index))
					setSelectedEntity(entitiesUnderMouse[index]);
			}
		}
	}
	else
	{
		displayProjectSelection();
	}

	if (FELOG::getInstance().log.size() > 0)
	{
		char text[1024];
		strcpy_s(text, FELOG::getInstance().log[0].size() + 1, FELOG::getInstance().log[0].data());
		ImGui::InputTextMultiline("LOG", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);
	}
}

void displayProjectSelection()
{
	float mainWindowW = float(FEngine::getInstance().getWindowWidth());
	float mainWindowH = float(FEngine::getInstance().getWindowHeight());

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(mainWindowW, mainWindowH - 170.0f));
	ImGui::Begin("Project Browser", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
	{
		ImGui::SetWindowFontScale(2.0f);
		ImGui::Text("CHOOSE WHAT PROJECT TO LOAD :");
		ImGui::SetWindowFontScale(1.0f);

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

		int columnCount = int(mainWindowW / (512.0f + 32.0f));
		ImGui::Columns(columnCount, "projectColumns", false);
		static bool pushedStyle = false;
		for (size_t i = 0; i < projectList.size(); i++)
		{
			ImGui::PushID(i);
			pushedStyle = false;
			if (projectChosen == i)
			{
				pushedStyle = true;
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
			}

			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (projectChosen != -1)
				{
					openProject(projectChosen);
				}
			}

			if (ImGui::ImageButton((void*)(intptr_t)projectList[i]->sceneScreenshot->getTextureID(), ImVec2(512.0f, 288.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
			{
				projectChosen = i;
			}

			ImVec2 textWidth = ImGui::CalcTextSize(projectList[i]->getName().c_str());
			ImGui::Text(projectList[i]->getName().c_str());

			if (pushedStyle)
			{
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}

			ImGui::PopID();
			ImGui::NextColumn();
		}

		ImGui::Columns(1);

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
	}
	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::SetNextWindowPos(ImVec2(0.0f, mainWindowH - 170.0f));
	ImGui::SetNextWindowSize(ImVec2(mainWindowW, 170.0f));
	ImGui::Begin("##create project", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
	{
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.75f, 0.70f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

		if (ImGui::Button("Create New Project", ImVec2(200.0f, 64.0f)))
			ImGui::OpenPopup("New Project");

		ImGui::SameLine();
		if (ImGui::Button("Open Project", ImVec2(200.0f, 64.0f)) && projectChosen != -1)
		{
			openProject(projectChosen);
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete Project", ImVec2(200.0f, 64.0f)) && projectChosen != -1)
		{
			std::string projectFolder = projectList[projectChosen]->getProjectFolder();
			projectFolder.erase(projectFolder.begin() + projectFolder.size() - 1);
			
			// geting list of all files and folders in project folder
			auto fileList = getFolderList(projectList[projectChosen]->getProjectFolder().c_str());
			// we would delete all files in project folder, my editor would not create folders there
			// so we are deleting only files.
			for (size_t i = 0; i < fileList.size(); i++)
			{
				deleteFile((projectList[projectChosen]->getProjectFolder() + fileList[i]).c_str());
			}
			// then we can try to delete project folder, but if user created some folders in it we will fail.
			deleteFolder(projectFolder.c_str());

			for (size_t i = 0; i < projectList.size(); i++)
			{
				delete projectList[i];
			}
			projectList.clear();
			currentProject = nullptr;

			loadProjectList();
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Insert name of new project :");
			static char projectName[512] = "";

			ImGui::InputText("", projectName, IM_ARRAYSIZE(projectName));
			ImGui::Separator();

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				bool alreadyCreated = false;
				for (size_t i = 0; i < projectList.size(); i++)
				{
					if (projectList[i]->getName() == std::string(projectName))
					{
						alreadyCreated = true;
						break;
					}
				}

				if (strlen(projectName) != 0 && !alreadyCreated)
				{
					createFolder((std::string(PROJECTS_FOLDER) + std::string("/") + projectName + "/").c_str());
					projectList.push_back(new FEProject(projectName, std::string(PROJECTS_FOLDER) + std::string("/") + projectName + "/"));
					projectList.back()->createDummyScreenshot();
					FEScene::getInstance().addLight(FE_DIRECTIONAL_LIGHT, "sun");
					ImGui::CloseCurrentPopup();
					strcpy_s(projectName, "");
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

bool changeFileName(const char* filePath, const char* newName)
{
	int result = rename(filePath, newName);
	return result == 0 ? true : false;
}

bool deleteFile(const char* filePath)
{
	int result = remove(filePath);
	return result == 0 ? true : false;
}

void displayTextureInMaterialEditor(FETexture*& texture)
{
	if (texture == nullptr)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));

		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			selectTextureWindow.show(&texture);
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		return;
	}
		
	ImGui::Image((void*)(intptr_t)texture->getTextureID(), ImVec2(64, 64), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Change"))
	{
		selectTextureWindow.show(&texture);
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void displayTexturesContentBrowser()
{
	std::vector<std::string> textureList = FEResourceManager::getInstance().getTextureList();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	if (!isOpenContextMenuInContentBrowser) textureUnderMouse = -1;
	ImGui::Columns(3, "mycolumns3", false);
	for (size_t i = 0; i < textureList.size(); i++)
	{
		if (ImGui::ImageButton((void*)(intptr_t)FEResourceManager::getInstance().getTexture(textureList[i])->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			//
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) textureUnderMouse = i;
		}

		ImGui::Text(textureList[i].c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void displayGameModelContentBrowser()
{
	std::vector<std::string> gameModelList = FEResourceManager::getInstance().getGameModelList();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	if (!isOpenContextMenuInContentBrowser) gameModelUnderMouse = -1;
	ImGui::Columns(3, "mycolumns3", false);
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FETexture* gameModelPreviewTexture;
		if (gameModelPreviewTextures.find(gameModelList[i]) != gameModelPreviewTextures.end())
		{
			gameModelPreviewTexture = gameModelPreviewTextures[gameModelList[i]];
		}
		else
		{
			gameModelPreviewTexture = FEResourceManager::getInstance().noTexture;
			// if we somehow could not find gameModel preview, we will create it.
			createGameModelPreview(gameModelList[i]);
		}

		if (ImGui::ImageButton((void*)(intptr_t)gameModelPreviewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			//
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) gameModelUnderMouse = i;
		}

		ImGui::Text(gameModelList[i].c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void displayMeshesContentBrowser()
{
	std::vector<std::string> meshList = FEResourceManager::getInstance().getMeshList();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	if (!isOpenContextMenuInContentBrowser) meshUnderMouse = -1;
	ImGui::Columns(3, "mycolumns3", false);
	for (size_t i = 0; i < meshList.size(); i++)
	{
		FETexture* meshPreviewTexture;
		if (meshPreviewTextures.find(meshList[i]) != meshPreviewTextures.end())
		{
			meshPreviewTexture = meshPreviewTextures[meshList[i]];
		}
		else
		{
			meshPreviewTexture = FEResourceManager::getInstance().noTexture;
			// if we somehow could not find mesh preview, we will create it.
			createMeshPreview(meshList[i]);
		}

		if (ImGui::ImageButton((void*)(intptr_t)meshPreviewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			//
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) meshUnderMouse = i;
		}

		ImGui::Text(meshList[i].c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

#ifdef FE_WIN_32
	// open dialog staff
	std::string toString(PWSTR wString)
	{
		std::wstring wFileName = wString;
		std::string result;
		char *szTo = new char[wFileName.length() + 1];
		szTo[wFileName.size()] = '\0';
		WideCharToMultiByte(CP_ACP, 0, wFileName.c_str(), -1, szTo, (int)wFileName.length(), NULL, NULL);
		result = szTo;
		delete[] szTo;

		return result;
	}

	void openDialog(std::string& filePath, const COMDLG_FILTERSPEC* filter, int filterCount)
	{
		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr))
		{
			IFileOpenDialog *pFileOpen;
			// Create the FileOpenDialog object.
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

			if (SUCCEEDED(hr))
			{
				hr = pFileOpen->SetFileTypes(filterCount, filter);
				// Show the Open dialog box.
				hr = pFileOpen->Show(NULL);

				// Get the file name from the dialog box.
				if (SUCCEEDED(hr))
				{
					IShellItem *pItem;
					hr = pFileOpen->GetResult(&pItem);
					if (SUCCEEDED(hr))
					{
						PWSTR pszFilePath;
						hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

						// Display the file name to the user.
						if (SUCCEEDED(hr))
						{
							filePath = toString(pszFilePath);
							CoTaskMemFree(pszFilePath);
						}
						pItem->Release();
					}
				}
				pFileOpen->Release();
			}
			CoUninitialize();
		}
	}

#endif

void openProject(int projectIndex)
{
	currentProject = projectList[projectIndex];
	currentProject->loadScene();
	projectChosen = -1;

	// after loading project we should update our previews
	meshPreviewTextures.clear();
	std::vector<std::string> meshList = FEResourceManager::getInstance().getMeshList();
	for (size_t i = 0; i < meshList.size(); i++)
	{
		createMeshPreview(meshList[i]);
	}

	materialPreviewTextures.clear();
	std::vector<std::string> materialList = FEResourceManager::getInstance().getMaterialList();
	for (size_t i = 0; i < materialList.size(); i++)
	{
		createMaterialPreview(materialList[i]);
	}

	gameModelPreviewTextures.clear();
	std::vector<std::string> gameModelList = FEResourceManager::getInstance().getGameModelList();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		createGameModelPreview(gameModelList[i]);
	}
}

void createMeshPreview(std::string meshName)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();

	FEMesh* previewMesh = resourceManager.getMesh(meshName);

	if (previewMesh == nullptr)
		return;

	previewGameModel->mesh = previewMesh;
	previewGameModel->material = meshPreviewMaterial;

	previewFB->bind();
	//glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// saving currently used variables
	FocalEngine::FETransformComponent regularMeshTransform = previewEntity->transform;
	glm::vec3 regularCameraPosition = engine.getCamera()->getPosition();
	float regularAspectRation = engine.getCamera()->getAspectRatio();
	float regularCameraPitch = engine.getCamera()->getPitch();
	float regularCameraRoll = engine.getCamera()->getRoll();
	float regularCameraYaw = engine.getCamera()->getYaw();

	// transform has influence on AABB, so before any calculations setting needed values
	previewEntity->transform.setPosition(glm::vec3(0.0, 0.0, 0.0));
	previewEntity->transform.setScale(glm::vec3(1.0, 1.0, 1.0));
	previewEntity->transform.setRotation(glm::vec3(15.0, -15.0, 0.0));

	FEAABB meshAABB = previewEntity->getAABB();
	glm::vec3 min = meshAABB.getMin();
	glm::vec3 max = meshAABB.getMax();

	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	float zSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	previewEntity->transform.setPosition(-glm::vec3(max.x - xSize / 2.0f, max.y - ySize / 2.0f, max.z - zSize / 2.0f));
	engine.getCamera()->setPosition(glm::vec3(0.0, 0.0, std::max(std::max(xSize, ySize), zSize) * 1.75f));

	engine.getCamera()->setAspectRatio(1.0f);
	glViewport(0, 0, 128, 128);

	engine.getCamera()->setPitch(0.0f);
	engine.getCamera()->setRoll(0.0f);
	engine.getCamera()->setYaw(0.0f);

	// rendering mesh to texture
	FERenderer::getInstance().renderEntity(previewEntity, engine.getCamera());

	// reversing all of our transformating.
	previewEntity->transform = regularMeshTransform;

	engine.getCamera()->setPosition(regularCameraPosition);
	engine.getCamera()->setAspectRatio(regularAspectRation);
	engine.getCamera()->setPitch(regularCameraPitch);
	engine.getCamera()->setRoll(regularCameraRoll);
	engine.getCamera()->setYaw(regularCameraYaw);

	previewFB->unBind();

	meshPreviewTextures[meshName] = previewFB->getColorAttachment();
	previewFB->setColorAttachment(previewFB->getColorAttachment()->createSameFormatTexture());
}

FETexture* getMeshPreview(std::string meshName)
{
	// if we somehow could not find preview, we will create it.
	if (meshPreviewTextures.find(meshName) == meshPreviewTextures.end())
		createMeshPreview(meshName);

	// if still we don't have it
	if (meshPreviewTextures.find(meshName) == meshPreviewTextures.end())
		return FEResourceManager::getInstance().noTexture;

	return meshPreviewTextures[meshName];
}

void createMaterialPreview(std::string materialName)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();

	FEMaterial* previewMaterial = resourceManager.getMaterial(materialName);
	if (previewMaterial == nullptr)
		return;

	FEDirectionalLight* currentDirectionalLight = nullptr;
	std::vector<std::string> lightList = FEScene::getInstance().getLightsList();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		if (FEScene::getInstance().getLight(lightList[i])->getType() == FE_DIRECTIONAL_LIGHT)
		{
			currentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(FEScene::getInstance().getLight(lightList[i]));
			break;
		}
	}
	glm::vec3 regularLightRotation = currentDirectionalLight->transform.getRotation();
	currentDirectionalLight->transform.setRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	float regularLightIntensity = currentDirectionalLight->getIntensity();
	currentDirectionalLight->setIntensity(5.0f);

	previewGameModel->mesh = resourceManager.getMesh("sphere");
	previewGameModel->material = previewMaterial;
	previewEntity->setReceivingShadows(false);

	previewFB->bind();
	//glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// saving currently used variables
	glm::vec3 regularCameraPosition = engine.getCamera()->getPosition();
	float regularAspectRation = engine.getCamera()->getAspectRatio();
	float regularCameraPitch = engine.getCamera()->getPitch();
	float regularCameraRoll = engine.getCamera()->getRoll();
	float regularCameraYaw = engine.getCamera()->getYaw();
	float regularExposure = engine.getCamera()->getExposure();
	engine.getCamera()->setExposure(1.0f);

	previewEntity->transform.setPosition(glm::vec3(0.0, 0.0, 0.0));
	previewEntity->transform.setScale(glm::vec3(1.0, 1.0, 1.0));
	previewEntity->transform.setRotation(glm::vec3(0.0, 0.0, 0.0));

	engine.getCamera()->setPosition(glm::vec3(0.0, 0.0, 3.0f));
	engine.getCamera()->setAspectRatio(1.0f);
	glViewport(0, 0, 128, 128);

	engine.getCamera()->setPitch(0.0f);
	engine.getCamera()->setRoll(0.0f);
	engine.getCamera()->setYaw(0.0f);

	// rendering material to texture
	FERenderer::getInstance().renderEntity(previewEntity, engine.getCamera(), true);

	engine.getCamera()->setPosition(regularCameraPosition);
	engine.getCamera()->setAspectRatio(regularAspectRation);
	engine.getCamera()->setPitch(regularCameraPitch);
	engine.getCamera()->setRoll(regularCameraRoll);
	engine.getCamera()->setYaw(regularCameraYaw);
	engine.getCamera()->setExposure(regularExposure);

	currentDirectionalLight->transform.setRotation(regularLightRotation);
	currentDirectionalLight->setIntensity(regularLightIntensity);

	previewFB->unBind();

	materialPreviewTextures[materialName] = previewFB->getColorAttachment();
	previewFB->setColorAttachment(previewFB->getColorAttachment()->createSameFormatTexture());
}

FETexture* getMaterialPreview(std::string materialName)
{
	// if we somehow could not find preview, we will create it.
	if (materialPreviewTextures.find(materialName) == materialPreviewTextures.end())
		createMeshPreview(materialName);

	// if still we don't have it
	if (materialPreviewTextures.find(materialName) == materialPreviewTextures.end())
		return FEResourceManager::getInstance().noTexture;

	return materialPreviewTextures[materialName];
}

void createGameModelPreview(std::string gameModelName)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();

	FEGameModel* gameModel = resourceManager.getGameModel(gameModelName);

	if (gameModel == nullptr)
		return;

	FEDirectionalLight* currentDirectionalLight = nullptr;
	std::vector<std::string> lightList = FEScene::getInstance().getLightsList();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		if (FEScene::getInstance().getLight(lightList[i])->getType() == FE_DIRECTIONAL_LIGHT)
		{
			currentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(FEScene::getInstance().getLight(lightList[i]));
			break;
		}
	}
	glm::vec3 regularLightRotation = currentDirectionalLight->transform.getRotation();
	currentDirectionalLight->transform.setRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	float regularLightIntensity = currentDirectionalLight->getIntensity();
	currentDirectionalLight->setIntensity(5.0f);

	previewGameModel->mesh = gameModel->mesh;
	previewGameModel->material = gameModel->material;
	previewEntity->setReceivingShadows(false);

	previewFB->bind();
	//glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// saving currently used variables
	FocalEngine::FETransformComponent regularMeshTransform = previewEntity->transform;
	glm::vec3 regularCameraPosition = engine.getCamera()->getPosition();
	float regularAspectRation = engine.getCamera()->getAspectRatio();
	float regularCameraPitch = engine.getCamera()->getPitch();
	float regularCameraRoll = engine.getCamera()->getRoll();
	float regularCameraYaw = engine.getCamera()->getYaw();
	float regularExposure = engine.getCamera()->getExposure();
	engine.getCamera()->setExposure(1.0f);
	
	// transform has influence on AABB, so before any calculations setting needed values
	previewEntity->transform.setPosition(glm::vec3(0.0, 0.0, 0.0));
	previewEntity->transform.setScale(glm::vec3(1.0, 1.0, 1.0));
	previewEntity->transform.setRotation(glm::vec3(15.0, -15.0, 0.0));

	FEAABB meshAABB = previewEntity->getAABB();
	glm::vec3 min = meshAABB.getMin();
	glm::vec3 max = meshAABB.getMax();

	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	float zSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	previewEntity->transform.setPosition(-glm::vec3(max.x - xSize / 2.0f, max.y - ySize / 2.0f, max.z - zSize / 2.0f));
	engine.getCamera()->setPosition(glm::vec3(0.0, 0.0, std::max(std::max(xSize, ySize), zSize) * 1.75f));

	engine.getCamera()->setAspectRatio(1.0f);
	glViewport(0, 0, 128, 128);

	engine.getCamera()->setPitch(0.0f);
	engine.getCamera()->setRoll(0.0f);
	engine.getCamera()->setYaw(0.0f);

	// rendering game model to texture
	FERenderer::getInstance().renderEntity(previewEntity, engine.getCamera(), true);

	// reversing all of our transformating.
	previewEntity->transform = regularMeshTransform;

	engine.getCamera()->setPosition(regularCameraPosition);
	engine.getCamera()->setAspectRatio(regularAspectRation);
	engine.getCamera()->setPitch(regularCameraPitch);
	engine.getCamera()->setRoll(regularCameraRoll);
	engine.getCamera()->setYaw(regularCameraYaw);
	engine.getCamera()->setExposure(regularExposure);

	currentDirectionalLight->transform.setRotation(regularLightRotation);
	currentDirectionalLight->setIntensity(regularLightIntensity);

	previewFB->unBind();

	gameModelPreviewTextures[gameModelName] = previewFB->getColorAttachment();
	previewFB->setColorAttachment(previewFB->getColorAttachment()->createSameFormatTexture());
}

void createGameModelPreview(FEGameModel* gameModel, FETexture** resultingTexture)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();

	if (gameModel == nullptr)
		return;

	FEDirectionalLight* currentDirectionalLight = nullptr;
	std::vector<std::string> lightList = FEScene::getInstance().getLightsList();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		if (FEScene::getInstance().getLight(lightList[i])->getType() == FE_DIRECTIONAL_LIGHT)
		{
			currentDirectionalLight = reinterpret_cast<FEDirectionalLight*>(FEScene::getInstance().getLight(lightList[i]));
			break;
		}
	}
	glm::vec3 regularLightRotation = currentDirectionalLight->transform.getRotation();
	currentDirectionalLight->transform.setRotation(glm::vec3(-40.0f, 10.0f, 0.0f));

	float regularLightIntensity = currentDirectionalLight->getIntensity();
	currentDirectionalLight->setIntensity(5.0f);

	previewGameModel->mesh = gameModel->mesh;
	previewGameModel->material = gameModel->material;
	previewEntity->setReceivingShadows(false);

	if (*resultingTexture == nullptr)
		*resultingTexture = previewFB->getColorAttachment()->createSameFormatTexture();
	FETexture* tempTexture = previewFB->getColorAttachment();
	previewFB->setColorAttachment(*resultingTexture);
	previewFB->bind();
	//glClearColor(0.55f, 0.73f, 0.87f, 1.0f);
	FE_GL_ERROR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// saving currently used variables
	FocalEngine::FETransformComponent regularMeshTransform = previewEntity->transform;
	glm::vec3 regularCameraPosition = engine.getCamera()->getPosition();
	float regularAspectRation = engine.getCamera()->getAspectRatio();
	float regularCameraPitch = engine.getCamera()->getPitch();
	float regularCameraRoll = engine.getCamera()->getRoll();
	float regularCameraYaw = engine.getCamera()->getYaw();
	float regularExposure = engine.getCamera()->getExposure();
	engine.getCamera()->setExposure(1.0f);

	// transform has influence on AABB, so before any calculations setting needed values
	previewEntity->transform.setPosition(glm::vec3(0.0, 0.0, 0.0));
	previewEntity->transform.setScale(glm::vec3(1.0, 1.0, 1.0));
	previewEntity->transform.setRotation(glm::vec3(15.0, -15.0, 0.0));

	FEAABB meshAABB = previewEntity->getAABB();
	glm::vec3 min = meshAABB.getMin();
	glm::vec3 max = meshAABB.getMax();

	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	float zSize = sqrt((max.z - min.z) * (max.z - min.z));

	// invert center point and it will be exactly how much we need to translate mesh in order to place it in origin.
	previewEntity->transform.setPosition(-glm::vec3(max.x - xSize / 2.0f, max.y - ySize / 2.0f, max.z - zSize / 2.0f));
	engine.getCamera()->setPosition(glm::vec3(0.0, 0.0, std::max(std::max(xSize, ySize), zSize) * 1.75f));

	engine.getCamera()->setAspectRatio(1.0f);
	glViewport(0, 0, 128, 128);

	engine.getCamera()->setPitch(0.0f);
	engine.getCamera()->setRoll(0.0f);
	engine.getCamera()->setYaw(0.0f);

	// rendering game model to texture
	FERenderer::getInstance().renderEntity(previewEntity, engine.getCamera(), true);

	// reversing all of our transformating.
	previewEntity->transform = regularMeshTransform;

	engine.getCamera()->setPosition(regularCameraPosition);
	engine.getCamera()->setAspectRatio(regularAspectRation);
	engine.getCamera()->setPitch(regularCameraPitch);
	engine.getCamera()->setRoll(regularCameraRoll);
	engine.getCamera()->setYaw(regularCameraYaw);
	engine.getCamera()->setExposure(regularExposure);

	currentDirectionalLight->transform.setRotation(regularLightRotation);
	currentDirectionalLight->setIntensity(regularLightIntensity);

	previewFB->unBind();

	previewFB->setColorAttachment(tempTexture);
}

FETexture* getGameModelPreview(std::string gameModelName)
{
	// if we somehow could not find preview, we will create it.
	if (gameModelPreviewTextures.find(gameModelName) == gameModelPreviewTextures.end())
		createMeshPreview(gameModelName);

	// if still we don't have it
	if (gameModelPreviewTextures.find(gameModelName) == gameModelPreviewTextures.end())
		return FEResourceManager::getInstance().noTexture;

	return gameModelPreviewTextures[gameModelName];
}