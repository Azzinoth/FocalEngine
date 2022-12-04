#include "selectPopups.h"

SelectFeObjectPopUp* SelectFeObjectPopUp::Instance = nullptr;
bool SelectFeObjectPopUp::ControlButtonPressed = false;
bool SelectFeObjectPopUp::bOneObjectSelectonMode = true;

SelectFeObjectPopUp::SelectFeObjectPopUp()
{
	PopupCaption = "Select";
	IconButton = new ImGuiImageButton(nullptr);
	IconButton->SetSize(ImVec2(128, 128));
	IconButton->SetUV0(ImVec2(0.0f, 1.0f));
	IconButton->SetUV1(ImVec2(1.0f, 0.0f));
	IconButton->SetFramePadding(8);

	SelectButton = new ImGuiButton("Select");
	SelectButton->SetSize(ImVec2(140, 24));
	SelectButton->SetPosition(ImVec2(500, 35));
	CancelButton = new ImGuiButton("Cancel");
	CancelButton->SetSize(ImVec2(140, 24));
	CancelButton->SetPosition(ImVec2(660, 35));

	ENGINE.AddKeyCallback(SelectFeObjectPopUp::KeyButtonCallback);
}

SelectFeObjectPopUp::~SelectFeObjectPopUp()
{
	delete SelectButton;
	delete CancelButton;
	delete IconButton;
}

void SelectFeObjectPopUp::Show(const FE_OBJECT_TYPE Type, void(*CallBack)(std::vector<FEObject*>), FEObject* HighlightedObject, const std::vector<FEObject*> CustomList)
{
	CurrenType = Type;
	if (CurrenType == FE_NULL)
		return;

	this->HighlightedObject = HighlightedObject;
	this->CallBack = CallBack;
	bShouldOpen = true;

	ItemsList.clear();

	if (CustomList.empty())
	{
		std::vector<std::string> TempList;

		switch (CurrenType)
		{
			case FE_TEXTURE:
			{
				TempList = RESOURCE_MANAGER.GetTextureList();
				TempList.insert(TempList.begin(), RESOURCE_MANAGER.NoTexture->GetObjectID());

				break;
			}

			case FE_MESH:
			{
				TempList = RESOURCE_MANAGER.GetMeshList();

				const std::vector<std::string> StandardMeshList = RESOURCE_MANAGER.GetStandardMeshList();
				for (size_t i = 0; i < StandardMeshList.size(); i++)
				{
					if (EDITOR_INTERNAL_RESOURCES.IsInInternalEditorList(RESOURCE_MANAGER.GetMesh(StandardMeshList[i])))
						continue;

					TempList.push_back(StandardMeshList[i]);
				}

				break;
			}
			
			case FE_MATERIAL:
			{
				TempList = RESOURCE_MANAGER.GetMaterialList();
				TempList.insert(TempList.begin(), "18251A5E0F08013Z3939317U"/*"SolidColorMaterial"*/);

				break;
			}
			
			case FE_GAMEMODEL:
			{
				TempList = RESOURCE_MANAGER.GetGameModelList();
				break;
			}
			
			case FE_PREFAB:
			{
				TempList = RESOURCE_MANAGER.GetPrefabList();
				break;
			}
		}
		
		for (size_t i = 0; i < TempList.size(); i++)
			ItemsList.push_back(OBJECT_MANAGER.GetFEObject(TempList[i]));
	}
	else
	{
		ItemsList = CustomList;
	}

	FilteredItemsList = ItemsList;
	strcpy_s(Filter, "");

	SelectedObjects.clear();
	if (HighlightedObject != nullptr)
	{
		for (size_t i = 0; i < ItemsList.size(); i++)
		{
			if (ItemsList[i]->GetObjectID() == HighlightedObject->GetObjectID())
			{
				AddToSelected(OBJECT_MANAGER.GetFEObject(ItemsList[i]->GetObjectID()));
				break;
			}
		}
	}
}

void SelectFeObjectPopUp::Render()
{
	ImGuiModalPopup::Render();

	ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetWindowPos(ImVec2(ENGINE.GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.GetWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosY(40);
		ImGui::Text("Filter: ");
		ImGui::SameLine();

		ImGui::SetCursorPosY(35);
		if (ImGui::InputText("##selectFEObjectPopUpFilter", Filter, IM_ARRAYSIZE(Filter)))
		{
			if (strlen(Filter) == 0)
			{
				FilteredItemsList = ItemsList;
			}
			else
			{
				FilteredItemsList.clear();
				for (size_t i = 0; i < ItemsList.size(); i++)
				{
					if (ItemsList[i]->GetName().find(Filter) != -1)
					{
						FilteredItemsList.push_back(ItemsList[i]);
					}
				}
			}
		}
		ImGui::Separator();

		ImGui::SetCursorPosX(0);
		ImGui::SetCursorPosY(80);
		ImGui::Columns(5, "selectPopupColumns", false);
		for (size_t i = 0; i < FilteredItemsList.size(); i++)
		{
			ImGui::PushID(FilteredItemsList[i]->GetName().c_str());
			if (ImGui::IsMouseDoubleClicked(0))
			{
				if (IndexUnderMouse != -1)
				{
					AddToSelected(OBJECT_MANAGER.GetFEObject(FilteredItemsList[IndexUnderMouse]->GetObjectID()));
					if (!ControlButtonPressed || bOneObjectSelectonMode && SelectedObjects.size() == 1)
					{
						OnSelectAction();
						Close();
					}
				}
			}

			IsSelected(FilteredItemsList[i]) ? SetSelectedStyle(IconButton) : SetDefaultStyle(IconButton);
			IconButton->SetTexture(PREVIEW_MANAGER.GetPreview(FilteredItemsList[i]));
			IconButton->Render();
			if (IconButton->IsClicked())
			{
				AddToSelected(OBJECT_MANAGER.GetFEObject(FilteredItemsList[i]->GetObjectID()));
			}

			if (ImGui::IsItemHovered())
			{
				std::string AdditionalTypeInfo;
				if (FilteredItemsList[i]->GetType() == FE_TEXTURE)
				{
					AdditionalTypeInfo += "\nTexture type: ";
					AdditionalTypeInfo += FETexture::TextureInternalFormatToString(RESOURCE_MANAGER.GetTexture(FilteredItemsList[i]->GetObjectID())->GetInternalFormat());
				}

				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(("Name: " + FilteredItemsList[i]->GetName() +
										"\nType: " + FEObjectTypeToString(FilteredItemsList[i]->GetType()) +
										AdditionalTypeInfo
										).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}

			if (IconButton->IsHovered())
			{
				IndexUnderMouse = static_cast<int>(i);
			}

			ImGui::Text(FilteredItemsList[i]->GetName().c_str());
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		SelectButton->Render();
		if (SelectButton->IsClicked())
		{
			AddToSelected(OBJECT_MANAGER.GetFEObject(FilteredItemsList[IndexUnderMouse]->GetObjectID()));
			OnSelectAction();

			Close();
		}

		CancelButton->Render();
		if (CancelButton->IsClicked())
		{
			Close();
		}

		ImGui::EndPopup();
	}
}

void SelectFeObjectPopUp::OnSelectAction()
{
	if (CallBack != nullptr)
	{
		CallBack(SelectedObjects);
		CallBack = nullptr;
	}
}

void SelectFeObjectPopUp::Close()
{
	ImGuiModalPopup::Close();
	IndexUnderMouse = -1;
	HighlightedObject = nullptr;
	SelectedObjects.clear();
}

void SelectFeObjectPopUp::KeyButtonCallback(const int Key, int Scancode, const int Action, int Mods)
{
	if ((Key == GLFW_KEY_LEFT_CONTROL || Action == GLFW_KEY_RIGHT_CONTROL) && Action == GLFW_RELEASE)
	{
		ControlButtonPressed = false;
	}
	else if ((Key == GLFW_KEY_LEFT_CONTROL || Action == GLFW_KEY_RIGHT_CONTROL) && Action == GLFW_PRESS)
	{
		ControlButtonPressed = true;
	}
}

bool SelectFeObjectPopUp::IsSelected(const FEObject* Object) const
{
	for (size_t i = 0; i < SelectedObjects.size(); i++)
	{
		if (SelectedObjects[i] == Object)
			return true;
	}
	
	return false;
}

bool SelectFeObjectPopUp::IsOneObjectSelectonMode()
{
	return bOneObjectSelectonMode;
}

void SelectFeObjectPopUp::SetOneObjectSelectonMode(const bool NewValue)
{
	bOneObjectSelectonMode = NewValue;
	if (bOneObjectSelectonMode && SelectedObjects.size() > 1)
		SelectedObjects.resize(1);
}

void SelectFeObjectPopUp::AddToSelected(FEObject* Object)
{
	if (IsSelected(Object))
		return;

	if (bOneObjectSelectonMode && SelectedObjects.size() == 1)
	{
		SelectedObjects[0] = Object;
	}
	else
	{
		SelectedObjects.push_back(Object);
	}
}