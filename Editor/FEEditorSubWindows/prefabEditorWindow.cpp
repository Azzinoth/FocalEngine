#include "prefabEditorWindow.h"

prefabEditorWindow* prefabEditorWindow::_instance = nullptr;
FEPrefab* prefabEditorWindow::objToWorkWith = nullptr;

bool prefabEditorWindow::addGameModelTargetCallBack(FEObject* object, void** entityPointer)
{
	if (object == nullptr)
		return false;

	FEGameModel* newGameModel = reinterpret_cast<FEGameModel*>(object);
	objToWorkWith->addComponent(newGameModel);

	return true;
}

void prefabEditorWindow::showTransformConfiguration(FETransformComponent* transform, int index)
{
	// ********************* POSITION *********************
	glm::vec3 position = transform->getPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + std::to_string(index)).c_str(), &position[0], 0.1f);
	showToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + std::to_string(index)).c_str(), &position[1], 0.1f);
	showToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + std::to_string(index)).c_str(), &position[2], 0.1f);
	showToolTip("Z position");
	transform->setPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = transform->getRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + std::to_string(index)).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	showToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + std::to_string(index)).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	showToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + std::to_string(index)).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	showToolTip("Z rotation");
	transform->setRotation(rotation);

	// ********************* SCALE *********************
	ImGui::Checkbox("Uniform scaling", &transform->uniformScaling);
	glm::vec3 scale = transform->getScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + std::to_string(index)).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	showToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + std::to_string(index)).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	showToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + std::to_string(index)).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	showToolTip("Z scale");

	glm::vec3 oldScale = transform->getScale();
	transform->changeXScaleBy(scale[0] - oldScale[0]);
	transform->changeYScaleBy(scale[1] - oldScale[1]);
	transform->changeZScaleBy(scale[2] - oldScale[2]);
}

void prefabEditorWindow::addNewGameModelCallBack(std::vector<FEObject*> selectionsResult)
{
	for (int i = 0; i < selectionsResult.size(); i++)
	{
		if (selectionsResult[i]->getType() == FE_GAMEMODEL)
			objToWorkWith->addComponent(reinterpret_cast<FEGameModel*>(selectionsResult[i]));
	}	
}

prefabEditorWindow::prefabEditorWindow()
{
	std::string tempCaption = "Edit Prefab";
	strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());

	closeButton = new ImGuiButton("Close");
	closeButton->setSize(ImVec2(140, 24));
	closeButton->setPosition(ImVec2(800.0 / 2.0 - 140.0 / 2.0, 800.0 - 35.0));

	//addGameModelTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_GAMEMODEL, addGameModelTargetCallBack, nullptr, "Drop to add Game model");
}

prefabEditorWindow::~prefabEditorWindow()
{
	if (closeButton != nullptr)
		delete closeButton;
}

void prefabEditorWindow::show(FEPrefab* Prefab)
{
	size = ImVec2(800, 800);
	position = ImVec2(FEngine::getInstance().getWindowWidth() / 2 - size.x / 2, FEngine::getInstance().getWindowHeight() / 2 - size.y / 2);

	flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

	objToWorkWith = Prefab;
	
	FEImGuiWindow::show();
}

void prefabEditorWindow::render()
{
	FEImGuiWindow::render();

	if (!isVisible())
		return;

	if (ImGui::BeginTabBar("##prefabSettings", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Game Models"))
		{
			static bool contextMenuOpened = false;

			ImGui::BeginChildFrame(ImGui::GetID("GameModels ListBox Child"), ImVec2(ImGui::GetWindowContentRegionWidth() - 10.0f, 500.0f), ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			bool isListBoxHovered = false;
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
			{
				isListBoxHovered = true;
				//addGameModelTarget->setActive(true);
			}

			static bool openContextMenu = false;
			if (ImGui::IsMouseClicked(1))
			{
				if (isListBoxHovered)
				{
					openContextMenu = true;
				}
			}

			if (ImGui::BeginListBox("##GameModels ListBox", ImVec2(ImGui::GetWindowContentRegionWidth() - 10.0f, 500.0f)))
			{
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

				for (int i = 0; i < objToWorkWith->componentsCount(); i++)
				{
					FEGameModel* gameModel = objToWorkWith->getComponent(i)->gameModel;
					if (gameModel == nullptr)
						break;

					ImVec2 postionBeforeDraw = ImGui::GetCursorPos();

					ImVec2 textSize = ImGui::CalcTextSize(gameModel->getName().c_str());
					ImGui::SetCursorPos(postionBeforeDraw + ImVec2(ImGui::GetWindowContentRegionWidth() / 2.0f - textSize.x / 2.0f, 16));
					ImGui::Text(gameModel->getName().c_str());
					ImGui::SetCursorPos(postionBeforeDraw);

					ImGui::PushID(int(i));
					if (ImGui::Selectable("##item", selectedGameModel == i ? true : false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetWindowContentRegionWidth() - 0, 64)))
					{
						selectedGameModel = int(i);
					}
					ImGui::PopID();

					if (ImGui::IsItemHovered())
						hoveredGameModelItem = int(i);

					ImGui::SetCursorPos(postionBeforeDraw);
					ImColor imageTint = ImGui::IsItemHovered() ? ImColor(1.0f, 1.0f, 1.0f, 0.5f) : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
					FETexture* previewTexture = PREVIEW_MANAGER.getPreview(gameModel->getObjectID());
					ImGui::Image((void*)(intptr_t)previewTexture->getTextureID(), ImVec2(64, 64), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), imageTint);


					showTransformConfiguration(&objToWorkWith->getComponent(i)->transform, i);
				}

				ImGui::EndListBox();
				ImGui::PopFont();
			}
			

			ImGui::EndChildFrame();
			ImGui::EndTabItem();

			if (openContextMenu)
			{
				openContextMenu = false;
				ImGui::OpenPopup("##layers_listBox_context_menu");
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
			if (ImGui::BeginPopup("##layers_listBox_context_menu"))
			{
				contextMenuOpened = true;

				if (ImGui::MenuItem("Add game model..."))
				{
					selectFEObjectPopUp::getInstance().show(FE_GAMEMODEL, addNewGameModelCallBack);
				}

				if (hoveredGameModelItem != -1)
				{
					FEGameModel* gameModel = objToWorkWith->getComponent(hoveredGameModelItem)->gameModel;
					if (gameModel != nullptr)
					{
						ImGui::Separator();
						std::string layerName = gameModel->getName();
						ImGui::Text((std::string("Actions with ") + layerName).c_str());
						ImGui::Separator();

						if (ImGui::MenuItem("Remove"))
						{
							objToWorkWith->removeComponent(hoveredGameModelItem);
						}
					}
				}

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			if (!contextMenuOpened)
				hoveredGameModelItem = -1;

			contextMenuOpened = false;
		}
		ImGui::EndTabBar();
	}

	closeButton->render();
	if (closeButton->getWasClicked())
		close();
	




	/*windowPosition = ImGui::GetWindowPos();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	
	currentNodeArea->setAreaPosition(nodeGridRelativePosition);
	currentNodeArea->setAreaSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 67));
	currentNodeArea->update();

	if (ImGui::GetIO().MouseReleased[1])
		mousePositionWhenContextMenuWasOpened = ImVec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	
	nodeAreaTarget->stickToItem();

	float yPosition = ImGui::GetCursorPosY();
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 8.0f - 120.0f / 2.0f);
	if (ImGui::Button("Combine", ImVec2(120, 0)))
	{
		if (textureNode->getTexture() != RESOURCE_MANAGER.noTexture)
		{
			textureNode->getTexture()->setDirtyFlag(true);
			PROJECT_MANAGER.getCurrent()->addUnSavedObject(textureNode->getTexture());
			VIRTUAL_FILE_SYSTEM.createFile(textureNode->getTexture(), VIRTUAL_FILE_SYSTEM.getCurrentPath());
			NODE_SYSTEM.deleteNodeArea(currentNodeArea);
		}

		FEImGuiWindow::close();
	}

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));

	ImGui::SetCursorPosY(yPosition);
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 8.0f - 120.0f / 2.0f);
	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopStyleVar();

		FEImGuiWindow::close();
		return;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PopStyleVar();*/
	FEImGuiWindow::onRenderEnd();
}

//bool CombineChannelsToTexturePopUp::dragAndDropnodeAreaTargetCallback(FEObject* object, void** callbackInfo)
//{
//	FEEditorTextureSourceNode* newNode = new FEEditorTextureSourceNode(RESOURCE_MANAGER.getTexture(object->getObjectID()));
//	
//	ImVec2 positionOnCanvas;
//	positionOnCanvas.x = ImGui::GetMousePos().x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
//	positionOnCanvas.y = ImGui::GetMousePos().y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;
//
//	newNode->setPosition(positionOnCanvas);
//	currentNodeArea->addNode(newNode);
//	return true;
//}
//
//void CombineChannelsToTexturePopUp::nodeSystemMainContextMenu()
//{
//	if (ImGui::BeginMenu("Add"))
//	{
//		if (ImGui::MenuItem("Texture node"))
//		{
//			textureForNewNode = RESOURCE_MANAGER.noTexture;
//			selectTexturePopUp::getInstance().show(&textureForNewNode, textureNodeCreationCallback, reinterpret_cast<void*>(&textureForNewNode));
//		}
//
//		if (ImGui::MenuItem("Float node"))
//		{
//			FEEditorFloatSourceNode* newNode = new FEEditorFloatSourceNode();
//
//			ImVec2 positionOnCanvas;
//			positionOnCanvas.x = mousePositionWhenContextMenuWasOpened.x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
//			positionOnCanvas.y = mousePositionWhenContextMenuWasOpened.y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;
//
//			newNode->setPosition(positionOnCanvas);
//			currentNodeArea->addNode(newNode);
//		}
//
//		ImGui::EndMenu();
//	}
//}
//
//void CombineChannelsToTexturePopUp::textureNodeCreationCallback(void* texture)
//{
//	if (texture != nullptr && texture != RESOURCE_MANAGER.noTexture)
//	{
//		FEEditorTextureSourceNode* newNode = new FEEditorTextureSourceNode(*reinterpret_cast<FETexture**>(texture));
//
//		ImVec2 positionOnCanvas;
//		positionOnCanvas.x = mousePositionWhenContextMenuWasOpened.x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
//		positionOnCanvas.y = mousePositionWhenContextMenuWasOpened.y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;
//
//		newNode->setPosition(positionOnCanvas);
//		currentNodeArea->addNode(newNode);
//	}
//}