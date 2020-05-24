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
		FEScene::getInstance().addEntity(resourceManager.getSimpleMesh("cube"));
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
		FEngine::getInstance().takeScreenshot((currentProject->getProjectFolder() + "projectScreenShot.texture").c_str());
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
			ImGui::PushID(i);
			showTransformConfiguration(entity->getName(), &entity->transform);

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
						entity->mesh = FEResourceManager::getInstance().getSimpleMesh(filePath);
						if (!entity->mesh)
							entity->mesh = FEResourceManager::getInstance().LoadOBJMesh(filePath, "", currentProject->getProjectFolder().c_str());
						ImGui::CloseCurrentPopup();
						strcpy_s(filePath, "");
					}
					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();
				}
			}

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
}

void displayMaterialEditor()
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
		static char filePath[256] = "";

		ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
		ImGui::Separator();

		if (ImGui::Button("Create", ImVec2(120, 0)))
		{
			FEMaterial* newMat = FEResourceManager::getInstance().createMaterial(filePath);
			if (newMat)
			{
				newMat->shader = new FEShader(FEPhongVS, FEPhongFS);
				FETexture* colorMap = FEResourceManager::getInstance().LoadPngTexture("C:/Users/kandr/Downloads/OpenGL test/resources/empty.png");
				colorMap->setName("color map");
				FETexture* normalMap = FEResourceManager::getInstance().LoadPngTexture("C:/Users/kandr/Downloads/OpenGL test/resources/empty.png");
				normalMap->setName("normal map");
				//FETexture* roughnessMap = FEResourceManager::getInstance().createTexture("C:/Users/kandr/Downloads/OpenGL test/resources/empty.png");
				//normalMap->setName("roughness map");

				newMat->addTexture(colorMap);
				newMat->addTexture(normalMap);
				//newMat->addTexture(roughnessMap);
			}

			ImGui::CloseCurrentPopup();
			strcpy_s(filePath, "");
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Save materials", ImVec2(105, 0)))
		ImGui::OpenPopup("Save material from");

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupModal("Save material from", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Insert path to material file :");
		static char filePath[256] = "";

		ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
		ImGui::Separator();

		if (ImGui::Button("Save", ImVec2(120, 0)))
		{
			saveMaterials(filePath);

			ImGui::CloseCurrentPopup();
			strcpy_s(filePath, "");
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Load materials", ImVec2(105, 0)))
		ImGui::OpenPopup("Load material from");

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupModal("Load material from", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Insert path to material file :");
		static char filePath[256] = "";

		ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
		ImGui::Separator();

		if (ImGui::Button("Load", ImVec2(120, 0)))
		{
			loadMaterials(filePath);

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

			std::vector<std::string> textures = material->getTextureList();
			for (size_t j = 0; j < textures.size(); j++)
			{
				ImGui::Text(textures[j].c_str());
				ImGui::Image((void*)(intptr_t)material->getTexture(textures[j])->getTextureID(), ImVec2(32, 32));

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
						material->setTexture(FEResourceManager::getInstance().LoadPngTexture(filePath, "", (currentProject->getProjectFolder() + material->getName() + textures[j] + ".texture").c_str()), textures[j]);
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
	}
}

void displayPostProcess()
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

void displayContentBrowser()
{
	float mainWindowW = float(FEngine::getInstance().getWindowWidth());
	float mainWindowH = float(FEngine::getInstance().getWindowHeight());
	float windowW = mainWindowW / 3.7f;
	float windowH = mainWindowH;

	ImGui::SetNextWindowPos(ImVec2(mainWindowW - windowW, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(windowW, windowH));
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_None);
		int activeTab = 0;
		if (ImGui::BeginTabBar("##Content Browser", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Textures"))
			{
				activeTab = 0;
				std::vector<std::string> textureList = FEResourceManager::getInstance().getTextureList();

				ImGui::Columns(3, "mycolumns3", false);
				for (size_t i = 0; i < textureList.size(); i++)
				{
					ImGui::Image((void*)(intptr_t)FEResourceManager::getInstance().getTexture(textureList[i])->getTextureID(), ImVec2(128, 128));
					ImGui::Text(textureList[i].c_str());
					ImGui::NextColumn();
				}
				ImGui::Columns(1);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Materials"))
			{
				activeTab = 1;
				displayMaterialEditor();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("PostProcess"))
			{
				activeTab = 2;
				displayPostProcess();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		bool open_context_menu = false;
		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
			open_context_menu = true;

		if (open_context_menu)
			ImGui::OpenPopup("##context_menu");

		bool shouldOpenLoadTextPopUp = false;
		if (ImGui::BeginPopup("##context_menu"))
		{
			switch (activeTab)
			{
				case 0:
				{
					if (ImGui::MenuItem("Load new texture..."))
					{
						shouldOpenLoadTextPopUp = true;
					}
					break;
				}
				case 1:
				{
					if (ImGui::MenuItem("Create new material..."))
					{
					}
					break;
				}
				case 2:
				{
					break;
				}
				default:
					break;
			}
			ImGui::EndPopup();
		}
	ImGui::End();

	if (shouldOpenLoadTextPopUp)
		ImGui::OpenPopup("Load Texture");

	if (ImGui::BeginPopupModal("Load Texture", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Insert texture file path :");
		static char filePath[256] = "";

		ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
		ImGui::Separator();

		if (ImGui::Button("Load", ImVec2(120, 0)))
		{
			FEResourceManager::getInstance().LoadPngTexture(filePath);
			ImGui::CloseCurrentPopup();
			strcpy_s(filePath, "");
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
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

void saveMaterials(const char* fileName)
{
	std::vector<std::string> materialList = FEResourceManager::getInstance().getMaterialList();
	std::ofstream materialFile;
	!fileName ? materialFile.open("materials.txt") : materialFile.open(fileName);

	Json::Value root;
	Json::Value data;
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FEMaterial* mat = FEResourceManager::getInstance().getMaterial(materialList[i]);
		// to-do: for now we will save only one type of material. But we need to implement robust load/save for all types.
		if (mat->getTextureList().size() == 0)
			continue;

		data[mat->getName()]["name"] = mat->getName();

		std::vector<std::string> textureList = mat->getTextureList();
		for (size_t j = 0; j < textureList.size(); j++)
		{
			data[mat->getName()]["textures"][textureList[j]]["file"] = mat->getTexture(textureList[j])->getFileName();
		}
	}
	root["materials"] = data;

	Json::StreamWriterBuilder builder;
	const std::string json_file = Json::writeString(builder, root);

	materialFile << json_file;
	materialFile.close();
}

void loadMaterials(const char* fileName)
{
	FEResourceManager& resourceManager = FEResourceManager::getInstance();

	std::ifstream materialFile;
	!fileName ? materialFile.open("materials.txt") : materialFile.open(fileName);

	std::string fileData((std::istreambuf_iterator<char>(materialFile)), std::istreambuf_iterator<char>());

	Json::Value root;
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	if (!reader->parse(fileData.c_str(), fileData.c_str() + fileData.size(), &root, &err))
		std::cout << "error" << std::endl;

	std::vector<Json::String> materialsList = root["materials"].getMemberNames();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FEMaterial* newMat = resourceManager.createMaterial(materialsList[i].c_str());
		newMat->shader = new FEShader(FEPhongVS, FEPhongFS);

		std::vector<Json::String> textureList = root["materials"][materialsList[i]]["textures"].getMemberNames();
		for (size_t j = 0; j < textureList.size(); j++)
		{
			FETexture* texture = resourceManager.LoadFETexture((std::string(PROJECTS_FOLDER "/StartScene/") + root["materials"][materialsList[i]]["textures"][textureList[j]]["file"].asCString()).c_str());
			texture->setName(textureList[j]);
			newMat->addTexture(texture);
		}
	}

	materialFile.close();
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
	return (_rmdir(dirPath) != 0);
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

			if (ImGui::ImageButton((void*)(intptr_t)projectList[i]->sceneScreenshot->getTextureID(), ImVec2(512.0f, 288.0f), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
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
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete Project", ImVec2(200.0f, 64.0f)) && projectChosen != -1)
		{
			bool f = deleteFolder(projectList[projectChosen]->getProjectFolder().c_str());
			loadEditor();

			/*ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::End();
			ImGui::PopStyleVar();

			return;*/
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		if (ImGui::BeginPopupModal("New Project", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Insert name of new project :");
			static char projectName[256] = "";

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