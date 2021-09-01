#include "../Editor/FEEditor.h"

FEEditor* FEEditor::_instance = nullptr;
ImGuiWindow* FEEditor::sceneWindow = nullptr;

bool sceneWindowDragAndDropCallBack(FEObject* object, void** userDat)
{
	if (object->getType() == FE_GAMEMODEL)
	{
		FEEntity* newEntity = FEScene::getInstance().addEntity(RESOURCE_MANAGER.getGameModel(object->getObjectID()));
		newEntity->transform.setPosition(ENGINE.getCamera()->getPosition() + ENGINE.getCamera()->getForward() * 10.0f);
		SELECTED.setSelected(newEntity);
		PROJECT_MANAGER.getCurrent()->modified = true;

		return true;
	}

	return false;
}

FEEditor::FEEditor()
{
	ENGINE.setRenderTargetMode(FE_CUSTOM_MODE);
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ENGINE.renderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera()));

	strcpy_s(filterForResourcesContentBrowser, "");
}

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
	if (ImGui::GetCurrentContext()->HoveredWindow != nullptr && FEEditor::sceneWindow != nullptr)
	{
		EDITOR.sceneWindowHovered = ImGui::GetCurrentContext()->HoveredWindow->Name == EDITOR.sceneWindow->Name;
	}
	else
	{
		EDITOR.sceneWindowHovered = false;
	}
	
	if (FEEditor::sceneWindow == nullptr || !FEEditor::sceneWindow->Active)
		EDITOR.sceneWindowHovered = false;

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
		DRAG_AND_DROP_MANAGER.dropAction();

	if (ImGui::GetIO().WantCaptureMouse && !EDITOR.sceneWindowHovered)
	{
		EDITOR.isCameraInputActive = false;
		ENGINE.getCamera()->setIsInputActive(false);

		return;
	}

	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
	{
		SELECTED.determineEntityUnderMouse(EDITOR.getMouseX(), EDITOR.getMouseY());
		SELECTED.checkForSelectionisNeeded = true;
		EDITOR.leftMousePressed = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		EDITOR.leftMousePressed = false;
		GIZMO_MANAGER.deactivateAllGizmo();
	}

	if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS)
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
		if (FEEditor::getInstance().isInGameMode())
		{
			FEEditor::getInstance().setGameMode(false);
		}
		else
		{
			if (PROJECT_MANAGER.getCurrent() == nullptr)
				ENGINE.terminate();
			shouldTerminate = true;
			projectWasModifiedPopUp::getInstance().show(PROJECT_MANAGER.getCurrent());
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && key == GLFW_KEY_DELETE)
	{
		if (SELECTED.getSelected() != nullptr && SELECTED.getSelected()->getType() == FE_ENTITY_INSTANCED)
		{
			if (SELECTED.instancedSubObjectIndexSelected != -1)
			{
				FEEntityInstanced* selectedEntityInstanced = SCENE.getEntityInstanced(SELECTED.getSelected()->getObjectID());
				selectedEntityInstanced->deleteInstance(SELECTED.instancedSubObjectIndexSelected);
				SELECTED.clear();
				PROJECT_MANAGER.getCurrent()->modified = true;
			}
		}

		if (SELECTED.getEntity() != nullptr)
		{
			SCENE.deleteEntity(SELECTED.getEntity()->getObjectID());
			SELECTED.clear();
			PROJECT_MANAGER.getCurrent()->modified = true;
		}
		else if (SELECTED.getTerrain() != nullptr)
		{
			SCENE.deleteTerrain(SELECTED.getTerrain()->getObjectID());
			SELECTED.clear();
			PROJECT_MANAGER.getCurrent()->modified = true;
		}
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_C && action == GLFW_RELEASE)
	{
		if (SELECTED.getEntity() != nullptr)
			EDITOR.setObjectNameInClipboard(SELECTED.getEntity()->getObjectID());
	}

	if (!ImGui::GetIO().WantCaptureKeyboard && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_V && action == GLFW_RELEASE)
	{
		if (EDITOR.getObjectNameInClipboard() != "")
		{
			FEEntity* newEntity = SCENE.addEntity(SCENE.getEntity(EDITOR.getObjectNameInClipboard())->gameModel, "");
			newEntity->transform = SCENE.getEntity(EDITOR.getObjectNameInClipboard())->transform;
			newEntity->transform.setPosition(newEntity->transform.getPosition() * 1.1f);
			SELECTED.setSelected(newEntity);
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

void FEEditor::showTransformConfiguration(FEObject* object, FETransformComponent* transform)
{
	// ********************* POSITION *********************
	glm::vec3 position = transform->getPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + object->getName()).c_str(), &position[0], 0.1f);
	showToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + object->getName()).c_str(), &position[1], 0.1f);
	showToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + object->getName()).c_str(), &position[2], 0.1f);
	showToolTip("Z position");
	transform->setPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = transform->getRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + object->getName()).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	showToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + object->getName()).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	showToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + object->getName()).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	showToolTip("Z rotation");
	transform->setRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &transform->uniformScaling);
	glm::vec3 scale = transform->getScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + object->getName()).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	showToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + object->getName()).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	showToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + object->getName()).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	showToolTip("Z scale");

	glm::vec3 oldScale = transform->getScale();
	transform->changeXScaleBy(scale[0] - oldScale[0]);
	transform->changeYScaleBy(scale[1] - oldScale[1]);
	transform->changeZScaleBy(scale[2] - oldScale[2]);

	// ********************* REAL WORLD COMPARISON SCALE *********************
	if (object->getType() == FE_ENTITY || object->getType() == FE_ENTITY_INSTANCED)
	{
		FEEntity* entity = SCENE.getEntity(object->getObjectID());

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
}

void FEEditor::showTransformConfiguration(std::string name, FETransformComponent* transform)
{
	// ********************* POSITION *********************
	glm::vec3 position = transform->getPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + name).c_str(), &position[0], 0.1f);
	showToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + name).c_str(), &position[1], 0.1f);
	showToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + name).c_str(), &position[2], 0.1f);
	showToolTip("Z position");
	transform->setPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = transform->getRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + name).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	showToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + name).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	showToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + name).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	showToolTip("Z rotation");
	transform->setRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &transform->uniformScaling);
	glm::vec3 scale = transform->getScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + name).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	showToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + name).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	showToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + name).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	showToolTip("Z scale");

	glm::vec3 oldScale = transform->getScale();
	transform->changeXScaleBy(scale[0] - oldScale[0]);
	transform->changeYScaleBy(scale[1] - oldScale[1]);
	transform->changeZScaleBy(scale[2] - oldScale[2]);
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
	showTransformConfiguration(light, &light->transform);

	if (light->getType() == FE_DIRECTIONAL_LIGHT)
	{
		FEDirectionalLight* directionalLight = reinterpret_cast<FEDirectionalLight*>(light);
		ImGui::Separator();
		ImGui::Text("-------------Shadow settings--------------");

		ImGui::Text("Cast shadows:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		bool castShadows = directionalLight->isCastShadows();
		ImGui::Checkbox("##Cast shadows", &castShadows);
		directionalLight->setCastShadows(castShadows);
		showToolTip("Will this light cast shadows.");

		if (!directionalLight->isCastShadows())
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
		}
		
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

		static FEShader* shaderPBR = RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);
		static FEShader* shaderInstancedPBR = RESOURCE_MANAGER.getShader("7C80085C184442155D0F3C7B"/*"FEPBRInstancedShader"*/);
		static FEShader* shaderTerrain = RESOURCE_MANAGER.getShader("5A3E4F5C13115856401F1D1C"/*"FETerrainShader"*/);

		ImGui::Text("Shadows blur factor:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200.0f);
		float shadowsBlurFactor = *(float*)shaderPBR->getParameter("shadowBlurFactor")->data;
		ImGui::DragFloat("##Shadows blur factor", &shadowsBlurFactor, 0.001f, 0.0f, 10.0f);
		shaderPBR->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
		shaderInstancedPBR->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
#ifdef USE_DEFERRED_RENDERER
		
#else
		shaderTerrain->getParameter("shadowBlurFactor")->updateData(shadowsBlurFactor);
#endif // USE_DEFERRED_RENDERER
	
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

		if (!directionalLight->isCastShadows())
		{
			//ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
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

void FEEditor::drawCorrectSceneBrowserIcon(FEObject* sceneObject)
{
	ImGui::SetCursorPosX(20);

	if (sceneObject->getType() == FE_ENTITY || sceneObject->getType() == FE_ENTITY_INSTANCED)
	{
		FEEntity* entity = SCENE.getEntity(sceneObject->getObjectID());

		if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(entity))
			return;

		if (entity->getType() == FE_ENTITY_INSTANCED)
		{
				
			ImGui::Image((void*)(intptr_t)instancedEntitySceneBrowserIcon->getTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
		else
		{
			ImGui::Image((void*)(intptr_t)entitySceneBrowserIcon->getTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		}
	}

	if (sceneObject->getType() == FE_DIRECTIONAL_LIGHT)
	{
		ImGui::Image((void*)(intptr_t)directionalLightSceneBrowserIcon->getTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		
	}

	if (sceneObject->getType() == FE_SPOT_LIGHT)
	{
		ImGui::Image((void*)(intptr_t)spotLightSceneBrowserIcon->getTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		
	}

	if (sceneObject->getType() == FE_POINT_LIGHT)
	{
		ImGui::Image((void*)(intptr_t)pointLightSceneBrowserIcon->getTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
		
	}

	if (sceneObject->getType() == FE_TERRAIN)
	{
		ImGui::Image((void*)(intptr_t)terrainSceneBrowserIcon->getTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}

	if (sceneObject->getType() == FE_CAMERA)
	{
		ImGui::Image((void*)(intptr_t)cameraSceneBrowserIcon->getTextureID(), ImVec2(16, 16), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}

	ImGui::SameLine();
	return;
}

void FEEditor::displaySceneBrowser()
{
	if (!sceneBrowserVisible)
		return;

	static int sceneObjectHoveredIndex = -1;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Scene Entities", nullptr, ImGuiWindowFlags_None);

	selectGameModelPopUp::getInstance().render();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));

	if (ImGui::Button("Enter game mode", ImVec2(220, 0)))
	{
		FEEditor::getInstance().setGameMode(true);
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	std::vector<std::string> entityList = SCENE.getEntityList();
	std::vector<std::string> sceneObjectsList;
	for (size_t i = 0; i < entityList.size(); i++)
	{
		if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(SCENE.getEntity(entityList[i])))
			continue;
		sceneObjectsList.push_back(entityList[i]);
	}
	std::vector<std::string> lightList = SCENE.getLightsList();
	for (size_t i = 0; i < lightList.size(); i++)
	{
		sceneObjectsList.push_back(lightList[i]);
	}
	std::vector<std::string> terrainList = SCENE.getTerrainList();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		sceneObjectsList.push_back(terrainList[i]);
	}

	sceneObjectsList.push_back(ENGINE.getCamera()->getObjectID());

	if (!isOpenContextMenuInSceneEntities)
		sceneObjectHoveredIndex = -1;
	
	for (size_t i = 0; i < sceneObjectsList.size(); i++)
	{
		drawCorrectSceneBrowserIcon(FEObjectManager::getInstance().getFEObject(sceneObjectsList[i]));

		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		if (SELECTED.getSelected() != nullptr)
		{
			if (SELECTED.getSelected()->getObjectID() == sceneObjectsList[i])
			{
				node_flags |= ImGuiTreeNodeFlags_Selected;
			}
		}

		ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, FEObjectManager::getInstance().getFEObject(sceneObjectsList[i])->getName().c_str(), i);
		if (ImGui::IsItemClicked())
		{
			SELECTED.setSelected(FEObjectManager::getInstance().getFEObject(sceneObjectsList[i]));
			SELECTED.setDirtyFlag(false);
		}

		if (ImGui::IsItemHovered())
		{
			sceneObjectHoveredIndex = i;
		}
	}

	bool open_context_menu = false;
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
		open_context_menu = true;

	if (open_context_menu)
		ImGui::OpenPopup("##context_menu");

	isOpenContextMenuInSceneEntities = false;

	if (ImGui::BeginPopup("##context_menu"))
	{
		isOpenContextMenuInSceneEntities = true;

		if (sceneObjectHoveredIndex == -1)
		{
			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::MenuItem("Entity"))
				{
					selectGameModelPopUp::getInstance().show(nullptr, true);
				}

				if (ImGui::MenuItem("Instanced entity"))
				{
					selectGameModelPopUp::getInstance().show(nullptr, true, true);
				}

				if (ImGui::MenuItem("Terrain"))
				{
					std::vector<std::string> terrainList = SCENE.getTerrainList();
					size_t nextID = terrainList.size();
					size_t index = 0;
					std::string newName = "terrain_" + std::to_string(nextID + index);

					while (true)
					{
						bool correctName = true;
						for (size_t i = 0; i < terrainList.size(); i++)
						{
							if (terrainList[i] == newName)
							{
								correctName = false;
								break;
							}
						}

						if (correctName)
							break;

						index++;
						newName = "terrain_" + std::to_string(nextID + index);
					}

					FETerrain* newTerrain = RESOURCE_MANAGER.createTerrain(true, newName);
					SCENE.addTerrain(newTerrain);
					newTerrain->heightMap->setDirtyFlag(true);
					PROJECT_MANAGER.getCurrent()->modified = true;
				}

				if (ImGui::BeginMenu("Light"))
				{
					if (ImGui::MenuItem("Directional"))
					{
						SCENE.addLight(FE_DIRECTIONAL_LIGHT, "");
					}

					if (ImGui::MenuItem("Spot"))
					{
						SCENE.addLight(FE_SPOT_LIGHT, "");
					}

					if (ImGui::MenuItem("Point"))
					{
						SCENE.addLight(FE_POINT_LIGHT, "");
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}
		}
		else
		{
			if (ImGui::MenuItem("Rename"))
			{
				if (SCENE.getEntity(sceneObjectsList[sceneObjectHoveredIndex]) != nullptr)
				{
					renamePopUp::getInstance().show(SCENE.getEntity(sceneObjectsList[sceneObjectHoveredIndex]));
				}
				else if (SCENE.getTerrain(sceneObjectsList[sceneObjectHoveredIndex]) != nullptr)
				{
					renamePopUp::getInstance().show(SCENE.getTerrain(sceneObjectsList[sceneObjectHoveredIndex]));
				}
				else if (SCENE.getLight(sceneObjectsList[sceneObjectHoveredIndex]) != nullptr)
				{
					renamePopUp::getInstance().show(SCENE.getLight(sceneObjectsList[sceneObjectHoveredIndex]));
					//renameLightWindow.show(SCENE.getLight(sceneObjectsList[sceneObjectHoveredIndex]));
				}
			}

			if (ImGui::MenuItem("Delete"))
			{
				if (SCENE.getEntity(sceneObjectsList[sceneObjectHoveredIndex]) != nullptr)
				{
					FEEntity* entity = SCENE.getEntity(sceneObjectsList[sceneObjectHoveredIndex]);
					if (SELECTED.getEntity() == entity)
						SELECTED.clear();

					SCENE.deleteEntity(entity->getObjectID());
				}
				else if (SCENE.getTerrain(sceneObjectsList[sceneObjectHoveredIndex]) != nullptr)
				{
					FETerrain* terrain = SCENE.getTerrain(sceneObjectsList[sceneObjectHoveredIndex]);
					if (SELECTED.getTerrain() == terrain)
						SELECTED.clear();

					SCENE.deleteTerrain(terrain->getObjectID());
				}
				else if (SCENE.getLight(sceneObjectsList[sceneObjectHoveredIndex]) != nullptr)
				{
					FELight* light = SCENE.getLight(sceneObjectsList[sceneObjectHoveredIndex]);
					if (SELECTED.getLight() == light)
						SELECTED.clear();

					SCENE.deleteLight(light->getObjectID());
				}
			}
		}

		ImGui::EndPopup();
	}

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
	if (SELECTED.getSelected() != nullptr && displaySelectedObjAABB)
	{
		FEAABB selectedAABB = SELECTED.getEntity() != nullptr ? SELECTED.getEntity()->getAABB() : SELECTED.getTerrain()->getAABB();
		RENDERER.drawAABB(selectedAABB);
	}

	ImGui::PopStyleVar();
	ImGui::End();

	selectMeshPopUp::getInstance().render();
	selectMaterialPopUp::getInstance().render();
	selectGameModelPopUp::getInstance().render();
	renamePopUp::getInstance().render();
	projectWasModifiedPopUp::getInstance().render();
}

void FEEditor::initializeResources()
{
	ENGINE.setKeyCallback(keyButtonCallback);
	ENGINE.setMouseButtonCallback(mouseButtonCallback);
	ENGINE.setMouseMoveCallback(mouseMoveCallback);
	//ENGINE.setWindowResizeCallback(windowResizeCallback);
	ENGINE.setRenderTargetResizeCallback(renderTargetResizeCallback);

	SELECTED.initializeResources();
	ENGINE.getCamera()->setIsInputActive(isCameraInputActive);
	PROJECT_MANAGER.initializeResources();
	PREVIEW_MANAGER.initializeResources();
	DRAG_AND_DROP_MANAGER.initializeResources();
	sceneWindowTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_GAMEMODEL, sceneWindowDragAndDropCallBack, nullptr, "Drop to add to scene");

	// **************************** Gizmos ****************************
	GIZMO_MANAGER.initializeResources();

	// hide all resources for gizmos from content browser
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(RESOURCE_MANAGER.getMesh("45191B6F172E3B531978692E"/*"transformationGizmoMesh"*/));
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(RESOURCE_MANAGER.getMesh("637C784B2E5E5C6548190E1B"/*"scaleGizmoMesh"*/));
	EDITOR_INTERNAL_RESOURCES.addResourceToInternalEditorList(RESOURCE_MANAGER.getMesh("19622421516E5B317E1B5360"/*"rotateGizmoMesh"*/));

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

	entitySceneBrowserIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/entitySceneBrowserIcon.png", "entitySceneBrowserIcon");
	RESOURCE_MANAGER.makeTextureStandard(entitySceneBrowserIcon);
	instancedEntitySceneBrowserIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/instancedEntitySceneBrowserIcon.png", "instancedEntitySceneBrowserIcon");
	RESOURCE_MANAGER.makeTextureStandard(instancedEntitySceneBrowserIcon);

	directionalLightSceneBrowserIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/directionalLightSceneBrowserIcon.png", "directionalLightSceneBrowserIcon");
	RESOURCE_MANAGER.makeTextureStandard(directionalLightSceneBrowserIcon);
	spotLightSceneBrowserIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/spotLightSceneBrowserIcon.png", "spotLightSceneBrowserIcon");
	RESOURCE_MANAGER.makeTextureStandard(spotLightSceneBrowserIcon);
	pointLightSceneBrowserIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/pointLightSceneBrowserIcon.png", "pointLightSceneBrowserIcon");
	RESOURCE_MANAGER.makeTextureStandard(pointLightSceneBrowserIcon);

	terrainSceneBrowserIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/terrainSceneBrowserIcon.png", "terrainSceneBrowserIcon");
	RESOURCE_MANAGER.makeTextureStandard(terrainSceneBrowserIcon);

	cameraSceneBrowserIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/cameraSceneBrowserIcon.png", "cameraSceneBrowserIcon");
	RESOURCE_MANAGER.makeTextureStandard(cameraSceneBrowserIcon);

	folderIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/folderIcon.png", "folderIcon");
	RESOURCE_MANAGER.makeTextureStandard(folderIcon);

	shaderIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/shaderIcon.png", "shaderIcon");
	RESOURCE_MANAGER.makeTextureStandard(shaderIcon);

	VFSBackIcon = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/VFSBackIcon.png", "VFSBackIcon");
	RESOURCE_MANAGER.makeTextureStandard(VFSBackIcon);
	
	ENGINE.getCamera()->setOnUpdate(onCameraUpdate);
	ENGINE.setWindowCloseCallback(closeWindowCallBack);
}

void FEEditor::mouseMoveCallback(double xpos, double ypos)
{
	EDITOR.setLastMouseX(EDITOR.getMouseX());
	EDITOR.setLastMouseY(EDITOR.getMouseY());

	EDITOR.setMouseX(xpos);
	EDITOR.setMouseY(ypos);

	DRAG_AND_DROP_MANAGER.mouseMove();

	if (SELECTED.getSelected() != nullptr)
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
	DRAG_AND_DROP_MANAGER.render();

	if (PROJECT_MANAGER.getCurrent())
	{
		if (gameMode)
			return;

		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save project"))
				{
					PROJECT_MANAGER.getCurrent()->saveScene();
					ENGINE.takeScreenshot((PROJECT_MANAGER.getCurrent()->getProjectFolder() + "projectScreenShot.texture").c_str());
				}

				if (ImGui::MenuItem("Close project"))
				{
					if (PROJECT_MANAGER.getCurrent()->modified)
					{
						projectWasModifiedPopUp::getInstance().show(PROJECT_MANAGER.getCurrent());
					}
					else
					{
						PROJECT_MANAGER.closeCurrentProject();

						ImGui::PopStyleVar();
						ImGui::EndMenu();
						ImGui::EndMainMenuBar();

						return;
					}
				}

				if (ImGui::MenuItem("Exit"))
				{
					ENGINE.terminate();
				}
				
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				if (ImGui::MenuItem("Scene Entities", NULL, sceneBrowserVisible))
				{
					sceneBrowserVisible = !sceneBrowserVisible;
				}

				if (ImGui::MenuItem("Inspector", NULL, inspectorVisible))
				{
					inspectorVisible = !inspectorVisible;
				}

				if (ImGui::MenuItem("Content Browser", NULL, contentBrowserVisible))
				{
					contentBrowserVisible = !contentBrowserVisible;
				}

				if (ImGui::MenuItem("Effects", NULL, effectsWindowVisible))
				{
					effectsWindowVisible = !effectsWindowVisible;
				}

				if (ImGui::MenuItem("Log", NULL, logWindowVisible))
				{
					logWindowVisible = !logWindowVisible;
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
		ImGui::PopStyleVar();
		//ImGui::PopStyleVar();

		ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_None | ImGuiWindowFlags_NoScrollbar);
		
		sceneWindow = ImGui::GetCurrentWindow();
		sceneWindowTarget->stickToCurrentWindow();
		
		ENGINE.setRenderTargetWidth((int)sceneWindow->ContentRegionRect.GetWidth());
		ENGINE.setRenderTargetHeight((int)sceneWindow->ContentRegionRect.GetHeight());

		ENGINE.setRenderTargetXShift((int)sceneWindow->ContentRegionRect.GetTL().x);
		ENGINE.setRenderTargetYShift((int)sceneWindow->ContentRegionRect.GetTL().y);

		ENGINE.renderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera()));

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowBorderSize = 0.0f;
		style.WindowPadding = ImVec2(0.0f, 0.0f);

		if (RENDERER.finalScene != nullptr)
		{
			ImGui::Image((void*)(intptr_t)RENDERER.finalScene->getTextureID(), ImVec2(ImGui::GetCurrentWindow()->ContentRegionRect.GetWidth(), ImGui::GetCurrentWindow()->ContentRegionRect.GetHeight()), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
		else if (RENDERER.sceneToTextureFB->getColorAttachment() != nullptr)
		{
			ImGui::Image((void*)(intptr_t)RENDERER.sceneToTextureFB->getColorAttachment()->getTextureID(), ImVec2(ImGui::GetCurrentWindow()->ContentRegionRect.GetWidth(), ImGui::GetCurrentWindow()->ContentRegionRect.GetHeight()), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		}
		// Something went terribly wrong!
		else
		{

		}

		ImGui::End();

		displaySceneBrowser();
		displayContentBrowser();
		displayInspector();
		displayEffectsWindow();
		displayLogWindow();
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
		projectWasModifiedPopUp::getInstance().show(PROJECT_MANAGER.getCurrent());
	}
	else
	{
		PROJECT_MANAGER.closeCurrentProject();
		ENGINE.terminate();
		return;
	}
}

//void FEEditor::windowResizeCallback(int newW, int newH)
//{
//	SELECTED.reInitializeResources();
//}

void FEEditor::renderTargetResizeCallback(int newW, int newH)
{
	ENGINE.renderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera()));
	SELECTED.reInitializeResources();
}

bool FEEditor::isInGameMode()
{
	return gameMode;
}

void FEEditor::setGameMode(bool gameMode)
{
	this->gameMode = gameMode;
	if (this->gameMode)
	{
		ENGINE.setRenderTargetMode(FE_GLFW_MODE);
		ENGINE.renderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera()));
	}
	else
	{
		ENGINE.setRenderTargetMode(FE_CUSTOM_MODE);
		ENGINE.renderTargetCenterForCamera(reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera()));
	}
}

bool entityChangeGameModelTargetCallBack(FEObject* object, void** entityPointer)
{
	FEEntity* entity = SELECTED.getEntity();
	if (entity == nullptr)
		return false;

	entity->gameModel = (RESOURCE_MANAGER.getGameModel(object->getObjectID()));
	return true;
}

bool terrainChangeMaterialTargetCallBack(FEObject* object, void** terrainPointer)
{
	FETerrain* terrain = SELECTED.getTerrain();
	if (terrain == nullptr)
		return false;

	terrain->layer0 = (RESOURCE_MANAGER.getMaterial(object->getObjectID()));
	return true;
}

void FEEditor::displayInspector()
{
	static DragAndDropTarget* entityChangeGameModelTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_GAMEMODEL, entityChangeGameModelTargetCallBack, nullptr, "Drop to assign game model");
	static DragAndDropTarget* terrainChangeMaterialTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_MATERIAL, terrainChangeMaterialTargetCallBack, nullptr, "Drop to assign material");

	if (!inspectorVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
	
	if (SELECTED.getSelected() == nullptr)
	{
		ImGui::PopStyleVar();
		ImGui::End();
		return;
	}

	if (SELECTED.getEntity() != nullptr)
	{
		FEEntity* entity = SELECTED.getEntity();

		if (entity->getType() == FE_ENTITY)
		{
			showTransformConfiguration(entity, &entity->transform);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			ImGui::Separator();
			ImGui::Text("Game model : ");
			FETexture* previewTexture = PREVIEW_MANAGER.getGameModelPreview(entity->gameModel->getObjectID());
			if (ImGui::ImageButton((void*)(intptr_t)previewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
			{
				selectGameModelPopUp::getInstance().show(&entity->gameModel);
			}
			entityChangeGameModelTarget->stickToItem();
			ImGui::Separator();

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
		}
		else if (entity->getType() == FE_ENTITY_INSTANCED)
		{
			FEEntityInstanced* instancedEntity = reinterpret_cast<FEEntityInstanced*>(entity);

			if (SELECTED.instancedSubObjectIndexSelected != -1)
			{
				std::string instancedSubObjectInfo = "index: ";

				ImGui::Text("Selected instance info:");
				instancedSubObjectInfo = "index: " + std::to_string(SELECTED.instancedSubObjectIndexSelected);
				ImGui::Text(instancedSubObjectInfo.c_str());

				FETransformComponent tempTransform = FETransformComponent(instancedEntity->getTransformedInstancedMatrix(SELECTED.instancedSubObjectIndexSelected));
				showTransformConfiguration("selected instance", &tempTransform);
				instancedEntity->modifyInstance(SELECTED.instancedSubObjectIndexSelected, tempTransform.getTransformMatrix());

				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.55f, 0.55f, 0.95f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.75f, 0.75f, 0.95f));

				if (ImGui::ImageButton((void*)(intptr_t)arrowToGroundIcon->getTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					instancedEntity->tryToSnapInstance(SELECTED.instancedSubObjectIndexSelected);
				}
				showToolTip("Selected instance will attempt to snap to the terrain.");

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
			else
			{
				showTransformConfiguration(entity, &entity->transform);

				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

				ImGui::Separator();
				ImGui::Text("Game model : ");
				FETexture* previewTexture = PREVIEW_MANAGER.getGameModelPreview(entity->gameModel->getObjectID());
				if (ImGui::ImageButton((void*)(intptr_t)previewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					selectGameModelPopUp::getInstance().show(&entity->gameModel);
				}
				entityChangeGameModelTarget->stickToItem();
				ImGui::Separator();

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();

				ImGui::Text("Snapped to: ");
				ImGui::SameLine();

				std::vector<std::string> terrainList = SCENE.getTerrainList();
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
						if (ImGui::Selectable(SCENE.getTerrain(terrainList[i])->getName().c_str(), is_selected))
						{
							SCENE.getTerrain(terrainList[i])->snapInstancedEntity(instancedEntity);
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
						SELECTED.setSelected(instancedEntity);
					}
				}
				showToolTip("Individual selection mode - Used to select individual instances.");

				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
				ImGui::PopStyleColor();
			}
		}
	}
	else if (SELECTED.getTerrain() != nullptr)
	{
		static ImGuiButton* loadHeightMapButton = new ImGuiButton("Load HeightMap");
		static ImGuiButton* testButton = new ImGuiButton("testButton");

		static ImGuiImageButton* sculptBrushButton = new ImGuiImageButton(nullptr);
		static ImGuiImageButton* levelBrushButton = new ImGuiImageButton(nullptr);
		static ImGuiImageButton* smoothBrushButton = new ImGuiImageButton(nullptr);
		static bool firstCall = true;
		if (firstCall)
		{
			sculptBrushButton->setTexture(sculptBrushIcon);
			sculptBrushButton->setSize(ImVec2(24, 24));

			levelBrushButton->setTexture(levelBrushIcon);
			levelBrushButton->setSize(ImVec2(24, 24));

			smoothBrushButton->setTexture(smoothBrushIcon);
			smoothBrushButton->setSize(ImVec2(24, 24));

			loadHeightMapButton->setSize(ImVec2(160, 0));

			firstCall = false;
		}

		FETerrain* currentTerrain = SELECTED.getTerrain();

		bool isActive = currentTerrain->isWireframeMode();
		ImGui::Checkbox("WireframeMode", &isActive);
		currentTerrain->setWireframeMode(isActive);

		loadHeightMapButton->render();
		if (loadHeightMapButton->getWasClicked())
		{
			std::string filePath = "";
			FILESYSTEM.openDialog(filePath, textureLoadFilter, 1);

			if (filePath != "")
			{
				FETexture* loadedTexture = RESOURCE_MANAGER.LoadPNGHeightmap(filePath.c_str(), currentTerrain);
				if (loadedTexture == RESOURCE_MANAGER.noTexture)
				{
					LOG.add(std::string("can't load height map: ") + filePath, FE_LOG_ERROR, FE_LOG_LOADING);
				}
				else
				{
					loadedTexture->setDirtyFlag(true);
					PROJECT_MANAGER.getCurrent()->modified = true;
					//RESOURCE_MANAGER.saveFETexture(loadedTexture, (PROJECT_MANAGER.getCurrent()->getProjectFolder() + loadedTexture->getName() + ".FETexture").c_str());
				}
			}
		}

#ifdef USE_DEFERRED_RENDERER
		int iData = *(int*)RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("debugFlag")->data;
		ImGui::SliderInt("debugFlag", &iData, 0, 10);
		RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/)->getParameter("debugFlag")->updateData(iData);
#else
		int iData = *(int*)currentTerrain->shader->getParameter("debugFlag")->data;
		ImGui::SliderInt("debugFlag", &iData, 0, 10);
		currentTerrain->shader->getParameter("debugFlag")->updateData(iData);
#endif // USE_DEFERRED_RENDERER

		float highScale = currentTerrain->getHightScale();
		ImGui::DragFloat("highScale", &highScale);
		currentTerrain->setHightScale(highScale);

		float displacementScale = currentTerrain->getDisplacementScale();
		ImGui::DragFloat("displacementScale", &displacementScale, 0.02f, -10.0f, 10.0f);
		currentTerrain->setDisplacementScale(displacementScale);

		float LODlevel = currentTerrain->getLODlevel();
		ImGui::DragFloat("LODlevel", &LODlevel, 2.0f, 2.0f, 128.0f);
		currentTerrain->setLODlevel(LODlevel);
		showToolTip("Bigger LODlevel more details terraine will have and less performance you will get.");

		glm::vec2 tileMult = currentTerrain->getTileMult();
		ImGui::DragFloat2("tile multiplicator", &tileMult[0], 0.1f, 1.0f, 100.0f);
		currentTerrain->setTileMult(tileMult);

		float chunkPerSide = currentTerrain->getChunkPerSide();
		ImGui::DragFloat("chunkPerSide", &chunkPerSide, 2.0f, 1.0f, 16.0f);
		currentTerrain->setChunkPerSide(chunkPerSide);
		
		ImGui::Separator();
		ImGui::Text("Material : ");
		FETexture* previewTexture = PREVIEW_MANAGER.getMaterialPreview(currentTerrain->layer0->getObjectID());
		if (ImGui::ImageButton((void*)(intptr_t)previewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			selectMaterialPopUp::getInstance().show(&currentTerrain->layer0);
		}
		terrainChangeMaterialTarget->stickToItem();
		ImGui::Separator();

		showTransformConfiguration(currentTerrain, &currentTerrain->transform);

		// ********************* REAL WORLD COMPARISON SCALE *********************
		FEAABB realAABB = currentTerrain->getAABB();
		glm::vec3 min = realAABB.getMin();
		glm::vec3 max = realAABB.getMax();

		float xSize = sqrt((max.x - min.x) * (max.x - min.x));
		float ySize = sqrt((max.y - min.y) * (max.y - min.y));
		float zSize = sqrt((max.z - min.z) * (max.z - min.z));

		std::string sizeInM = "Approximate terrain size: ";
		sizeInM += std::to_string(std::max(xSize, std::max(ySize, zSize)));
		sizeInM += " m";
		ImGui::Text(sizeInM.c_str());
		// ********************* REAL WORLD COMPARISON SCALE END *********************

		ImGui::Separator();

		float currentBrushSize = currentTerrain->getBrushSize();
		ImGui::DragFloat("brushSize", &currentBrushSize, 0.1f, 0.01f, 100.0f);
		currentTerrain->setBrushSize(currentBrushSize);

		float currentBrushIntensity = currentTerrain->getBrushIntensity();
		ImGui::DragFloat("brushIntensity", &currentBrushIntensity, 0.0001f, 0.0001f, 10.0f);
		currentTerrain->setBrushIntensity(currentBrushIntensity);

		currentTerrain->isBrushSculptMode() ? setSelectedStyle(sculptBrushButton) : setDefaultStyle(sculptBrushButton);
		sculptBrushButton->render();
		showToolTip("Sculpt Brush. Left mouse to increase height, hold shift to decrease height.");

		if (sculptBrushButton->getWasClicked())
		{
			currentTerrain->setBrushSculptMode(!currentTerrain->isBrushSculptMode());
			if (currentTerrain->isBrushSculptMode())
			{
				currentTerrain->setBrushLevelMode(false);
				currentTerrain->setBrushSmoothMode(false);
			}
		}

		currentTerrain->isBrushLevelMode() ? setSelectedStyle(levelBrushButton) : setDefaultStyle(levelBrushButton);
		ImGui::SameLine();
		levelBrushButton->render();
		showToolTip("Level Brush.");

		if (levelBrushButton->getWasClicked())
		{
			currentTerrain->setBrushLevelMode(!currentTerrain->isBrushLevelMode());
			if (currentTerrain->isBrushLevelMode())
			{
				currentTerrain->setBrushSculptMode(false);
				currentTerrain->setBrushSmoothMode(false);
			}
		}

		currentTerrain->isBrushSmoothMode() ? setSelectedStyle(smoothBrushButton) : setDefaultStyle(smoothBrushButton);
		ImGui::SameLine();
		smoothBrushButton->render();
		showToolTip("Smooth Brush.");

		if (smoothBrushButton->getWasClicked())
		{
			currentTerrain->setBrushSmoothMode(!currentTerrain->isBrushSmoothMode());
			if (currentTerrain->isBrushSmoothMode())
			{
				currentTerrain->setBrushSculptMode(false);
				currentTerrain->setBrushLevelMode(false);
			}
		}

		if (currentTerrain->isBrushSculptMode() || currentTerrain->isBrushLevelMode() || currentTerrain->isBrushSmoothMode())
		{
			// to hide gizmos
			if (SELECTED.getTerrain() != nullptr)
				SELECTED.setSelected(SELECTED.getTerrain());

			currentTerrain->setBrushActive(EDITOR.leftMousePressed);
			currentTerrain->setBrushInversed(EDITOR.shiftPressed);
		}
		else
		{
			// to show gizmos
			if (SELECTED.getTerrain() != nullptr)
				SELECTED.setSelected(SELECTED.getTerrain());
		}

		selectMaterialPopUp::getInstance().render();
	}
	else if (SELECTED.getLight() != nullptr)
	{
		displayLightProperties(SELECTED.getLight());
	}
	else if (SELECTED.getSelected()->getType() == FE_CAMERA)
	{
		float cameraSpeed = reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera())->getSpeed();
		ImGui::DragFloat("Camera speed in m/s", &cameraSpeed, 0.01f, 0.01f, 100.0f);
		reinterpret_cast<FEFreeCamera*>(ENGINE.getCamera())->setSpeed(cameraSpeed);
	}

	ImGui::PopStyleVar();
	ImGui::End();
}

void FEEditor::displayEffectsWindow()
{
	if (!effectsWindowVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Effects settings", nullptr, ImGuiWindowFlags_None);

	int GUIID = 0;
	static float buttonWidth = 80.0f;
	static float fieldWidth = 250.0f;

	static ImGuiButton* resetButton = new ImGuiButton("Reset");
	static bool firstCall = true;
	if (firstCall)
	{
		resetButton->setSize(ImVec2(buttonWidth, 28.0f));
		firstCall = false;
	}

	if (ImGui::CollapsingHeader("Gamma Correction & Exposure", 0))
	{
		ImGui::Text("Gamma Correction:");
		float FEGamma = ENGINE.getCamera()->getGamma();
		ImGui::SetNextItemWidth(fieldWidth);
		ImGui::DragFloat("##Gamma Correction", &FEGamma, 0.01f, 0.001f, 10.0f);
		ENGINE.getCamera()->setGamma(FEGamma);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		resetButton->render();
		if (resetButton->getWasClicked())
		{
			ENGINE.getCamera()->setGamma(2.2f);
		}
		ImGui::PopID();

		ImGui::Text("Exposure:");
		float FEExposure = ENGINE.getCamera()->getExposure();
		ImGui::SetNextItemWidth(fieldWidth);
		ImGui::DragFloat("##Exposure", &FEExposure, 0.01f, 0.001f, 100.0f);
		ENGINE.getCamera()->setExposure(FEExposure);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		resetButton->render();
		if (resetButton->getWasClicked())
		{
			ENGINE.getCamera()->setExposure(1.0f);
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Anti-Aliasing(FXAA)", 0))
	{
		static const char* options[5] = { "none", "1x", "2x", "4x", "8x" };
		static std::string selectedOption = "1x";
		//FEPostProcess* PPEffect = RENDERER.getPostProcessEffect("FE_FXAA");

		static bool firstLook = true;
		if (firstLook)
		{
			float FXAASpanMax = RENDERER.getFXAASpanMax();
			if (FXAASpanMax == 0.0f)
			{
				selectedOption = options[0];
			}
			else if (FXAASpanMax > 0.1f && FXAASpanMax < 1.1f)
			{
				selectedOption = options[1];
			}
			else if (FXAASpanMax > 1.1f && FXAASpanMax < 2.1f)
			{
				selectedOption = options[2];
			}
			else if (FXAASpanMax > 2.1f && FXAASpanMax < 4.1f)
			{
				selectedOption = options[3];
			}
			else if (FXAASpanMax > 4.1f && FXAASpanMax < 8.1f)
			{
				selectedOption = options[4];
			}
			else
			{
				selectedOption = options[5];
			}

			firstLook = false;
		}

		static bool debugSettings = false;
		if (ImGui::Checkbox("debug view", &debugSettings))
		{
			float FXAASpanMax = RENDERER.getFXAASpanMax();
			if (FXAASpanMax == 0.0f)
			{
				selectedOption = options[0];
			}
			else if (FXAASpanMax > 0.1f && FXAASpanMax < 1.1f)
			{
				selectedOption = options[1];
			}
			else if (FXAASpanMax > 1.1f && FXAASpanMax < 2.1f)
			{
				selectedOption = options[2];
			}
			else if (FXAASpanMax > 2.1f && FXAASpanMax < 4.1f)
			{
				selectedOption = options[3];
			}
			else if (FXAASpanMax > 4.1f && FXAASpanMax < 8.1f)
			{
				selectedOption = options[4];
			}
			else
			{
				selectedOption = options[5];
			}
		}

		if (!debugSettings)
		{
			ImGui::Text("Anti Aliasing Strength:");
			if (ImGui::BeginCombo("##Anti Aliasing Strength", selectedOption.c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < 5; i++)
				{
					bool is_selected = (selectedOption == options[i]);
					if (ImGui::Selectable(options[i], is_selected))
					{
						RENDERER.setFXAASpanMax(float(pow(2.0, (i - 1))));
						if (i == 0)
							RENDERER.setFXAASpanMax(0.0f);
						selectedOption = options[i];
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else
		{
			ImGui::Text("FXAASpanMax:");
			ImGui::SetNextItemWidth(fieldWidth);
			float FXAASpanMax = RENDERER.getFXAASpanMax();
			ImGui::DragFloat("##FXAASpanMax", &FXAASpanMax, 0.0f, 0.001f, 32.0f);
			RENDERER.setFXAASpanMax(FXAASpanMax);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			resetButton->render();
			if (resetButton->getWasClicked())
			{
				RENDERER.setFXAASpanMax(8.0f);
			}
			ImGui::PopID();

			ImGui::Text("FXAAReduceMin:");
			ImGui::SetNextItemWidth(fieldWidth);
			float FXAAReduceMin = RENDERER.getFXAAReduceMin();
			ImGui::DragFloat("##FXAAReduceMin", &FXAAReduceMin, 0.01f, 0.001f, 100.0f);
			RENDERER.setFXAAReduceMin(FXAAReduceMin);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			resetButton->render();
			if (resetButton->getWasClicked())
			{
				RENDERER.setFXAAReduceMin(0.008f);
			}
			ImGui::PopID();

			ImGui::Text("FXAAReduceMul:");
			ImGui::SetNextItemWidth(fieldWidth);
			float FXAAReduceMul = RENDERER.getFXAAReduceMul();
			ImGui::DragFloat("##FXAAReduceMul", &FXAAReduceMul, 0.01f, 0.001f, 100.0f);
			RENDERER.setFXAAReduceMul(FXAAReduceMul);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			resetButton->render();
			if (resetButton->getWasClicked())
			{
				RENDERER.setFXAAReduceMul(0.400f);
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Bloom", 0))
	{
		ImGui::Text("Threshold:");
		float Threshold = RENDERER.getBloomThreshold();
		ImGui::SetNextItemWidth(fieldWidth);
		ImGui::DragFloat("##Threshold", &Threshold, 0.01f, 0.001f, 30.0f);
		RENDERER.setBloomThreshold(Threshold);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		resetButton->render();
		if (resetButton->getWasClicked())
		{
			RENDERER.setBloomThreshold(1.5f);
		}
		ImGui::PopID();

		ImGui::Text("Size:");
		float Size = RENDERER.getBloomSize();
		ImGui::SetNextItemWidth(fieldWidth);
		ImGui::DragFloat("##BloomSize", &Size, 0.01f, 0.001f, 100.0f);
		RENDERER.setBloomSize(Size);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		resetButton->render();
		if (resetButton->getWasClicked())
		{
			RENDERER.setBloomSize(5.0f);
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Depth of Field", 0))
	{
		ImGui::Text("Near distance:");
		ImGui::SetNextItemWidth(fieldWidth);
		float depthThreshold = RENDERER.getDOFNearDistance();
		ImGui::DragFloat("##depthThreshold", &depthThreshold, 0.0f, 0.001f, 100.0f);
		RENDERER.setDOFNearDistance(depthThreshold);

		ImGui::Text("Far distance:");
		ImGui::SetNextItemWidth(fieldWidth);
		float depthThresholdFar = RENDERER.getDOFFarDistance();
		ImGui::DragFloat("##depthThresholdFar", &depthThresholdFar, 0.0f, 0.001f, 100.0f);
		RENDERER.setDOFFarDistance(depthThresholdFar);

		ImGui::Text("Strength:");
		ImGui::SetNextItemWidth(fieldWidth);
		float Strength = RENDERER.getDOFStrength();
		ImGui::DragFloat("##Strength", &Strength, 0.0f, 0.001f, 10.0f);
		RENDERER.setDOFStrength(Strength);

		ImGui::Text("Distance dependent strength:");
		ImGui::SetNextItemWidth(fieldWidth);
		float intMult = RENDERER.getDOFDistanceDependentStrength();
		ImGui::DragFloat("##Distance dependent strength", &intMult, 0.0f, 0.001f, 100.0f);
		RENDERER.setDOFDistanceDependentStrength(intMult);
	}

	if (ImGui::CollapsingHeader("Distance fog", 0))
	{
		bool enabledFog = RENDERER.isDistanceFogEnabled();
		if (ImGui::Checkbox("Enable fog", &enabledFog))
		{
			RENDERER.setDistanceFogEnabled(enabledFog);
		}

		if (enabledFog)
		{
			ImGui::Text("Density:");
			ImGui::SetNextItemWidth(fieldWidth);
			float fogDensity = RENDERER.getDistanceFogDensity();
			ImGui::DragFloat("##fogDensity", &fogDensity, 0.001f, 0.0f, 5.0f);
			RENDERER.setDistanceFogDensity(fogDensity);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			resetButton->render();
			if (resetButton->getWasClicked())
			{
				RENDERER.setDistanceFogDensity(0.007f);
			}
			ImGui::PopID();

			ImGui::Text("Gradient:");
			ImGui::SetNextItemWidth(fieldWidth);
			float fogGradient = RENDERER.getDistanceFogGradient();
			ImGui::DragFloat("##fogGradient", &fogGradient, 0.001f, 0.0f, 5.0f);
			RENDERER.setDistanceFogGradient(fogGradient);

			ImGui::PushID(GUIID++);
			ImGui::SameLine();
			resetButton->render();
			if (resetButton->getWasClicked())
			{
				RENDERER.setDistanceFogGradient(2.5f);
			}
			ImGui::PopID();
		}
	}

	if (ImGui::CollapsingHeader("Chromatic Aberration", 0))
	{
		ImGui::Text("Shift strength:");
		ImGui::SetNextItemWidth(fieldWidth);
		float intensity = RENDERER.getChromaticAberrationIntensity();
		ImGui::DragFloat("##intensity", &intensity, 0.01f, 0.0f, 30.0f);
		RENDERER.setChromaticAberrationIntensity(intensity);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		resetButton->render();
		if (resetButton->getWasClicked())
		{
			RENDERER.setChromaticAberrationIntensity(1.0f);
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Sky", 0))
	{
		bool enabledSky = RENDERER.isSkyEnabled();
		if (ImGui::Checkbox("enable sky", &enabledSky))
		{
			RENDERER.setSkyEnabld(enabledSky);
		}

		ImGui::Text("Sphere size:");
		ImGui::SetNextItemWidth(fieldWidth);
		float size = RENDERER.getDistanceToSky();
		ImGui::DragFloat("##Sphere size", &size, 0.01f, 0.0f, 200.0f);
		RENDERER.setDistanceToSky(size);

		ImGui::PushID(GUIID++);
		ImGui::SameLine();
		resetButton->render();
		if (resetButton->getWasClicked())
		{
			RENDERER.setDistanceToSky(50.0f);
		}
		ImGui::PopID();
	}

	ImGui::PopStyleVar();
	ImGui::End();
}

void FEEditor::displayLogWindow()
{
	if (!logWindowVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Log", nullptr, ImGuiWindowFlags_None);

	static int selectedChannel = FE_LOG_GENERAL;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
	ImGui::Text("Channel:");
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5);
	if (ImGui::BeginCombo("##Channel", selectedChannel == -1 ? "ALL" : LOG.channelTypeToString(LOG_CHANNEL(selectedChannel)).c_str(), ImGuiWindowFlags_None))
	{
		for (int i = -1; i < LOG.channelCount; i++)
		{
			ImGui::PushID(i);

			if (i == -1)
			{
				bool is_selected = (selectedChannel == -1);

				if (ImGui::Selectable("ALL", is_selected))
				{
					selectedChannel = -1;
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			else
			{
				bool is_selected = (selectedChannel == LOG_CHANNEL(i));
				if (ImGui::Selectable(LOG.channelTypeToString(LOG_CHANNEL(i)).c_str(), is_selected))
				{
					selectedChannel = LOG_CHANNEL(i);
				}

				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			
			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	std::string logMessages;
	std::vector<LogItem> logItems;

	if (selectedChannel == -1)
	{
		std::vector<LogItem> tempItems;
		for (int i = 0; i < LOG.channelCount; i++)
		{
			tempItems = LOG.getLogItems(LOG_CHANNEL(i));
			for (size_t j = 0; j < tempItems.size(); j++)
			{
				logItems.push_back(tempItems[j]);
			}
		}
	}
	else
	{
		logItems = LOG.getLogItems(LOG_CHANNEL(selectedChannel));
	}

	std::sort(logItems.begin(), logItems.end(),
	[](const LogItem& a, const LogItem& b) -> bool
	{
		return a.timeStamp < b.timeStamp;
	});

	for (size_t i = 0; i < logItems.size(); i++)
	{
		logMessages += logItems[i].text;

		if (logItems[i].count < 1000)
		{
			logMessages += " | COUNT: " + std::to_string(logItems[i].count);
		}
		else
		{
			logMessages += " | COUNT: 1000+(Suppressed)";
		}

		logMessages += " | SEVERITY: " + LOG.severityLevelToString(logItems[i].severity);

		if (selectedChannel == -1)
		{
			logMessages += " | CHANNEL: " + LOG.channelTypeToString(logItems[i].channel);
		}

		if (i < logItems.size() - 1)
			logMessages += "\n";
	}

	static TextEditor logEditor;
	logEditor.SetReadOnly(true);
	logEditor.SetShowWhitespaces(false);
	logEditor.SetText(logMessages);
	logEditor.Render("Log messages");

	ImGui::PopStyleVar();
	ImGui::End();
}