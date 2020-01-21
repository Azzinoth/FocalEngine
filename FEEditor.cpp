#include "FEEditor.h"

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
		FocalEngine::FEngine::getInstance().getCamera()->setIsInputActive(isCameraInputActive);
	}

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		FocalEngine::FEngine::getInstance().terminate();
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

void showTransformConfiguration(std::string objectName, FocalEngine::FETransformComponent* transform)
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
		ImGui::DragFloat(param->name.c_str(), &fData, 0.1f, 0.0f, 100.0f);
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

void displayMaterialPrameters(FocalEngine::FEMaterial* material)
{
	static std::string currentMaterial = "";
	std::vector<std::string> materialList = FocalEngine::FEResourceManager::getInstance().getMaterialList();
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

void displayLightProperties(FocalEngine::FELight* light)
{
	showTransformConfiguration(light->getName(), &light->transform);

	if (light->getType() == FocalEngine::FE_DIRECTIONAL_LIGHT)
	{
		glm::vec3 d = light->getDirection();
		ImGui::DragFloat("##x", &d[0], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##y", &d[1], 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("##z", &d[2], 0.01f, 0.0f, 1.0f);
	}
	else if (light->getType() == FocalEngine::FE_POINT_LIGHT)
	{
	}
	else if (light->getType() == FocalEngine::FE_SPOT_LIGHT)
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
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();
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
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Add new entity", ImVec2(220, 0)))
	{
		FocalEngine::FEScene::getInstance().addEntity(resourceManager.getSimpleMesh("cube"));
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void displaySceneEntities()
{
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();
	std::vector<std::string> entityList = scene.getEntityList();
	std::vector<std::string> materialList = FocalEngine::FEResourceManager::getInstance().getMaterialList();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(FocalEngine::FEngine::getInstance().getWindowWidth() / 3.7f, float(FocalEngine::FEngine::getInstance().getWindowHeight())));
	ImGui::Begin("Scene Entities", nullptr, ImGuiWindowFlags_None);
	addEntityButton();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Save scene", ImVec2(220, 0)))
	{
		saveScene();
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Load scene", ImVec2(220, 0)))
	{
		ImGui::OpenPopup("LoadScene");
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupModal("LoadScene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Insert scene file path :");
		static char filePath[256] = "";

		ImGui::InputText("", filePath, IM_ARRAYSIZE(filePath));
		ImGui::Separator();

		if (ImGui::Button("Load", ImVec2(120, 0)))
		{
			entityList.clear();
			materialList.clear();

			scene.clear();
			FocalEngine::FEResourceManager::getInstance().clear();

			loadScene(filePath);
			ImGui::CloseCurrentPopup();
			strcpy_s(filePath, "");
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

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
		FocalEngine::FEEntity* entity = scene.getEntity(entityList[i]);
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
						entity->mesh = FocalEngine::FEResourceManager::getInstance().getSimpleMesh(filePath);
						if (!entity->mesh)
							entity->mesh = FocalEngine::FEResourceManager::getInstance().LoadOBJMesh(filePath);
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
						entity->material = FocalEngine::FEResourceManager::getInstance().getMaterial(materialList[n]);
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

	float FEExposure = FocalEngine::FEngine::getInstance().getCamera()->getExposure();
	ImGui::DragFloat("Camera Exposure", &FEExposure, 0.01f, 0.001f, 100.0f);
	FocalEngine::FEngine::getInstance().getCamera()->setExposure(FEExposure);

	ImGui::End();
}

void displayMaterialEditor()
{
	std::vector<std::string> materialList = FocalEngine::FEResourceManager::getInstance().getMaterialList();

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
			FocalEngine::FEMaterial* newMat = FocalEngine::FEResourceManager::getInstance().createMaterial(filePath);
			if (newMat)
			{
				newMat->shader = new FocalEngine::FEShader(FEPhongVS, FEPhongFS);
				FocalEngine::FETexture* colorMap = FocalEngine::FEResourceManager::getInstance().LoadPngTexture("C:/Users/kandr/Downloads/OpenGL test/resources/empty.png");
				colorMap->setName("color map");
				FocalEngine::FETexture* normalMap = FocalEngine::FEResourceManager::getInstance().LoadPngTexture("C:/Users/kandr/Downloads/OpenGL test/resources/empty.png");
				normalMap->setName("normal map");
				//FocalEngine::FETexture* roughnessMap = FocalEngine::FEResourceManager::getInstance().createTexture("C:/Users/kandr/Downloads/OpenGL test/resources/empty.png");
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
		FocalEngine::FEMaterial* material = FocalEngine::FEResourceManager::getInstance().getMaterial(materialList[i]);

		if (ImGui::CollapsingHeader(materialList[i].c_str(), 0))
		{
			std::vector<std::string> params = material->getParameterList();
			FocalEngine::FEShaderParam* param;
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
						material->setTexture(FocalEngine::FEResourceManager::getInstance().LoadPngTexture(filePath), textures[j]);
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
	FocalEngine::FERenderer& renderer = FocalEngine::FERenderer::getInstance();
	std::vector<std::string> postProcessList = renderer.getPostProcessList();

	for (size_t i = 0; i < postProcessList.size(); i++)
	{
		FocalEngine::FEPostProcess* PPEffect = renderer.getPostProcessEffect(postProcessList[i]);
		if (ImGui::CollapsingHeader(PPEffect->getName().c_str(), 0)) //ImGuiTreeNodeFlags_DefaultOpen
		{
			for (size_t j = 0; j < PPEffect->stages.size(); j++)
			{
				ImGui::PushID(j);
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
}

void displayContentBrowser()
{
	float mainWindowW = float(FocalEngine::FEngine::getInstance().getWindowWidth());
	float mainWindowH = float(FocalEngine::FEngine::getInstance().getWindowHeight());
	float windowW = mainWindowW / 3.7f;
	float windowH = mainWindowH;

	ImGui::SetNextWindowPos(ImVec2(mainWindowW - windowW, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(windowW, windowH));
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_None); // ImGuiWindowFlags_NoCollapse
		int activeTab = 0;
		if (ImGui::BeginTabBar("##Content Browser", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Textures"))
			{
				activeTab = 0;
				std::vector<std::string> textureList = FocalEngine::FEResourceManager::getInstance().getTextureList();

				ImGui::Columns(3, "mycolumns3", false);
				for (size_t i = 0; i < textureList.size(); i++)
				{
					ImGui::Image((void*)(intptr_t)FocalEngine::FEResourceManager::getInstance().getTexture(textureList[i])->getTextureID(), ImVec2(128, 128));
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
			FocalEngine::FEResourceManager::getInstance().LoadPngTexture(filePath);
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
	normalizedMouseCoords.x = (2.0f * mouseX) / FocalEngine::FEngine::getInstance().getWindowWidth() - 1;
	normalizedMouseCoords.y = 1.0f - (2.0f * mouseY) / FocalEngine::FEngine::getInstance().getWindowHeight();

	glm::dvec4 clipCoords = glm::dvec4(normalizedMouseCoords.x, normalizedMouseCoords.y, -1.0, 1.0);
	glm::dvec4 eyeCoords = glm::inverse(FocalEngine::FEngine::getInstance().getCamera()->getProjectionMatrix()) * clipCoords;
	eyeCoords.z = -1.0f;
	eyeCoords.w = 0.0f;
	glm::dvec3 worldRay = glm::inverse(FocalEngine::FEngine::getInstance().getCamera()->getViewMatrix()) * eyeCoords;
	worldRay = glm::normalize(worldRay);

	return worldRay;
}

void determineEntityUnderMouse()
{
	std::vector<std::string> entityList = FocalEngine::FEScene::getInstance().getEntityList();
	entityUnderMouse.second = FLT_MAX;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		float dis = 0;
		FocalEngine::FEAABB box = FocalEngine::FEScene::getInstance().getEntity(entityList[i])->getAABB();

		if (box.rayIntersect(FocalEngine::FEngine::getInstance().getCamera()->getPosition(), mouseRay(), dis))
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
	std::vector<std::string> materialList = FocalEngine::FEResourceManager::getInstance().getMaterialList();
	std::ofstream materialFile;
	!fileName ? materialFile.open("materials.txt") : materialFile.open(fileName);

	Json::Value root;
	Json::Value data;
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FocalEngine::FEMaterial* mat = FocalEngine::FEResourceManager::getInstance().getMaterial(materialList[i]);
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
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();

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
		FocalEngine::FEMaterial* newMat = resourceManager.createMaterial(materialsList[i].c_str());
		newMat->shader = new FocalEngine::FEShader(FEPhongVS, FEPhongFS);

		std::vector<Json::String> textureList = root["materials"][materialsList[i]]["textures"].getMemberNames();
		for (size_t j = 0; j < textureList.size(); j++)
		{
			FocalEngine::FETexture* texture = resourceManager.LoadFETexture((std::string(PROJECTS_FOLDER "/StartScene/") + root["materials"][materialsList[i]]["textures"][textureList[j]]["file"].asCString()).c_str());
			texture->setName(textureList[j]);
			newMat->addTexture(texture);
		}
	}

	materialFile.close();
}

void writeTransformToJSON(Json::Value& root, FocalEngine::FETransformComponent* transform)
{
	root["position"]["X"] = transform->getPosition()[0];
	root["position"]["Y"] = transform->getPosition()[1];
	root["position"]["Z"] = transform->getPosition()[2];
	root["rotation"]["X"] = transform->getRotation()[0];
	root["rotation"]["Y"] = transform->getRotation()[1];
	root["rotation"]["Z"] = transform->getRotation()[2];
	root["scale"]["uniformScaling"] = transform->uniformScaling;
	root["scale"]["X"] = transform->getScale()[0];
	root["scale"]["Y"] = transform->getScale()[1];
	root["scale"]["Z"] = transform->getScale()[2];
}

void readTransformToJSON(Json::Value& root, FocalEngine::FETransformComponent* transform)
{
	transform->setPosition(glm::vec3(root["position"]["X"].asFloat(),
									 root["position"]["Y"].asFloat(),
									 root["position"]["Z"].asFloat()));

	transform->setRotation(glm::vec3(root["rotation"]["X"].asFloat(),
									 root["rotation"]["Y"].asFloat(),
									 root["rotation"]["Z"].asFloat()));

	transform->uniformScaling = root["scale"]["uniformScaling"].asBool();

	transform->setScale(glm::vec3(root["scale"]["X"].asFloat(),
								  root["scale"]["Y"].asFloat(),
								  root["scale"]["Z"].asFloat()));
}

void saveScene(const char* fileName)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();

	Json::Value root;
	std::ofstream sceneFile;
	!fileName ? sceneFile.open(PROJECTS_FOLDER "\\StartScene\\scene.txt") : sceneFile.open(fileName);

	// saving Meshes
	std::vector<std::string> meshList = resourceManager.getMeshList();
	Json::Value meshData;
	for (size_t i = 0; i < meshList.size(); i++)
	{
		FocalEngine::FEMesh* mesh = resourceManager.getMesh(meshList[i]);
		
		if (mesh->getFileName().size() == 0)
			continue;

		meshData[mesh->getName()]["file"] = mesh->getFileName();
		
	}
	root["meshes"] = meshData;

	// saving Materials
	std::vector<std::string> materialList = resourceManager.getMaterialList();
	Json::Value materialData;
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FocalEngine::FEMaterial* mat = resourceManager.getMaterial(materialList[i]);
		// to-do: for now we will save only one type of material. But we need to implement robust load/save for all types.
		if (mat->getTextureList().size() == 0)
			continue;

		std::vector<std::string> textureList = mat->getTextureList();
		for (size_t j = 0; j < textureList.size(); j++)
		{
			materialData[mat->getName()]["textures"][textureList[j]]["file"] = mat->getTexture(textureList[j])->getFileName();
		}
	}
	root["materials"] = materialData;

	// saving Entities
	std::vector<std::string> entityList = scene.getEntityList();
	Json::Value entityData;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		FocalEngine::FEEntity* entity = scene.getEntity(entityList[i]);

		entityData[entity->getName()]["mesh"] = entity->mesh->getName();
		entityData[entity->getName()]["material"] = entity->material->getName();
		writeTransformToJSON(entityData[entity->getName()]["transformation"], &entity->transform);
	}
	root["entities"] = entityData;

	// saving Lights
	std::vector<std::string> LightList = scene.getLightsList();
	Json::Value lightData;
	for (size_t i = 0; i < LightList.size(); i++)
	{
		FocalEngine::FELight* light = scene.getLight(LightList[i]);

		lightData[light->getName()]["type"] = light->getType();
		lightData[light->getName()]["intensity"] = light->getIntensity();
		lightData[light->getName()]["range"] = light->getRange();
		lightData[light->getName()]["spotAngle"] = light->getSpotAngle();
		lightData[light->getName()]["spotAngleOuter"] = light->getSpotAngleOuter();
		lightData[light->getName()]["castShadows"] = light->isCastShadows();
		lightData[light->getName()]["enabled"] = light->isLightEnabled();
		lightData[light->getName()]["color"]["R"] = light->getColor()[0];
		lightData[light->getName()]["color"]["G"] = light->getColor()[1];
		lightData[light->getName()]["color"]["B"] = light->getColor()[2];
		writeTransformToJSON(lightData[light->getName()]["transformation"], &light->transform);
		lightData[light->getName()]["direction"]["X"] = light->getDirection()[0];
		lightData[light->getName()]["direction"]["Y"] = light->getDirection()[1];
		lightData[light->getName()]["direction"]["Z"] = light->getDirection()[2];
	}
	root["lights"] = lightData;

	// saving Camera settings
	Json::Value cameraData;
	
	cameraData["position"]["X"] = engine.getCamera()->getPosition()[0];
	cameraData["position"]["Y"] = engine.getCamera()->getPosition()[1];
	cameraData["position"]["Z"] = engine.getCamera()->getPosition()[2];

	cameraData["fov"] = engine.getCamera()->getFov();
	cameraData["nearPlane"] = engine.getCamera()->getNearPlane();
	cameraData["farPlane"] = engine.getCamera()->getFarPlane();

	cameraData["yaw"] = engine.getCamera()->getYaw();
	cameraData["pitch"] = engine.getCamera()->getPitch();
	cameraData["roll"] = engine.getCamera()->getRoll();

	cameraData["aspectRatio"] = engine.getCamera()->getAspectRatio();

	cameraData["gamma"] = engine.getCamera()->getGamma();
	cameraData["exposure"] = engine.getCamera()->getExposure();

	root["camera"] = cameraData;

	// saving into file
	Json::StreamWriterBuilder builder;
	const std::string json_file = Json::writeString(builder, root);

	sceneFile << json_file;
	sceneFile.close();
}

void loadScene(const char* fileName)
{
	FocalEngine::FEngine& engine = FocalEngine::FEngine::getInstance();
	FocalEngine::FEResourceManager& resourceManager = FocalEngine::FEResourceManager::getInstance();
	FocalEngine::FEScene& scene = FocalEngine::FEScene::getInstance();
	std::ifstream sceneFile;

	!fileName ? sceneFile.open(PROJECTS_FOLDER "/StartScene/scene.txt") : sceneFile.open(fileName);

	std::string fileData((std::istreambuf_iterator<char>(sceneFile)), std::istreambuf_iterator<char>());

	Json::Value root;
	JSONCPP_STRING err;
	Json::CharReaderBuilder builder;

	const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	if (!reader->parse(fileData.c_str(), fileData.c_str() + fileData.size(), &root, &err))
		return;

	// loading Meshes
	std::vector<Json::String> meshList = root["meshes"].getMemberNames();
	for (size_t i = 0; i < meshList.size(); i++)
	{
		FocalEngine::FEMesh* newMesh = resourceManager.LoadFEMesh((std::string(PROJECTS_FOLDER "/StartScene/") + root["meshes"][meshList[i]]["file"].asCString()).c_str(), meshList[i]);
	}

	// loading Materials
	std::vector<Json::String> materialsList = root["materials"].getMemberNames();
	for (size_t i = 0; i < materialsList.size(); i++)
	{
		FocalEngine::FEMaterial* newMat = resourceManager.createMaterial(materialsList[i].c_str());
		newMat->shader = new FocalEngine::FEShader(FEPhongVS, FEPhongFS);

		std::vector<Json::String> textureList = root["materials"][materialsList[i]]["textures"].getMemberNames();
		for (size_t j = 0; j < textureList.size(); j++)
		{
			FocalEngine::FETexture* texture = resourceManager.LoadFETexture((std::string(PROJECTS_FOLDER "/StartScene/") + root["materials"][materialsList[i]]["textures"][textureList[j]]["file"].asCString()).c_str());
			texture->setName(textureList[j]);
			newMat->addTexture(texture);
		}
	}

	// loading Entities
	std::vector<Json::String> entityList = root["entities"].getMemberNames();
	for (size_t i = 0; i < entityList.size(); i++)
	{
		scene.addEntity(resourceManager.getSimpleMesh(root["entities"][entityList[i]]["mesh"].asCString()),
						resourceManager.getMaterial(root["entities"][entityList[i]]["material"].asCString()),
						entityList[i]);

		readTransformToJSON(root["entities"][entityList[i]]["transformation"], &scene.getEntity(entityList[i])->transform);
	}

	// loading Lights
	std::vector<Json::String> lightList = root["lights"].getMemberNames();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		scene.addLight(static_cast<FocalEngine::FELightType>(root["lights"][lightList[i]]["type"].asInt()), lightList[i]);
		FocalEngine::FELight* light = scene.getLight(lightList[i]);

		light->setIntensity(root["lights"][lightList[i]]["intensity"].asFloat());
		light->setRange(root["lights"][lightList[i]]["range"].asFloat());
		light->setSpotAngle(root["lights"][lightList[i]]["spotAngle"].asFloat());
		light->setSpotAngleOuter(root["lights"][lightList[i]]["spotAngleOuter"].asFloat());
		light->setCastShadows(root["lights"][lightList[i]]["castShadows"].asBool());
		light->setLightEnabled(root["lights"][lightList[i]]["enabled"].asBool());

		readTransformToJSON(root["lights"][lightList[i]]["transformation"], &light->transform);

		light->setDirection(glm::vec3(root["lights"][lightList[i]]["direction"]["X"].asFloat(),
									  root["lights"][lightList[i]]["direction"]["Y"].asFloat(),
									  root["lights"][lightList[i]]["direction"]["Z"].asFloat()));

		light->setColor(glm::vec3(root["lights"][lightList[i]]["color"]["R"].asFloat(),
								  root["lights"][lightList[i]]["color"]["G"].asFloat(),
								  root["lights"][lightList[i]]["color"]["B"].asFloat()));
	}

	// loading Camera settings
	engine.getCamera()->setPosition(glm::vec3(root["camera"]["position"]["X"].asFloat(),
											  root["camera"]["position"]["Y"].asFloat(),
											  root["camera"]["position"]["Z"].asFloat()));

	engine.getCamera()->setFov(root["camera"]["fov"].asFloat());
	engine.getCamera()->setNearPlane(root["camera"]["nearPlane"].asFloat());
	engine.getCamera()->setFarPlane(root["camera"]["farPlane"].asFloat());

	engine.getCamera()->setYaw(root["camera"]["yaw"].asFloat());
	engine.getCamera()->setPitch(root["camera"]["pitch"].asFloat());
	engine.getCamera()->setRoll(root["camera"]["roll"].asFloat());

	engine.getCamera()->setAspectRatio(root["camera"]["aspectRatio"].asFloat());

	engine.getCamera()->setGamma(root["camera"]["gamma"].asFloat());
	engine.getCamera()->setExposure(root["camera"]["exposure"].asFloat());

	sceneFile.close();
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

void loadEditor()
{
	if (!checkFolder(PROJECTS_FOLDER))
		createFolder(PROJECTS_FOLDER);
}