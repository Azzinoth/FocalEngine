#include "../Editor/FEEditor.h"
#include "nmmintrin.h"

FEEditor* FEEditor::_instance = nullptr;
FEEditor::FEEditor() {}
FEEditor::~FEEditor() {}

double FEEditor::getLastMouseX()
{
	return lastMouseX;
}

double FEEditor::getLastMouseY()
{
	return lastMouseY;
}

double FEEditor::getMouseX()
{
	return mouseX;
}

double FEEditor::getMouseY()
{
	return mouseY;
}

void FEEditor::setLastMouseX(double newValue)
{
	lastMouseX = newValue;
}

void FEEditor::setLastMouseY(double newValue)
{
	lastMouseY = newValue;
}

void FEEditor::setMouseX(double newValue)
{
	mouseX = newValue;
}

void FEEditor::setMouseY(double newValue)
{
	mouseY = newValue;
}

std::string FEEditor::getObjectNameInClipboard()
{
	return objectNameInClipboard;
}

void FEEditor::setObjectNameInClipboard(std::string newValue)
{
	objectNameInClipboard = newValue;
}

void FEEditor::mouseButtonCallback(int button, int action, int mods)
{
	if ((!ImGui::GetIO().WantCaptureMouse) && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
	{
		SELECTED.determineEntityUnderMouse(EDITOR.getMouseX(), EDITOR.getMouseY());
		SELECTED.checkForSelectionisNeeded = true;
		EDITOR.leftMousePressed = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		EDITOR.leftMousePressed = false;
	}

	if ((!ImGui::GetIO().WantCaptureMouse) && button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		GIZMO_MANAGER.deactivateAllGizmo();
	}

	if ((!ImGui::GetIO().WantCaptureMouse) && button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
	{
		EDITOR.isCameraInputActive = true;
		ENGINE.getCamera()->setIsInputActive(true);
	}
	else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_RELEASE)
	{
		EDITOR.isCameraInputActive = false;
		ENGINE.getCamera()->setIsInputActive(false);
	}
}

void FEEditor::keyButtonCallback(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		if (PROJECT_MANAGER.getCurrent() == nullptr)
			ENGINE.terminate();
		shouldTerminate = true;
		projectWasModifiedPopUpWindow.show(PROJECT_MANAGER.getCurrent());
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && key == GLFW_KEY_DELETE)
	{
		if (SELECTED.getType() == SELECTED_ENTITY_INSTANCED_SUBOBJECT)
		{
			FEEntityInstanced* selectedEntityInstanced = reinterpret_cast<FEEntityInstanced*>(SELECTED.getBareObject());
			selectedEntityInstanced->deleteInstance(SELECTED.getAdditionalInformation());
			SELECTED.clear();
		}

		if (SELECTED.getEntity() != nullptr)
		{
			SCENE.deleteEntity(SELECTED.getEntity()->getName());
			SELECTED.clear();
		}
		else if (SELECTED.getTerrain() != nullptr)
		{
			SCENE.deleteTerrain(SELECTED.getTerrain()->getName());
			SELECTED.clear();
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_C && action == GLFW_RELEASE)
	{
		if (SELECTED.getEntity() != nullptr)
			EDITOR.setObjectNameInClipboard(SELECTED.getEntity()->getName());
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_V && action == GLFW_RELEASE)
	{
		if (EDITOR.getObjectNameInClipboard() != "")
		{
			FEEntity* newEntity = SCENE.addEntity(SCENE.getEntity(EDITOR.getObjectNameInClipboard())->gameModel, "");
			newEntity->transform = SCENE.getEntity(EDITOR.getObjectNameInClipboard())->transform;
			newEntity->transform.setPosition(newEntity->transform.getPosition() * 1.1f);
			SELECTED.setEntity(newEntity);
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && (key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT) && action == GLFW_RELEASE)
	{
		int newState = GIZMO_MANAGER.gizmosState + 1;
		if (newState > 2)
			newState = 0;
		GIZMO_MANAGER.updateGizmoState(newState);
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && (key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT) && action == GLFW_RELEASE)
	{
		EDITOR.shiftPressed = false;
	}
	else if (!ImGui::GetIO().WantCaptureKeyboard && (key == GLFW_KEY_RIGHT_SHIFT || key == GLFW_KEY_LEFT_SHIFT) && action == GLFW_PRESS)
	{
		EDITOR.shiftPressed = true;
	}
}

void FEEditor::showTransformConfiguration(std::string objectName, FETransformComponent* transform)
{
	// ********************* POSITION *********************
	glm::vec3 position = transform->getPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + objectName).c_str(), &position[0], 0.1f);
	showToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + objectName).c_str(), &position[1], 0.1f);
	showToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + objectName).c_str(), &position[2], 0.1f);
	showToolTip("Z position");
	transform->setPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = transform->getRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + objectName).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	showToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + objectName).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	showToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + objectName).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	showToolTip("Z rotation");
	transform->setRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &transform->uniformScaling);
	glm::vec3 scale = transform->getScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + objectName).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	showToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + objectName).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	showToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + objectName).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	showToolTip("Z scale");

	glm::vec3 oldScale = transform->getScale();
	transform->changeXScaleBy(scale[0] - oldScale[0]);
	transform->changeYScaleBy(scale[1] - oldScale[1]);
	transform->changeZScaleBy(scale[2] - oldScale[2]);

	// ********************* REAL WORLD COMPARISON SCALE *********************
	FEEntity* entity = SCENE.getEntity(objectName);
	if (entity == nullptr)
		return;

	FEAABB realAABB = entity->getAABB();
	glm::vec3 min = realAABB.getMin();
	glm::vec3 max = realAABB.getMax();

	float xSize = sqrt((max.x - min.x) * (max.x - min.x));
	float ySize = sqrt((max.y - min.y) * (max.y - min.y));
	float zSize = sqrt((max.z - min.z) * (max.z - min.z));

	std::string sizeInM = "Approximate object size: ";
	sizeInM += std::to_string(std::max(xSize, std::max(ySize, zSize)));
	sizeInM += " m";

	/*std::string dementionsInM = "Xlength: ";
	dementionsInM += std::to_string(xSize);
	dementionsInM += " m Ylength: ";
	dementionsInM += std::to_string(ySize);
	dementionsInM += " m Zlength: ";
	dementionsInM += std::to_string(zSize);
	dementionsInM += " m";*/
	ImGui::Text(sizeInM.c_str());
}

void FEEditor::showPosition(std::string objectName, glm::vec3& position)
{
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + objectName).c_str(), &position[0], 0.1f);
	showToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + objectName).c_str(), &position[1], 0.1f);
	showToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + objectName).c_str(), &position[2], 0.1f);
	showToolTip("Z position");
}

void FEEditor::showRotation(std::string objectName, glm::vec3& rotation)
{
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + objectName).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	showToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + objectName).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	showToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + objectName).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	showToolTip("Z rotation");
}

void FEEditor::showScale(std::string objectName, glm::vec3& scale)
{
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + objectName).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	showToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + objectName).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	showToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + objectName).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	showToolTip("Z scale");
}

void FEEditor::displayMaterialPrameter(FEShaderParam* param)
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

void FEEditor::displayLightProperties(FELight* light)
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
		showToolTip("How much steps of shadow quality will be used.");

		ImGui::Text("Shadow coverage in M :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float firstCascadeSize = directionalLight->getShadowCoverage();
		ImGui::DragFloat("##shadowCoverage", &firstCascadeSize, 0.1f, 0.1f, 500.0f);
		directionalLight->setShadowCoverage(firstCascadeSize);
		showToolTip("Distance from camera at which shadows would be present.");

		ImGui::Text("Z depth of shadow map :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float CSMZDepth = directionalLight->getCSMZDepth();
		ImGui::DragFloat("##CSMZDepth", &CSMZDepth, 0.01f, 0.1f, 100.0f);
		directionalLight->setCSMZDepth(CSMZDepth);
		showToolTip("If you have problems with shadow disapearing when camera is at close distance to shadow reciver, tweaking this parameter could help. Otherwise this parameter should be as small as possible.");

		ImGui::Text("XY depth of shadow map :");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		float CSMXYDepth = directionalLight->getCSMXYDepth();
		ImGui::DragFloat("##CSMXYDepth", &CSMXYDepth, 0.01f, 0.0f, 100.0f);
		directionalLight->setCSMXYDepth(CSMXYDepth);
		showToolTip("If you have problems with shadow on edges of screen, tweaking this parameter could help. Otherwise this parameter should be as small as possible.");

		static FEShader* shaderPBR = RESOURCE_MANAGER.getShader("FEPBRShader");
		static FEShader* shaderInstancedPBR = RESOURCE_MANAGER.getShader("FEPBRInstancedShader");
		static FEShader* shaderTerrain = RESOURCE_MANAGER.getShader("FETerrainShader");

		ImGui::Text("Shadows blur factor:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		float shadowsBlurFactor = *(float*)shaderPBR->getParameter("shadowBlurFactor")->data;
		ImGui::DragFloat("##Shadows blur factor", &shadowsBlurFactor, 0.001f, 0.0f, 10.0f);
		shaderPBR->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
		shaderInstancedPBR->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
		shaderTerrain->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
	
		bool staticShadowBias = directionalLight->isStaticShadowBias();
		ImGui::Checkbox("Static shadow bias :", &staticShadowBias);
		directionalLight->setIsStaticShadowBias(staticShadowBias);

		if (directionalLight->isStaticShadowBias())
		{
			ImGui::Text("Static shadow bias value :");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			float shadowBias = directionalLight->getShadowBias();
			ImGui::DragFloat("##shadowBias", &shadowBias, 0.0001f, 0.00001f, 0.1f);
			directionalLight->setShadowBias(shadowBias);
		}
		else
		{
			ImGui::Text("Intensity of variable shadow bias :");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			float shadowBiasIntensity = directionalLight->getShadowBiasVariableIntensity();
			ImGui::DragFloat("##shadowBiasIntensity", &shadowBiasIntensity, 0.01f, 0.01f, 10.0f);
			directionalLight->setShadowBiasVariableIntensity(shadowBiasIntensity);
		}
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

void FEEditor::displayLightsProperties()
{
	std::vector<std::string> lightList = SCENE.getLightsList();

	for (size_t i = 0; i < lightList.size(); i++)
	{
		if (ImGui::TreeNode(lightList[i].c_str()))
		{
			displayLightProperties(SCENE.getLight(lightList[i]));
			ImGui::TreePop();
		}
	}
}

void FEEditor::displaySceneEntities()
{
	std::vector<std::string> entityList = SCENE.getEntityList();
	std::vector<std::string> materialList = RESOURCE_MANAGER.getMaterialList();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	//#fix this non-sence with proper Imgui docking system 
	float windowW = ENGINE.getWindowWidth() / 3.7f;
	if (windowW > 600)
		windowW = 600;
	ImGui::SetNextWindowSize(ImVec2(windowW, float(ENGINE.getWindowHeight())));
	ImGui::Begin("Scene Entities", nullptr, ImGuiWindowFlags_None);
	
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Add new entity", ImVec2(220, 0)))
	{
		selectGameModelWindow.show(nullptr, true);
	}

	if (ImGui::Button("Add new instanced entity", ImVec2(220, 0)))
	{
		selectGameModelWindow.show(nullptr, true, true);
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	selectGameModelWindow.render();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));
	if (ImGui::Button("Save project", ImVec2(220, 0)))
	{
		PROJECT_MANAGER.getCurrent()->saveScene();
		ENGINE.takeScreenshot((PROJECT_MANAGER.getCurrent()->getProjectFolder() + "projectScreenShot.texture").c_str());
	}

	if (ImGui::Button("Close project", ImVec2(220, 0)))
	{
		if (PROJECT_MANAGER.getCurrent()->modified)
		{
			projectWasModifiedPopUpWindow.show(PROJECT_MANAGER.getCurrent());
		}
		else
		{
			PROJECT_MANAGER.closeCurrentProject();

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::End();

			return;
		}
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	if (SELECTED.getDirtyFlag() && SELECTED.getEntity() != nullptr)
	{
		for (size_t i = 0; i < entityList.size(); i++)
		{
			ImGui::GetStateStorage()->SetInt(ImGui::GetID(entityList[i].c_str()), 0);
		}
		SELECTED.setDirtyFlag(false);
		ImGui::GetStateStorage()->SetInt(ImGui::GetID(SELECTED.getEntityName().c_str()), 1);
	}

	entityUnderMouse = -1;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		FEEntity* entity = SCENE.getEntity(entityList[i]);

		if (entity->transform.isDirty())
			PROJECT_MANAGER.getCurrent()->modified = true;

		if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(entity))
			continue;

		if (ImGui::TreeNode(entity->getName().c_str()))
		{
			// when tree node is opened, only here we can catch mouse hover.
			if (ImGui::IsItemHovered())
				entityUnderMouse = i;
			
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
				if (SELECTED.getEntity() == entity)
					SELECTED.clear();
				
				SCENE.deleteEntity(entity->getName());

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();

				ImGui::PopID();
				ImGui::TreePop();
				break;
			}

			if (entity->getType() == FE_ENTITY_INSTANCED)
			{
				FEEntityInstanced* instancedEntity = reinterpret_cast<FEEntityInstanced*>(entity);
				ImGui::Text("Snapped to: ");
				ImGui::SameLine();

				std::vector<std::string> terrainList = RESOURCE_MANAGER.getTerrainList();
				static std::string currentTerrain = "none";

				if (instancedEntity->getSnappedToTerrain() == nullptr)
				{
					currentTerrain = "none";
				}
				else
				{
					currentTerrain = instancedEntity->getSnappedToTerrain()->getName();
				}

				if (ImGui::BeginCombo("##Terrain", currentTerrain.c_str(), ImGuiWindowFlags_None))
				{
					bool is_selected = (currentTerrain == "none");
					if (ImGui::Selectable("none", is_selected))
					{
						if (instancedEntity->getSnappedToTerrain() != nullptr)
							instancedEntity->getSnappedToTerrain()->unSnapInstancedEntity(instancedEntity);
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();

					for (size_t i = 0; i < terrainList.size(); i++)
					{
						bool is_selected = (currentTerrain == terrainList[i]);
						if (ImGui::Selectable(terrainList[i].c_str(), is_selected))
						{
							RESOURCE_MANAGER.getTerrain(terrainList[i])->snapInstancedEntity(instancedEntity);
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}


				ImGui::Text("Seed:");
				int seed = instancedEntity->spawnInfo.seed;
				ImGui::SameLine();
				ImGui::DragInt("##Seed", &seed);
				instancedEntity->spawnInfo.seed = seed;

				ImGui::Text("Object count:");
				int objectCount = instancedEntity->spawnInfo.objectCount;
				ImGui::SameLine();
				ImGui::DragInt("##Object count", &objectCount);
				if (objectCount <= 0)
					objectCount = 1;
				instancedEntity->spawnInfo.objectCount = objectCount;

				ImGui::Text("Radius:");
				float radius = instancedEntity->spawnInfo.radius;
				ImGui::SameLine();
				ImGui::DragFloat("##Radius", &radius);
				if (radius < 0.0f)
					radius = 0.1f;
				instancedEntity->spawnInfo.radius = radius;

				ImGui::Text("Scale deviation:");
				float scaleDeviation = instancedEntity->spawnInfo.scaleDeviation;
				ImGui::SameLine();
				ImGui::DragFloat("##Scale deviation", &scaleDeviation, 0.01f);
				if (scaleDeviation < 0.0f)
					scaleDeviation = 0.0f;
				instancedEntity->spawnInfo.scaleDeviation = scaleDeviation;

				ImGui::Text("Rotation deviation:");
				float rotationDeviationX = instancedEntity->spawnInfo.rotationDeviation.x;
				ImGui::Text("X:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation X", &rotationDeviationX, 0.01f);
				if (rotationDeviationX < 0.01f)
					rotationDeviationX = 0.01f;
				if (rotationDeviationX > 1.0f)
					rotationDeviationX = 1.0f;
				instancedEntity->spawnInfo.rotationDeviation.x = rotationDeviationX;

				float rotationDeviationY = instancedEntity->spawnInfo.rotationDeviation.y;
				ImGui::Text("Y:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation Y", &rotationDeviationY, 0.01f);
				if (rotationDeviationY < 0.01f)
					rotationDeviationY = 0.01f;
				if (rotationDeviationY > 1.0f)
					rotationDeviationY = 1.0f;
				instancedEntity->spawnInfo.rotationDeviation.y = rotationDeviationY;
				
				float rotationDeviationZ = instancedEntity->spawnInfo.rotationDeviation.z;
				ImGui::Text("Z:");
				ImGui::SameLine();
				ImGui::DragFloat("##Rotation deviation z", &rotationDeviationZ, 0.01f);
				if (rotationDeviationZ < 0.01f)
					rotationDeviationZ = 0.01f;
				if (rotationDeviationZ > 1.0f)
					rotationDeviationZ = 1.0f;
				instancedEntity->spawnInfo.rotationDeviation.z = rotationDeviationZ;
				
				if (ImGui::Button("Spawn/Re-Spawn"))
				{
					instancedEntity->clear();
					instancedEntity->populate(instancedEntity->spawnInfo);
				}

				if (ImGui::Button("Add instance"))
				{
					glm::mat4 newInstanceMatrix = glm::identity<glm::mat4>();
					newInstanceMatrix = glm::translate(newInstanceMatrix, ENGINE.getCamera()->getPosition() + ENGINE.getCamera()->getForward() * 10.0f);
					instancedEntity->addInstance(newInstanceMatrix);

					PROJECT_MANAGER.getCurrent()->modified = true;
				}

				if (instancedEntity->isSelectMode())
				{
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.0f, 0.75f, 0.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.0f, 1.0f, 0.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.0f, 1.0f, 0.0f));
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.55f, 0.55f, 0.95f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
				}

				ImGui::Separator();
				if (ImGui::ImageButton((void*)(intptr_t)mouseCursorIcon->getTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					SCENE.setSelectMode(instancedEntity, !instancedEntity->isSelectMode());
					if (!instancedEntity->isSelectMode())
					{
						SELECTED.clear();
						SELECTED.setEntity(instancedEntity);
					}
				}
				showToolTip("Individual selection mode - Used to select individual instances.");

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();

				if (SELECTED.getType() != SELECTED_ENTITY_INSTANCED_SUBOBJECT || SELECTED.getBareObject() != instancedEntity || !instancedEntity->isSelectMode() || instancedEntity->getSnappedToTerrain() == nullptr)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.55f, 0.55f, 0.55f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.75f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.75f));
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.55f, 0.55f, 0.95f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
				}

				ImGui::SameLine();
				if (ImGui::ImageButton((void*)(intptr_t)arrowToGroundIcon->getTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					instancedEntity->tryToSnapInstance(SELECTED.getAdditionalInformation());
				}
				showToolTip("Selected instance will attempt to snap to the terrain.");

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();

				std::string instancedSubObjectInfo = "index: ";
				if (SELECTED.getType() == SELECTED_ENTITY_INSTANCED_SUBOBJECT)
				{
					if (SELECTED.getBareObject() == instancedEntity && instancedEntity->isSelectMode())
					{
						ImGui::Text("Selected instance info:");
						instancedSubObjectInfo = "index: " + std::to_string(SELECTED.getAdditionalInformation());
						ImGui::Text(instancedSubObjectInfo.c_str());

						FETransformComponent tempTransform = FETransformComponent(instancedEntity->getTransformedInstancedMatrix(SELECTED.getAdditionalInformation()));
						ImGui::PushID(instancedSubObjectInfo.c_str());
						showTransformConfiguration("selected instance", &tempTransform);
						ImGui::PopID();

						instancedEntity->modifyInstance(SELECTED.getAdditionalInformation(), tempTransform.getTransformMatrix());
					}
				}
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			
			ImGui::PopID();
			ImGui::TreePop();
		}

		if (ImGui::IsItemHovered())
			entityUnderMouse = i;

		if (ImGui::IsMouseClicked(0) && entityUnderMouse != -1)
		{
			if (SELECTED.getEntity() != SCENE.getEntity(entityList[entityUnderMouse]))
			{
				if (SELECTED.getType() == SELECTED_ENTITY_INSTANCED_SUBOBJECT)
				{
					FEEntityInstanced* etityInstanced = reinterpret_cast<FEEntityInstanced*>(SELECTED.getBareObject());
					if (etityInstanced != SCENE.getEntity(entityList[entityUnderMouse]))
					{
						SELECTED.setEntity(SCENE.getEntity(entityList[entityUnderMouse]));
						SELECTED.setDirtyFlag(false);
					}
				}
				else
				{
					SELECTED.setEntity(SCENE.getEntity(entityList[entityUnderMouse]));
					SELECTED.setDirtyFlag(false);
				}
			}
		}
	}
	ImGui::Text("============================================");

	displayLightsProperties();

	float cameraSpeed = reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera())->getSpeed();
	ImGui::DragFloat("Camera speed in m/s", &cameraSpeed, 0.01f, 0.01f, 100.0f);
	reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera())->setSpeed(cameraSpeed);

	static bool displayGrid = true;
	ImGui::Checkbox("Display grid", &displayGrid);

	static glm::vec3 color = glm::vec3(0.2f, 0.3f, 0.4f);

	float basicW = 0.1f;
	float width = basicW * 4.0f;
	if (displayGrid)
	{
		int gridSize = 200;
		for (int i = -gridSize / 2; i < gridSize / 2; i++)
		{
			color = glm::vec3(0.4f, 0.65f, 0.73f);
			width = basicW * 4.0f;
			if (i % 2 != 0 && i != 0)
			{
				color = color / 4.0f;
				width = width / 4.0f;
			}
			else if (i == 0)
			{
				color = glm::vec3(0.9f, 0.9f, 0.9f);
				width = basicW * 4.0f;
			}

			RENDERER.drawLine(glm::vec3(i, 0.0f, -gridSize / 2), glm::vec3(i, 0.0f, gridSize / 2), color, width);
			RENDERER.drawLine(glm::vec3(-gridSize / 2, 0.0f, i), glm::vec3(gridSize / 2, 0.0f, i), color, width);
		}
	}

	static float favgTime = 0.0f;
	static std::vector<float> avgTime;
	static int counter = 0;

	ImGui::Text((std::string("Time : ") + std::to_string(RENDERER.lastTestTime)).c_str());

	if (avgTime.size() < 100)
	{
		avgTime.push_back(RENDERER.lastTestTime);
	}
	else if (avgTime.size() >= 100)
	{
		avgTime[counter++ % 100] = RENDERER.lastTestTime;
	}

	for (size_t i = 0; i < avgTime.size(); i++)
	{
		favgTime += avgTime[i];
	}
	favgTime /= avgTime.size();


	if (counter > 1000000)
		counter = 0;

	ImGui::Text((std::string("avg Time : ") + std::to_string(favgTime)).c_str());

	bool freezeCulling = RENDERER.freezeCulling;
	ImGui::Checkbox("freezeCulling", &freezeCulling);
	RENDERER.freezeCulling = freezeCulling;

	static bool displaySelectedObjAABB = false;
	ImGui::Checkbox("Display AABB of selected object", &displaySelectedObjAABB);

	// draw AABB
	if (SELECTED.isAnyObjectSelected() && displaySelectedObjAABB)
	{
		FEAABB selectedAABB = SELECTED.getEntity() != nullptr ? SELECTED.getEntity()->getAABB() : SELECTED.getTerrain()->getAABB();
		RENDERER.drawAABB(selectedAABB);
	}

	ImGui::End();

	selectMeshWindow.render();
	selectMaterialWindow.render();
	selectGameModelWindow.render();
	renameEntityWindow.render();
	projectWasModifiedPopUpWindow.render();
}

void FEEditor::initializeResources()
{
	ENGINE.setKeyCallback(keyButtonCallback);
	ENGINE.setMouseButtonCallback(mouseButtonCallback);
	ENGINE.setMouseMoveCallback(mouseMoveCallback);

	SELECTED.initializeResources();
	ENGINE.getCamera()->setIsInputActive(isCameraInputActive);
	PROJECT_MANAGER.initializeResources();

	// **************************** Meshes Content Browser ****************************
	PREVIEW_MANAGER.initializeResources();

	// **************************** Gizmos ****************************
	GIZMO_MANAGER.initializeResources();

	// hide all resources for gizmos from content browser
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(RESOURCE_MANAGER.getMesh("transformationGizmoMesh"));
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(RESOURCE_MANAGER.getMesh("scaleGizmoMesh"));
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(RESOURCE_MANAGER.getMesh("rotateGizmoMesh"));

	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationYGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationZGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXYGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationYZGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationYZGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXZGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.transformationXZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleXGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleYGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleZGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.scaleZGizmoEntity);

	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateXGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateXGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateYGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateYGizmoEntity);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateZGizmoEntity->gameModel);
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(GIZMO_MANAGER.rotateZGizmoEntity);

	sculptBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/sculptBrush.png", "sculptBrushIcon");
	RESOURCE_MANAGER.makeTextureStandard(sculptBrushIcon);
	levelBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/levelBrush.png", "levelBrushIcon");
	RESOURCE_MANAGER.makeTextureStandard(levelBrushIcon);
	smoothBrushIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/smoothBrush.png", "smoothBrushIcon");
	RESOURCE_MANAGER.makeTextureStandard(smoothBrushIcon);
	mouseCursorIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/mouseCursorIcon.png", "mouseCursorIcon");
	RESOURCE_MANAGER.makeTextureStandard(mouseCursorIcon);
	arrowToGroundIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/arrowToGroundIcon.png", "arrowToGroundIcon");
	RESOURCE_MANAGER.makeTextureStandard(arrowToGroundIcon);

	ENGINE.getCamera()->setOnUpdate(onCameraUpdate);
	ENGINE.setWindowCloseCallback(closeWindowCallBack);
}

void FEEditor::mouseMoveCallback(double xpos, double ypos)
{
	EDITOR.setLastMouseX(EDITOR.getMouseX());
	EDITOR.setLastMouseY(EDITOR.getMouseY());

	EDITOR.setMouseX(xpos);
	EDITOR.setMouseY(ypos);

	//SELECTED.determineEntityUnderMouse(EDITOR.getMouseX(), EDITOR.getMouseY());
	
	//if (SELECTED.getEntity() != nullptr || SELECTED.getTerrain() != nullptr)
	if (SELECTED.isAnyObjectSelected())
	{
		if (SELECTED.getTerrain() != nullptr)
		{
			if (SELECTED.getTerrain()->isBrushSculptMode() || SELECTED.getTerrain()->isBrushLevelMode() || SELECTED.getTerrain()->isBrushSmoothMode())
				return;
		}

		GIZMO_MANAGER.mouseMove(EDITOR.getLastMouseX(), EDITOR.getLastMouseY(), EDITOR.getMouseX(), EDITOR.getMouseY());
	}
}

void FEEditor::onCameraUpdate(FEBasicCamera* camera)
{
	SELECTED.onCameraUpdate();
	GIZMO_MANAGER.render();
}

void FEEditor::render()
{
	if (PROJECT_MANAGER.getCurrent())
	{
		/*glm::dvec3 currentTerrainPoint;
		FEEntity* test = SCENE.getEntity("tree");
		if (test != nullptr)
		{
			FETerrain* terrain = SCENE.getTerrain("test");
			if (terrain != nullptr)
			{
				float range = 256.0f;
				glm::dvec3 currentRay = mouseRay();

				currentTerrainPoint = terrain->getPointOnTerrain(ENGINE.getCamera()->getPosition(), currentRay, 0, range);
				if (currentTerrainPoint.x != FLT_MAX)
					test->transform.setPosition(glm::vec3(currentTerrainPoint.x, currentTerrainPoint.y, currentTerrainPoint.z));
			}
		}*/

		displaySceneEntities();
		displayContentBrowser();
		gyzmosSettingsWindowObject.show();
		gyzmosSettingsWindowObject.render();

		int index = SELECTED.getIndexOfObjectUnderMouse(EDITOR.getMouseX(), EDITOR.getMouseY());
		if (index >= 0)
		{
			if (!GIZMO_MANAGER.wasSelected(index))
			{
				SELECTED.setSelectedByIndex(index);
			}
		}
	}
	else
	{
		PROJECT_MANAGER.displayProjectSelection();
	}

	if (LOG.log.size() > 0)
	{
		char text[20000];
		std::string text1;
		for (size_t i = 0; i < LOG.log.size(); i++)
		{
			text1 += LOG.log[i];
			text1 += "\n";
		}

		strcpy_s(text, text1.size() + 1, text1.data());
		//strcpy_s(text, LOG.log[0].size() + 1, LOG.log[0].data());
		ImGui::InputTextMultiline("LOG", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 0);
	}
}

void FEEditor::closeWindowCallBack()
{
	if (PROJECT_MANAGER.getCurrent() == nullptr)
	{
		ENGINE.terminate();
		return;
	}

	if (PROJECT_MANAGER.getCurrent()->modified)
	{
		shouldTerminate = true;
		projectWasModifiedPopUpWindow.show(PROJECT_MANAGER.getCurrent());
	}
	else
	{
		PROJECT_MANAGER.closeCurrentProject();
		ENGINE.terminate();
		return;
	}
}