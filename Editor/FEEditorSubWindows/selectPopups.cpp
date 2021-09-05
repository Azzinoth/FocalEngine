#include "selectPopups.h"

selectMaterialPopUp* selectMaterialPopUp::_instance = nullptr;

selectMaterialPopUp::selectMaterialPopUp()
{
	popupCaption = "Select material";
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
}

selectMaterialPopUp::~selectMaterialPopUp()
{
	if (selectButton != nullptr)
		delete selectButton;

	if (cancelButton != nullptr)
		delete cancelButton;

	if (iconButton != nullptr)
		delete iconButton;
}

void selectMaterialPopUp::render()
{
	ImGuiModalPopup::render();

	ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
	ImVec2 POSITION = ImGui::GetWindowPos();

	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosY(40);
		ImGui::Text("Filter: ");
		ImGui::SameLine();

		ImGui::SetCursorPosY(35);
		if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
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
					if (itemsList[i].FEObjectPart->getName().find(filter) != -1)
					{
						filteredItemsList.push_back(itemsList[i]);
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::SetCursorPosX(0);
		ImGui::SetCursorPosY(80);
		ImGui::Columns(5, "selectMeshPopupColumns", false);
		for (size_t i = 0; i < filteredItemsList.size(); i++)
		{
			ImGui::PushID(filteredItemsList[i].FEObjectPart->getName().c_str());
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (IndexUnderMouse != -1)
				{
					*objToWorkWith = RESOURCE_MANAGER.getMaterial(filteredItemsList[IndexUnderMouse].FEObjectPart->getObjectID());
					PROJECT_MANAGER.getCurrent()->setModified(true);
					close();
				}
			}

			selectedItemID == filteredItemsList[i].FEObjectPart->getObjectID() ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
			iconButton->setTexture(PREVIEW_MANAGER.getMaterialPreview(filteredItemsList[i].FEObjectPart->getObjectID()));
			iconButton->render();
			if (iconButton->getWasClicked())
			{
				IndexSelected = i;
				selectedItemID = filteredItemsList[i].FEObjectPart->getObjectID();
			}

			if (iconButton->isHovered())
			{
				IndexUnderMouse = i;
			}

			ImGui::Text(filteredItemsList[i].FEObjectPart->getName().c_str());
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		selectButton->render();
		if (selectButton->getWasClicked())
		{
			if (IndexSelected != -1)
			{
				*objToWorkWith = RESOURCE_MANAGER.getMaterial(filteredItemsList[IndexSelected].FEObjectPart->getObjectID());
				PROJECT_MANAGER.getCurrent()->setModified(true);
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

void selectMaterialPopUp::setAllowedShader(FEShader* shader)
{
	allowedShader = shader;
}

void selectMaterialPopUp::show(FEMaterial** material)
{
	shouldOpen = true;
	objToWorkWith = material;

	itemsList.clear();
	itemsList.insert(itemsList.begin(), RESOURCE_MANAGER.getMaterial("18251A5E0F08013Z3939317U"));

	std::vector<std::string> tempList = RESOURCE_MANAGER.getMaterialList();
	for (size_t i = 0; i < tempList.size(); i++)
	{
		itemsList.push_back(contenetBrowserItem<FEMaterial>(RESOURCE_MANAGER.getMaterial(tempList[i])));
	}

	if (allowedShader != nullptr)
	{
		for (int i = 0; i < int(itemsList.size()); i++)
		{
			if (RESOURCE_MANAGER.getMaterial(itemsList[i].FEObjectPart->getObjectID())->shader->getObjectID() != allowedShader->getObjectID())
			{
				itemsList.erase(itemsList.begin() + i);
				i--;
			}
		}
		allowedShader = nullptr;
	}

	filteredItemsList = itemsList;
	strcpy_s(filter, "");

	if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
	{
		for (size_t i = 0; i < itemsList.size(); i++)
		{
			if (itemsList[i].FEObjectPart->getObjectID() == (*objToWorkWith)->getObjectID())
			{
				IndexSelected = i;
				selectedItemID = itemsList[i].FEObjectPart->getObjectID();
				break;
			}
		}
	}
}

void selectMaterialPopUp::close()
{
	ImGuiModalPopup::close();
	IndexUnderMouse = -1;
	IndexSelected = -1;
	selectedItemID = "";
}

selectMeshPopUp* selectMeshPopUp::_instance = nullptr;

selectMeshPopUp::selectMeshPopUp()
{
	popupCaption = "Select mesh";
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
}

selectMeshPopUp::~selectMeshPopUp()
{
	if (selectButton != nullptr)
		delete selectButton;

	if (cancelButton != nullptr)
		delete cancelButton;

	if (iconButton != nullptr)
		delete iconButton;
}

void selectMeshPopUp::show(FEMesh** mesh)
{
	shouldOpen = true;
	objToWorkWith = mesh;

	std::vector<std::string> tempList = RESOURCE_MANAGER.getMeshList();
	itemsList.clear();

	for (size_t i = 0; i < tempList.size(); i++)
	{
		itemsList.push_back(contenetBrowserItem<FEMesh>(RESOURCE_MANAGER.getMesh(tempList[i])));
	}

	std::vector<std::string> standardMeshList = RESOURCE_MANAGER.getStandardMeshList();
	for (size_t i = 0; i < standardMeshList.size(); i++)
	{
		if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(RESOURCE_MANAGER.getMesh(standardMeshList[i])))
			continue;

		itemsList.push_back(contenetBrowserItem<FEMesh>(RESOURCE_MANAGER.getMesh(standardMeshList[i])));
	}

	filteredItemsList = itemsList;
	strcpy_s(filter, "");

	if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
	{
		for (size_t i = 0; i < itemsList.size(); i++)
		{
			if (itemsList[i].FEObjectPart->getObjectID() == (*objToWorkWith)->getObjectID())
			{
				IndexSelected = i;
				selectedItemID = itemsList[i].FEObjectPart->getObjectID();
				break;
			}
		}
	}
}

void selectMeshPopUp::close()
{
	ImGuiModalPopup::close();
	IndexUnderMouse = -1;
	IndexSelected = -1;
	selectedItemID = "";
}

void selectMeshPopUp::render()
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
		if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
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
					if (itemsList[i].FEObjectPart->getName().find(filter) != -1)
					{
						filteredItemsList.push_back(itemsList[i]);
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::SetCursorPosX(0);
		ImGui::SetCursorPosY(80);
		ImGui::Columns(5, "selectMeshPopupColumns", false);
		for (size_t i = 0; i < filteredItemsList.size(); i++)
		{
			ImGui::PushID(filteredItemsList[i].FEObjectPart->getName().c_str());

			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (IndexUnderMouse != -1)
				{
					*objToWorkWith = RESOURCE_MANAGER.getMesh(filteredItemsList[IndexUnderMouse].FEObjectPart->getObjectID());

					PROJECT_MANAGER.getCurrent()->setModified(true);
					close();
				}
			}

			iconButton->setTexture(PREVIEW_MANAGER.getMeshPreview(filteredItemsList[i].FEObjectPart->getObjectID()));
			selectedItemID == filteredItemsList[i].FEObjectPart->getObjectID() ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);

			iconButton->render();
			if (iconButton->getWasClicked())
			{
				IndexSelected = i;
				selectedItemID = filteredItemsList[i].FEObjectPart->getObjectID();
			}

			if (iconButton->isHovered())
				IndexUnderMouse = i;

			ImGui::Text(filteredItemsList[i].FEObjectPart->getName().c_str());
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		selectButton->render();
		if (selectButton->getWasClicked())
		{
			if (IndexSelected != -1)
			{
				*objToWorkWith = RESOURCE_MANAGER.getMesh(filteredItemsList[IndexSelected].FEObjectPart->getObjectID());

				PROJECT_MANAGER.getCurrent()->setModified(true);
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

selectTexturePopUp* selectTexturePopUp::_instance = nullptr;

selectTexturePopUp::selectTexturePopUp()
{
	popupCaption = "Select texture";
	iconButton = new ImGuiImageButton(nullptr);
	iconButton->setSize(ImVec2(128, 128));
	iconButton->setUV0(ImVec2(0.0f, 0.0f));
	iconButton->setUV1(ImVec2(1.0f, 1.0f));
	iconButton->setFramePadding(8);

	selectButton = new ImGuiButton("Select");
	selectButton->setSize(ImVec2(140, 24));
	selectButton->setPosition(ImVec2(500, 35));
	cancelButton = new ImGuiButton("Cancel");
	cancelButton->setSize(ImVec2(140, 24));
	cancelButton->setPosition(ImVec2(660, 35));
}

selectTexturePopUp::~selectTexturePopUp()
{
	if (selectButton != nullptr)
		delete selectButton;

	if (cancelButton != nullptr)
		delete cancelButton;

	if (iconButton != nullptr)
		delete iconButton;
}

void selectTexturePopUp::show(FETexture** texture, void(*func)(void*), void* ptr)
{
	onSelect = func;
	ptrOnClose = ptr;
	shouldOpen = true;
	objToWorkWith = texture;

	std::vector<std::string> tempList = RESOURCE_MANAGER.getTextureList();
	itemsList.clear();
	itemsList.insert(itemsList.begin(), RESOURCE_MANAGER.noTexture);

	for (size_t i = 0; i < tempList.size(); i++)
	{
		itemsList.push_back(contenetBrowserItem<FETexture>(RESOURCE_MANAGER.getTexture(tempList[i])));
	}

	filteredItemsList = itemsList;
	strcpy_s(filter, "");

	if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
	{
		for (size_t i = 0; i < itemsList.size(); i++)
		{
			if (itemsList[i].FEObjectPart->getObjectID() == (*objToWorkWith)->getObjectID())
			{
				IndexSelected = i;
				selectedItemID = itemsList[i].FEObjectPart->getObjectID();
				break;
			}
		}
	}

	if ((*objToWorkWith) == nullptr)
	{
		IndexSelected = 0;
		selectedItemID = RESOURCE_MANAGER.noTexture->getObjectID();
	}
}

void selectTexturePopUp::showWithCustomList(FETexture** texture, std::vector<FETexture*> customList)
{
	shouldOpen = true;
	objToWorkWith = texture;

	itemsList.clear();
	itemsList.insert(itemsList.begin(), RESOURCE_MANAGER.noTexture);

	for (size_t i = 0; i < customList.size(); i++)
	{
		if (customList[i] != nullptr)
			itemsList.push_back(customList[i]);
	}

	filteredItemsList = itemsList;
	strcpy_s(filter, "");

	if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
	{
		for (size_t i = 0; i < itemsList.size(); i++)
		{
			if (itemsList[i].FEObjectPart->getObjectID() == (*objToWorkWith)->getObjectID())
			{
				IndexSelected = i;
				selectedItemID = itemsList[i].FEObjectPart->getObjectID();
				break;
			}
		}
	}

	if ((*objToWorkWith) == nullptr)
	{
		IndexSelected = 0;
		selectedItemID = RESOURCE_MANAGER.noTexture->getObjectID();
	}
}

void selectTexturePopUp::close()
{
	ImGuiModalPopup::close();
	IndexUnderMouse = -1;
	IndexSelected = -1;
	selectedItemID = "";
}

void selectTexturePopUp::onSelectAction()
{
	if (onSelect != nullptr && ptrOnClose != nullptr)
	{
		onSelect(ptrOnClose);
		onSelect = nullptr;
		ptrOnClose = nullptr;
	}
}

void selectTexturePopUp::render()
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
		if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
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
					if (itemsList[i].FEObjectPart->getName().find(filter) != -1)
					{
						filteredItemsList.push_back(itemsList[i]);
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::SetCursorPosX(0);
		ImGui::SetCursorPosY(80);
		ImGui::Columns(5, "selectTexturePopupColumns", false);
		for (size_t i = 0; i < filteredItemsList.size(); i++)
		{
			ImGui::PushID(filteredItemsList[i].FEObjectPart->getName().c_str());
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (IndexUnderMouse != -1)
				{
					if (filteredItemsList[IndexUnderMouse].FEObjectPart->getName() == "noTexture")
					{
						*objToWorkWith = nullptr;
					}
					else
					{
						*objToWorkWith = RESOURCE_MANAGER.getTexture(filteredItemsList[IndexUnderMouse].FEObjectPart->getObjectID());
					}

					PROJECT_MANAGER.getCurrent()->setModified(true);
					onSelectAction();
					close();
				}
			}

			selectedItemID == filteredItemsList[i].FEObjectPart->getObjectID() ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
			filteredItemsList[i].FEObjectPart->getName() == "noTexture" ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(RESOURCE_MANAGER.getTexture(filteredItemsList[i].FEObjectPart->getObjectID()));
			iconButton->render();
			if (iconButton->getWasClicked())
			{
				IndexSelected = i;
				selectedItemID = filteredItemsList[i].FEObjectPart->getObjectID();
			}

			if (iconButton->isHovered())
				IndexUnderMouse = i;

			ImGui::Text(filteredItemsList[i].FEObjectPart->getName().c_str());
			ImGui::PopID();
			ImGui::NextColumn();

		}
		ImGui::Columns(1);

		selectButton->render();
		if (selectButton->getWasClicked())
		{
			if (IndexSelected != -1)
			{
				if (filteredItemsList[IndexSelected].FEObjectPart->getName() == "noTexture")
				{
					*objToWorkWith = nullptr;
				}
				else
				{
					*objToWorkWith = RESOURCE_MANAGER.getTexture(filteredItemsList[IndexSelected].FEObjectPart->getObjectID());
				}

				PROJECT_MANAGER.getCurrent()->setModified(true);
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

selectGameModelPopUp* selectGameModelPopUp::_instance = nullptr;

selectGameModelPopUp::selectGameModelPopUp()
{
	popupCaption = "Select game model";
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
}

selectGameModelPopUp::~selectGameModelPopUp()
{
	if (selectButton != nullptr)
		delete selectButton;

	if (cancelButton != nullptr)
		delete cancelButton;

	if (iconButton != nullptr)
		delete iconButton;
}

void selectGameModelPopUp::show(FEGameModel** gameModel, bool newEntityFlag, bool isInstanced)
{
	this->isInstanced = isInstanced;
	wasSelectedAlready = false;
	shouldOpen = true;
	objToWorkWith = gameModel;
	this->newEntityFlag = newEntityFlag;
	if (newEntityFlag)
	{
		popupCaption = "Select game model to create new Entity";
	}
	else
	{
		popupCaption = "Select game model";
	}

	std::vector<std::string> tempList = RESOURCE_MANAGER.getGameModelList();
	itemsList.clear();
	for (size_t i = 0; i < tempList.size(); i++)
	{
		itemsList.push_back(contenetBrowserItem<FEGameModel>(RESOURCE_MANAGER.getGameModel(tempList[i])));
	}

	filteredItemsList = itemsList;
	strcpy_s(filter, "");

	if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
	{
		for (size_t i = 0; i < itemsList.size(); i++)
		{
			if (itemsList[i].FEObjectPart->getObjectID() == (*objToWorkWith)->getObjectID())
			{
				IndexSelected = i;
				selectedItemID = itemsList[i].FEObjectPart->getObjectID();
				break;
			}
		}
	}
}

void selectGameModelPopUp::close()
{
	ImGuiModalPopup::close();
	IndexUnderMouse = -1;
	IndexSelected = -1;
	selectedItemID = "";
}

void selectGameModelPopUp::render()
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
		if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
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
					if (itemsList[i].FEObjectPart->getName().find(filter) != -1)
					{
						filteredItemsList.push_back(itemsList[i]);
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::SetCursorPosX(0);
		ImGui::SetCursorPosY(80);
		ImGui::Columns(5, "selectGameModelPopupColumns", false);
		for (size_t i = 0; i < filteredItemsList.size(); i++)
		{
			ImGui::PushID(filteredItemsList[i].FEObjectPart->getName().c_str());
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (IndexUnderMouse != -1 && !wasSelectedAlready)
				{
					if (newEntityFlag)
					{
						if (isInstanced)
						{
							FEEntityInstanced* newEntity = FEScene::getInstance().addEntityInstanced(RESOURCE_MANAGER.getGameModel(filteredItemsList[IndexUnderMouse].FEObjectPart->getObjectID()));
							newEntity->transform.setPosition(ENGINE.getCamera()->getPosition() + ENGINE.getCamera()->getForward() * 10.0f);
							SELECTED.setSelected(newEntity);

							wasSelectedAlready = true;
							PROJECT_MANAGER.getCurrent()->setModified(true);
						}
						else
						{
							FEEntity* newEntity = FEScene::getInstance().addEntity(RESOURCE_MANAGER.getGameModel(filteredItemsList[IndexUnderMouse].FEObjectPart->getObjectID()));
							newEntity->transform.setPosition(ENGINE.getCamera()->getPosition() + ENGINE.getCamera()->getForward() * 10.0f);
							SELECTED.setSelected(newEntity);

							wasSelectedAlready = true;
							PROJECT_MANAGER.getCurrent()->setModified(true);
						}
					}
					else
					{
						*objToWorkWith = RESOURCE_MANAGER.getGameModel(filteredItemsList[IndexUnderMouse].FEObjectPart->getObjectID());
					}

					close();
				}
			}

			selectedItemID == filteredItemsList[i].FEObjectPart->getObjectID() ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
			iconButton->setTexture(PREVIEW_MANAGER.getGameModelPreview(filteredItemsList[i].FEObjectPart->getObjectID()));
			iconButton->render();
			if (iconButton->getWasClicked())
			{
				IndexSelected = i;
				selectedItemID = filteredItemsList[i].FEObjectPart->getObjectID();
			}

			if (iconButton->isHovered())
			{
				IndexUnderMouse = i;
			}

			ImGui::Text(filteredItemsList[i].FEObjectPart->getName().c_str());
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		selectButton->render();
		if (selectButton->getWasClicked())
		{
			if (IndexSelected != -1)
			{
				if (newEntityFlag)
				{
					FEScene::getInstance().addEntity(RESOURCE_MANAGER.getGameModel(filteredItemsList[IndexSelected].FEObjectPart->getObjectID()));
				}
				else
				{
					*objToWorkWith = RESOURCE_MANAGER.getGameModel(filteredItemsList[IndexUnderMouse].FEObjectPart->getObjectID());
				}
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