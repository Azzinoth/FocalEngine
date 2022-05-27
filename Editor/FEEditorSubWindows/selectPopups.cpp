#include "selectPopups.h"

selectFEObjectPopUp* selectFEObjectPopUp::_instance = nullptr;
bool selectFEObjectPopUp::controlButtonPressed = false;

selectFEObjectPopUp::selectFEObjectPopUp()
{
	popupCaption = "Select";
	iconButton = new ImGuiImageButton(nullptr);
	iconButton->setSize(ImVec2(128, 128));
	iconButton->setUV0(ImVec2(0.0f, 1.0f));
	iconButton->setUV1(ImVec2(1.0f, 0.0f));
	iconButton->setFramePadding(8);

	selectButton = new ImGuiButton("Select");
	selectButton->setSize(ImVec2(140, 24));
	selectButton->setPosition(ImVec2(500, 35));
	cancelButton = new ImGuiButton("Cancel");
	cancelButton->setSize(ImVec2(140, 24));
	cancelButton->setPosition(ImVec2(660, 35));

	ENGINE.addKeyCallback(selectFEObjectPopUp::keyButtonCallback);
}

selectFEObjectPopUp::~selectFEObjectPopUp()
{
	if (selectButton != nullptr)
		delete selectButton;

	if (cancelButton != nullptr)
		delete cancelButton;

	if (iconButton != nullptr)
		delete iconButton;
}

void selectFEObjectPopUp::show(FEObjectType type, void(*CallBack)(std::vector<FEObject*>), FEObject* HighlightedObject, std::vector<FEObject*> customList)
{
	currenType = type;
	if (currenType == FE_NULL)
		return;

	highlightedObject = HighlightedObject;
	callBack = CallBack;
	shouldOpen = true;

	itemsList.clear();

	if (customList.size() == 0)
	{
		std::vector<std::string> tempList;

		switch (currenType)
		{
			case FE_TEXTURE:
			{
				tempList = RESOURCE_MANAGER.getTextureList();
				tempList.insert(tempList.begin(), RESOURCE_MANAGER.noTexture->getObjectID());

				break;
			}

			case FE_MESH:
			{
				tempList = RESOURCE_MANAGER.getMeshList();

				std::vector<std::string> standardMeshList = RESOURCE_MANAGER.getStandardMeshList();
				for (size_t i = 0; i < standardMeshList.size(); i++)
				{
					if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(RESOURCE_MANAGER.getMesh(standardMeshList[i])))
						continue;

					tempList.push_back(standardMeshList[i]);
				}

				break;
			}
			

			case FE_MATERIAL:
			{
				tempList = RESOURCE_MANAGER.getMaterialList();
				tempList.insert(tempList.begin(), "18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);

				break;
			}
			

			case FE_GAMEMODEL:
			{
				tempList = RESOURCE_MANAGER.getGameModelList();
				break;
			}
			

			case FE_PREFAB:
			{
				tempList = RESOURCE_MANAGER.getPrefabList();
				break;
			}

		}
		
		for (size_t i = 0; i < tempList.size(); i++)
			itemsList.push_back(FEObjectManager::getInstance().getFEObject(tempList[i]));
	}
	else
	{
		itemsList = customList;
	}

	filteredItemsList = itemsList;
	strcpy_s(filter, "");

	if (highlightedObject != nullptr)
	{
		for (size_t i = 0; i < itemsList.size(); i++)
		{
			if (itemsList[i]->getObjectID() == highlightedObject->getObjectID())
			{
				IndexSelected = int(i);
				break;
			}
		}
	}
	else
	{
		IndexSelected = -1;
	}
}

void selectFEObjectPopUp::render()
{
	ImGuiModalPopup::render();

	ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosY(40);
		ImGui::Text("Filter: ");
		ImGui::SameLine();

		ImGui::SetCursorPosY(35);
		if (ImGui::InputText("selectFEObjectPopUpFilter", filter, IM_ARRAYSIZE(filter)))
		{
			if (strlen(filter) == 0)
			{
				filteredItemsList = itemsList;
			}
			else
			{
				filteredItemsList.clear();
				for (size_t i = 0; i < itemsList.size(); i++)
				{
					if (itemsList[i]->getName().find(filter) != -1)
					{
						filteredItemsList.push_back(itemsList[i]);
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::SetCursorPosX(0);
		ImGui::SetCursorPosY(80);
		ImGui::Columns(5, "selectPopupColumns", false);
		for (size_t i = 0; i < filteredItemsList.size(); i++)
		{
			ImGui::PushID(filteredItemsList[i]->getName().c_str());
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (IndexUnderMouse != -1)
				{
					selectedObjects.push_back(FEObjectManager::getInstance().getFEObject(filteredItemsList[IndexUnderMouse]->getObjectID()));
					if (!controlButtonPressed)
					{
						onSelectAction();
						close();
					}
				}
			}

			//IndexSelected == i ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
			isSelected(filteredItemsList[i]) ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
			iconButton->setTexture(PREVIEW_MANAGER.getPreview(filteredItemsList[i]));
			iconButton->render();
			if (iconButton->getWasClicked())
			{
				IndexSelected = int(i);
			}

			if (ImGui::IsItemHovered())
			{
				std::string additionalTypeInfo = "";
				if (filteredItemsList[i]->getType() == FE_TEXTURE)
				{
					additionalTypeInfo += "\nTexture type: ";
					additionalTypeInfo += FETexture::textureInternalFormatToString(RESOURCE_MANAGER.getTexture(filteredItemsList[i]->getObjectID())->getInternalFormat());
				}

				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(("Name: " + filteredItemsList[i]->getName() +
										"\nType: " + FEObjectTypeToString(filteredItemsList[i]->getType()) +
										additionalTypeInfo
										).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			if (iconButton->isHovered())
			{
				IndexUnderMouse = int(i);
			}

			ImGui::Text(filteredItemsList[i]->getName().c_str());
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		selectButton->render();
		if (selectButton->getWasClicked())
		{
			if (IndexSelected != -1)
			{
				selectedObjects.push_back(FEObjectManager::getInstance().getFEObject(filteredItemsList[IndexUnderMouse]->getObjectID()));
				onSelectAction();

				close();
			}
		}

		cancelButton->render();
		if (cancelButton->getWasClicked())
		{
			close();
		}

		ImGui::EndPopup();
	}
}

void selectFEObjectPopUp::onSelectAction()
{
	if (callBack != nullptr)
	{
		callBack(selectedObjects);
		callBack = nullptr;
	}
}

void selectFEObjectPopUp::close()
{
	ImGuiModalPopup::close();
	IndexUnderMouse = -1;
	IndexSelected = -1;
	highlightedObject = nullptr;
	selectedObjects.clear();
}

void selectFEObjectPopUp::keyButtonCallback(int key, int scancode, int action, int mods)
{
	if ((key == GLFW_KEY_LEFT_CONTROL || action == GLFW_KEY_RIGHT_CONTROL) && action == GLFW_RELEASE)
	{
		controlButtonPressed = false;
	}
	else if ((key == GLFW_KEY_LEFT_CONTROL || action == GLFW_KEY_RIGHT_CONTROL) && action == GLFW_PRESS)
	{
		controlButtonPressed = true;
	}
}

bool selectFEObjectPopUp::isSelected(FEObject* object)
{
	for (size_t i = 0; i < selectedObjects.size(); i++)
	{
		if (selectedObjects[i] == object)
			return true;
	}
	
	return false;
}