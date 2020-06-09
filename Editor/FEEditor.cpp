#include "../Editor/FEEditor.h"

void mouseButtonCallback(int button, int action, int mods)
{
	if ((!ImGui::GetIO().WantCaptureMouse))
	{
		if (entityUnderMouse.second != FLT_MAX)
		{
			selectedEntity = entityUnderMouse.first;
			selectedEntityWasChanged = true;
		}
		else
		{
			selectedEntity = "";
		}
	}
}

std::string getSelectedEntity()
{
	return selectedEntity;
}

void keyButtonCallback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		isCameraInputActive = !isCameraInputActive;
		FEngine::getInstance().getCamera()->setIsInputActive(isCameraInputActive);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		FEngine::getInstance().terminate();
	}

	if (key == GLFW_KEY_DELETE)
	{
		if (selectedEntity != "")
			FEScene::getInstance().deleteEntity(selectedEntity);
	}

	if (mods == GLFW_MOD_CONTROL && key == GLFW_KEY_C)
	{
		clipboardEntity = selectedEntity;
	}

	if (mods == GLFW_MOD_CONTROL && key == GLFW_KEY_V)
	{
		if (clipboardEntity != "")
		{
			FEScene& Scene = FEScene::getInstance();
			FEEntity* newEntity = Scene.addEntity(Scene.getEntity(clipboardEntity)->mesh, Scene.getEntity(clipboardEntity)->material, "");
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
		glm::vec3 d = light->getDirection();
		ImGui::DragFloat("##x", &d[0], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##y", &d[1], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##z", &d[2], 0.01f, 0.0f, 1.0f);
	}
	else if (light->getType() == FE_POINT_LIGHT)
	{
	}
	else if (light->getType() == FE_SPOT_LIGHT)
	{
		glm::vec3 d = light->getDirection();
		ImGui::DragFloat("##x", &d[0], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##y", &d[1], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##z", &d[2], 0.01f, 0.0f, 1.0f);

		float spotAngle = light->getSpotAngle();
		ImGui::SliderFloat((std::string("Inner angle##") + light->getName()).c_str(), &spotAngle, 0.0f, 90.0f);
		light->setSpotAngle(spotAngle);

		float spotAngleOuter = light->getSpotAngleOuter();
		ImGui::SliderFloat((std::string("Outer angle ##") + light->getName()).c_str(), &spotAngleOuter, 0.0f, 90.0f);
		light->setSpotAngleOuter(spotAngleOuter);
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
		FEScene::getInstance().addEntity(resourceManager.getMesh("cube"));
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
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
		ImGui::GetStateStorage()->SetInt(ImGui::GetID(entityUnderMouse.first.c_str()), 1);
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

			std::string meshText = "Mesh name : ";
			meshText += entity->mesh->getName();
			ImGui::Text(meshText.c_str());
			ImGui::SameLine();

			if (ImGui::Button("Change Mesh"))
			{
				selectMeshWindow.show(&entity->mesh);
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			static std::string currentMaterial = "";
			currentMaterial = entity->material->getName();
			if (ImGui::BeginCombo("Materials", currentMaterial.c_str(), ImGuiWindowFlags_None))
			{
				for (size_t n = 0; n < materialList.size(); n++)
				{
					bool is_selected = (currentMaterial == materialList[n]);
					if (ImGui::Selectable(materialList[n].c_str(), is_selected))
					{
						currentMaterial = materialList[n].c_str();
						entity->material = FEResourceManager::getInstance().getMaterial(materialList[n]);
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));

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

			ImGui::Text("albedoMap:");
			displayTextureInMaterialEditor(material->albedoMap);
			ImGui::Text("normalMap:");
			displayTextureInMaterialEditor(material->normalMap);
			ImGui::Text("roughtnessMap:");
			displayTextureInMaterialEditor(material->roughtnessMap);
			ImGui::Text("metalnessMap:");
			displayTextureInMaterialEditor(material->metalnessMap);
			ImGui::Text("AOMap:");
			displayTextureInMaterialEditor(material->AOMap);
			ImGui::Text("displacementMap:");
			displayTextureInMaterialEditor(material->displacementMap);
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
	std::vector<std::string> entitiesList = FEScene::getInstance().getEntityList();

	for (size_t i = 0; i < entitiesList.size(); i++)
	{
		FEEntity* currentEntity = FEScene::getInstance().getEntity(entitiesList[i]);

		if (currentEntity->mesh == mesh)
			result++;
	}

	return result;
}

int textureUnderMouse = -1;
bool isOpenContextMenuInContentBrowser = false;
int meshUnderMouse = -1;
static int activeTabContentBrowser = 0;

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

			if (ImGui::BeginTabItem("PostProcess"))
			{
				activeTabContentBrowser = 3;
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
						loadMeshWindow.show();
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
					if (ImGui::MenuItem("Load new texture..."))
					{
						loadTextureWindow.show();
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
					break;
				}
				default:
					break;
			}
			ImGui::EndPopup();
		}
	ImGui::End();
	
	loadMeshWindow.render();
	loadTextureWindow.render();
	renameMeshWindow.render();
	renameTextureWindow.render();
	renameFailedWindow.render();
	deleteTextureWindow.render();
	deleteMeshWindow.render();
}

glm::dvec3 mouseRay()
{
	glm::dvec2 normalizedMouseCoords;
	normalizedMouseCoords.x = (2.0f * mouseX) / FEngine::getInstance().getWindowWidth() - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * mouseY) / FEngine::getInstance().getWindowHeight();

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
	std::vector<std::string> entityList = FEScene::getInstance().getEntityList();
	entityUnderMouse.second = FLT_MAX;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		float dis = 0;
		FEAABB box = FEScene::getInstance().getEntity(entityList[i])->getAABB();

		if (box.rayIntersect(FEngine::getInstance().getCamera()->getPosition(), mouseRay(), dis))
		{
			if (entityUnderMouse.second > dis)
			{
				entityUnderMouse.first = entityList[i];
				entityUnderMouse.second = dis;
			}
		}
	}
}

void mouseMoveCallback(double xpos, double ypos)
{
	mouseX = xpos;
	mouseY = ypos;

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
	}
	else
	{
		displayProjectSelection();
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

			/*if (ImGui::IsMouseDoubleClicked(0) && projectChosen != -1)
			{
				currentProject = projectList[i];
				currentProject->loadScene();
			}*/

			if (ImGui::ImageButton((void*)(intptr_t)/*testTexture->getTextureID()*/projectList[i]->sceneScreenshot->getTextureID(), ImVec2(512.0f, 288.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
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
			currentProject = projectList[projectChosen];
			currentProject->loadScene();
			projectChosen = -1;
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
		return;

	ImGui::Image((void*)(intptr_t)texture->getTextureID(), ImVec2(64, 64));

	// I am using texture->getTextureID as a unique ID for ImGui correct work.
	ImGui::PushID(texture->getTextureID());
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Change"))
	{
		selectTextureWindow.show(&texture);
		//ImGui::OpenPopup("ChangeTexture");
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PopID();

	// old representation of textures list
	//std::vector<std::string> allTextures = FEResourceManager::getInstance().getTextureList();
	//if (ImGui::BeginCombo("Textures", texture->getName().c_str(), ImGuiWindowFlags_None))
	//{
	//	for (size_t n = 0; n < allTextures.size(); n++)
	//	{
	//		bool is_selected = (texture->getName() == allTextures[n]);
	//		if (ImGui::Selectable(("##" + std::to_string(n)).c_str(), is_selected, 0, ImVec2(0, 64)))
	//		{
	//			texture = FEResourceManager::getInstance().getTexture(allTextures[n]);
	//		}
	//		ImGui::SameLine();
	//		ImGui::Image((void*)(intptr_t)FEResourceManager::getInstance().getTexture(allTextures[n])->getTextureID(), ImVec2(64, 64));
	//		ImGui::SameLine();
	//		ImGui::Text(allTextures[n].c_str());

	//		if (is_selected)
	//			ImGui::SetItemDefaultFocus();
	//	}
	//	ImGui::EndCombo();

	//	if (ImGui::BeginPopupModal("ChangeTexture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	//	{
	//		ImGui::Text("Insert texture file path :");
	//		static char filePath[512] = "";

	//		ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
	//		ImGui::Separator();

	//		if (ImGui::Button("Load", ImVec2(120, 0)))
	//		{
	//			//material->setTexture(FEResourceManager::getInstance().LoadPngTexture(filePath, "", (currentProject->getProjectFolder() + material->getName() + textures[j] + ".texture").c_str()), textures[j]);
	//			ImGui::CloseCurrentPopup();
	//			strcpy_s(filePath, "");
	//		}
	//		ImGui::SetItemDefaultFocus();
	//		ImGui::SameLine();
	//		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
	//		ImGui::EndPopup();
	//	}

	//	ImGui::PopID();
	//}
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
		if (ImGui::ImageButton((void*)(intptr_t)FEResourceManager::getInstance().noTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
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