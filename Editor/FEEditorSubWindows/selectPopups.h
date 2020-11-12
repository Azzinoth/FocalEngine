#pragma once

#include "../Editor/FEEditorSubWindows/renamePopups.h"

class selectTexturePopUp : public ImGuiModalPopup
{
	FETexture** objToWorkWith;
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::string selectedItemName = "";
	std::vector<std::string> list;
	std::vector<std::string> filteredList;
	char filter[512];

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;

	void(*onSelect)(void*) = nullptr;
	void* ptrOnClose = nullptr;
public:
	selectTexturePopUp()
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

	~selectTexturePopUp()
	{
		if (selectButton != nullptr)
			delete selectButton;

		if (cancelButton != nullptr)
			delete cancelButton;

		if (iconButton != nullptr)
			delete iconButton;
	}

	void show(FETexture** texture, void(*func)(void*) = nullptr, void* ptr = nullptr)
	{
		onSelect = func;
		ptrOnClose = ptr;
		shouldOpen = true;
		objToWorkWith = texture;
		list = RESOURCE_MANAGER.getTextureList();
		list.insert(list.begin(), "noTexture");
		filteredList = list;
		strcpy_s(filter, "");

		if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
		{
			for (size_t i = 0; i < list.size(); i++)
			{
				if (list[i] == (*objToWorkWith)->getName())
				{
					IndexSelected = i;
					selectedItemName = list[i];
					break;
				}
			}
		}

		if ((*objToWorkWith) == nullptr)
		{
			IndexSelected = 0;
			selectedItemName = "noTexture";
		}
	}

	void showWithCustomList(FETexture** texture, std::vector<FETexture*> customList)
	{
		shouldOpen = true;
		objToWorkWith = texture;

		list.clear();
		for (size_t i = 0; i < customList.size(); i++)
		{
			if (customList[i] != nullptr)
				list.push_back(customList[i]->getName());
		}
		list.insert(list.begin(), "noTexture");
		filteredList = list;
		strcpy_s(filter, "");

		if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
		{
			for (size_t i = 0; i < list.size(); i++)
			{
				if (list[i] == (*objToWorkWith)->getName())
				{
					IndexSelected = i;
					selectedItemName = list[i];
					break;
				}
			}
		}

		if ((*objToWorkWith) == nullptr)
		{
			IndexSelected = 0;
			selectedItemName = "noTexture";
		}
	}

	void close() override
	{
		ImGuiModalPopup::close();
		IndexUnderMouse = -1;
		IndexSelected = -1;
		selectedItemName = "";
	}

	void onSelectAction()
	{
		if (onSelect != nullptr && ptrOnClose != nullptr)
		{
			onSelect(ptrOnClose);
			onSelect = nullptr;
			ptrOnClose = nullptr;
		}
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetCursorPosY(40);
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			ImGui::SetCursorPosY(35);
			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredList = list;
				}
				else
				{
					filteredList.clear();
					for (size_t i = 0; i < list.size(); i++)
					{
						if (list[i].find(filter) != -1)
						{
							filteredList.push_back(list[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(80);
			ImGui::Columns(5, "selectTexturePopupColumns", false);
			for (size_t i = 0; i < filteredList.size(); i++)
			{
				ImGui::PushID(filteredList[i].c_str());
				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1)
					{
						if (filteredList[IndexUnderMouse] == "noTexture")
						{
							*objToWorkWith = nullptr;
						}
						else
						{
							*objToWorkWith = RESOURCE_MANAGER.getTexture(filteredList[IndexUnderMouse]);
						}

						PROJECT_MANAGER.getCurrent()->modified = true;
						onSelectAction();
						close();
					}
				}

				selectedItemName == filteredList[i] ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
				filteredList[i] == "noTexture" ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(RESOURCE_MANAGER.getTexture(filteredList[i]));
				iconButton->render();
				if (iconButton->getWasClicked())
				{
					IndexSelected = i;
					selectedItemName = filteredList[i];
				}

				if (iconButton->isHovered())
					IndexUnderMouse = i;

				ImGui::Text(filteredList[i].c_str());
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			selectButton->render();
			if (selectButton->getWasClicked())
			{
				if (IndexSelected != -1)
				{
					if (filteredList[IndexSelected] == "noTexture")
					{
						*objToWorkWith = nullptr;
					}
					else
					{
						*objToWorkWith = RESOURCE_MANAGER.getTexture(filteredList[IndexSelected]);
					}

					PROJECT_MANAGER.getCurrent()->modified = true;
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
};
static selectTexturePopUp selectTextureWindow;

class selectMeshPopUp : public ImGuiModalPopup
{
	FEMesh** objToWorkWith;
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::string selectedItemName = "";
	std::vector<std::string> list;
	std::vector<std::string> filteredList;
	char filter[512];

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;
public:
	selectMeshPopUp()
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

	~selectMeshPopUp()
	{
		if (selectButton != nullptr)
			delete selectButton;

		if (cancelButton != nullptr)
			delete cancelButton;

		if (iconButton != nullptr)
			delete iconButton;
	}

	void show(FEMesh** mesh)
	{
		shouldOpen = true;
		objToWorkWith = mesh;
		list = RESOURCE_MANAGER.getMeshList();
		std::vector<std::string> standardMeshList = RESOURCE_MANAGER.getStandardMeshList();
		for (size_t i = 0; i < standardMeshList.size(); i++)
		{
			if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(RESOURCE_MANAGER.getMesh( standardMeshList[i])))
				continue;
			
			list.insert(list.begin(), standardMeshList[i]);
		}

		filteredList = list;
		strcpy_s(filter, "");

		if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
		{
			for (size_t i = 0; i < list.size(); i++)
			{
				if (list[i] == (*objToWorkWith)->getName())
				{
					IndexSelected = i;
					selectedItemName = list[i];
					break;
				}
			}
		}
	}

	void close() override
	{
		ImGuiModalPopup::close();
		IndexUnderMouse = -1;
		IndexSelected = -1;
		selectedItemName = "";
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetCursorPosY(40);
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			ImGui::SetCursorPosY(35);
			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredList = list;
				}
				else
				{
					filteredList.clear();
					for (size_t i = 0; i < list.size(); i++)
					{
						if (list[i].find(filter) != -1)
						{
							filteredList.push_back(list[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(80);
			ImGui::Columns(5, "selectMeshPopupColumns", false);
			for (size_t i = 0; i < filteredList.size(); i++)
			{
				ImGui::PushID(filteredList[i].c_str());

				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1)
					{
						*objToWorkWith = RESOURCE_MANAGER.getMesh(filteredList[IndexUnderMouse]);
						PROJECT_MANAGER.getCurrent()->modified = true;
						close();
					}
				}

				selectedItemName == filteredList[i] ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
				iconButton->setTexture(PREVIEW_MANAGER.getMeshPreview(filteredList[i]));
				iconButton->render();
				if (iconButton->getWasClicked())
				{
					IndexSelected = i;
					selectedItemName = filteredList[i];
				}

				if (iconButton->isHovered())
					IndexUnderMouse = i;

				ImGui::Text(filteredList[i].c_str());
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			selectButton->render();
			if (selectButton->getWasClicked())
			{
				if (IndexSelected != -1)
				{
					*objToWorkWith = RESOURCE_MANAGER.getMesh(filteredList[IndexSelected]);
					PROJECT_MANAGER.getCurrent()->modified = true;
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
};
static selectMeshPopUp selectMeshWindow;

class selectMaterialPopUp : public ImGuiModalPopup
{
	FEMaterial** objToWorkWith;
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::string selectedItemName = "";
	std::vector<std::string> list;
	std::vector<std::string> filteredList;
	char filter[512];

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;
public:
	selectMaterialPopUp()
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

	~selectMaterialPopUp()
	{
		if (selectButton != nullptr)
			delete selectButton;

		if (cancelButton != nullptr)
			delete cancelButton;

		if (iconButton != nullptr)
			delete iconButton;
	}

	void show(FEMaterial** material)
	{
		shouldOpen = true;
		objToWorkWith = material;
		list = RESOURCE_MANAGER.getMaterialList();
		list.insert(list.begin(), "SolidColorMaterial");

		filteredList = list;
		strcpy_s(filter, "");

		if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
		{
			for (size_t i = 0; i < list.size(); i++)
			{
				if (list[i] == (*objToWorkWith)->getName())
				{
					IndexSelected = i;
					selectedItemName = list[i];
					break;
				}
			}
		}
	}

	void close() override
	{
		ImGuiModalPopup::close();
		IndexUnderMouse = -1;
		IndexSelected = -1;
		selectedItemName = "";
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetCursorPosY(40);
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			ImGui::SetCursorPosY(35);
			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredList = list;
				}
				else
				{
					filteredList.clear();
					for (size_t i = 0; i < list.size(); i++)
					{
						if (list[i].find(filter) != -1)
						{
							filteredList.push_back(list[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(80);
			ImGui::Columns(5, "selectMeshPopupColumns", false);
			for (size_t i = 0; i < filteredList.size(); i++)
			{
				ImGui::PushID(filteredList[i].c_str());
				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1)
					{
						*objToWorkWith = RESOURCE_MANAGER.getMaterial(filteredList[IndexUnderMouse]);
						PROJECT_MANAGER.getCurrent()->modified = true;
						close();
					}
				}

				selectedItemName == filteredList[i] ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
				iconButton->setTexture(PREVIEW_MANAGER.getMaterialPreview(filteredList[i]));
				iconButton->render();
				if (iconButton->getWasClicked())
				{
					IndexSelected = i;
					selectedItemName = filteredList[i];
				}

				if (iconButton->isHovered())
				{
					IndexUnderMouse = i;
				}

				ImGui::Text(filteredList[i].c_str());
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			selectButton->render();
			if (selectButton->getWasClicked())
			{
				if (IndexSelected != -1)
				{
					*objToWorkWith = RESOURCE_MANAGER.getMaterial(filteredList[IndexSelected]);
					PROJECT_MANAGER.getCurrent()->modified = true;
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
};
static selectMaterialPopUp selectMaterialWindow;

class selectGameModelPopUp : public ImGuiModalPopup
{
	FEGameModel** objToWorkWith;
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::string selectedItemName = "";
	std::vector<std::string> list;
	std::vector<std::string> filteredList;
	char filter[512];
	bool newEntityFlag = false;
	bool wasSelectedAlready = false;

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;
public:
	selectGameModelPopUp()
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

	~selectGameModelPopUp()
	{
		if (selectButton != nullptr)
			delete selectButton;

		if (cancelButton != nullptr)
			delete cancelButton;

		if (iconButton != nullptr)
			delete iconButton;
	}

	void show(FEGameModel** gameModel, bool newEntityFlag = false)
	{
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

		list = RESOURCE_MANAGER.getGameModelList();
		std::vector<std::string> standardGameModelList = RESOURCE_MANAGER.getStandardGameModelList();
		for (size_t i = 0; i < standardGameModelList.size(); i++)
		{
			if (EDITOR_INTERNAL_RESOURCES.isInInternalEditorList(RESOURCE_MANAGER.getGameModel(standardGameModelList[i])))
				continue;

			list.insert(list.begin(), standardGameModelList[i]);
		}

		filteredList = list;
		strcpy_s(filter, "");

		if (objToWorkWith != nullptr && (*objToWorkWith) != nullptr)
		{
			for (size_t i = 0; i < list.size(); i++)
			{
				if (list[i] == (*objToWorkWith)->getName())
				{
					IndexSelected = i;
					selectedItemName = list[i];
					break;
				}
			}
		}
	}

	void close() override
	{
		ImGuiModalPopup::close();
		IndexUnderMouse = -1;
		IndexSelected = -1;
		selectedItemName = "";
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SetCursorPosY(40);
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			ImGui::SetCursorPosY(35);
			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredList = list;
				}
				else
				{
					filteredList.clear();
					for (size_t i = 0; i < list.size(); i++)
					{
						if (list[i].find(filter) != -1)
						{
							filteredList.push_back(list[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(80);
			ImGui::Columns(5, "selectGameModelPopupColumns", false);
			for (size_t i = 0; i < filteredList.size(); i++)
			{
				ImGui::PushID(filteredList[i].c_str());
				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1 && !wasSelectedAlready)
					{
						if (newEntityFlag)
						{
							FEEntity* newEntity = FEScene::getInstance().addEntity(RESOURCE_MANAGER.getGameModel(filteredList[IndexUnderMouse]));
							newEntity->transform.setPosition(ENGINE.getCamera()->getPosition() + ENGINE.getCamera()->getForward() * 10.0f);
							SELECTED.setEntity(newEntity);

							wasSelectedAlready = true;
							PROJECT_MANAGER.getCurrent()->modified = true;
						}
						else
						{
							*objToWorkWith = RESOURCE_MANAGER.getGameModel(filteredList[IndexUnderMouse]);
						}

						close();
					}
				}

				selectedItemName == filteredList[i] ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
				iconButton->setTexture(PREVIEW_MANAGER.getGameModelPreview(filteredList[i]));
				iconButton->render();
				if (iconButton->getWasClicked())
				{
					IndexSelected = i;
					selectedItemName = filteredList[i];
				}

				if (iconButton->isHovered())
				{
					IndexUnderMouse = i;
				}

				ImGui::Text(filteredList[i].c_str());
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
						FEScene::getInstance().addEntity(RESOURCE_MANAGER.getGameModel(filteredList[IndexSelected]));
					}
					else
					{
						*objToWorkWith = RESOURCE_MANAGER.getGameModel(filteredList[IndexUnderMouse]);
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
};
static selectGameModelPopUp selectGameModelWindow;