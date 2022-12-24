#include "../Editor/FEEditor.h"

static FETexture* TempTexture = nullptr;
static void AddTransparencyToTextureCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (TempTexture == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_TEXTURE)
	{
		FETexture* OriginalTexture = TempTexture;

		FETexture* NewTexture = RESOURCE_MANAGER.CreateTextureWithTransparency(OriginalTexture, reinterpret_cast<FETexture*>(SelectionsResult[0]));
		if (NewTexture == nullptr)
		{
			TempTexture = nullptr;
			return;
		}

		unsigned char* NewRawData = NewTexture->GetRawData();
		const int MaxDimention = std::max(OriginalTexture->GetWidth(), OriginalTexture->GetHeight());
		const size_t MipCount = size_t(floor(log2(MaxDimention)) + 1);
		OriginalTexture->UpdateRawData(NewRawData, MipCount);
		FE_GL_ERROR(glGenerateMipmap(GL_TEXTURE_2D));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f));
		FE_GL_ERROR(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0.0f));
		PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(OriginalTexture);

		delete[] NewRawData;
		RESOURCE_MANAGER.DeleteFETexture(NewTexture);
	}

	TempTexture = nullptr;
}

static void CreateNewPrefabCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (!SelectionsResult.empty() && SelectionsResult[0]->GetType() == FE_GAMEMODEL)
	{
		FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab(reinterpret_cast<FEGameModel*>(SelectionsResult[0]));

		if (SelectionsResult.size() > 1)
		{
			for (int i = 1; i < SelectionsResult.size(); i++)
			{
				if (SelectionsResult[i]->GetType() == FE_GAMEMODEL)
					NewPrefab->AddComponent(reinterpret_cast<FEGameModel*>(SelectionsResult[i]));
			}
		}
		
		PROJECT_MANAGER.GetCurrent()->SetModified(true);
		VIRTUAL_FILE_SYSTEM.CreateFile(NewPrefab, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
	}

	SelectFeObjectPopUp::getInstance().SetOneObjectSelectonMode(true);
}

void FEEditor::DisplayContentBrowser()
{
	if (!bContentBrowserVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_None);

	DisplayContentBrowserItems();

	bool open_context_menu = false;
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
		open_context_menu = true;

	if (open_context_menu)
		ImGui::OpenPopup("##context_menu");

	bShouldOpenContextMenuInContentBrowser = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		bShouldOpenContextMenuInContentBrowser = true;

		if (ContentBrowserItemUnderMouse == -1)
		{
			if (ImGui::MenuItem("Import Asset..."))
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileOpenDialog(FilePath, ALL_IMPORT_LOAD_FILTER, 3);
				if (!FilePath.empty())
				{
					const std::vector<FEObject*> LoadedObjects = RESOURCE_MANAGER.ImportAsset(FilePath.c_str());
					for (size_t i = 0; i < LoadedObjects.size(); i++)
					{
						if (LoadedObjects[i] != nullptr)
						{
							VIRTUAL_FILE_SYSTEM.CreateFile(LoadedObjects[i], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
							PROJECT_MANAGER.GetCurrent()->SetModified(true);
							PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(LoadedObjects[i]);
						}
					}
				}
			}

			if (ImGui::MenuItem("*DEPRECATED*Import Tree..."))
			{
				std::string FilePath;
				FILE_SYSTEM.ShowFileOpenDialog(FilePath, OBJ_LOAD_FILTER, 1);
				if (!FilePath.empty())
				{
					const std::vector<FEObject*> LoadedObjects = RESOURCE_MANAGER.ImportOBJ(FilePath.c_str(), true);
					for (size_t i = 0; i < LoadedObjects.size(); i++)
					{
						if (LoadedObjects[i] != nullptr)
						{
							VIRTUAL_FILE_SYSTEM.CreateFile(LoadedObjects[i], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
							PROJECT_MANAGER.GetCurrent()->SetModified(true);
							PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(LoadedObjects[i]);
						}
					}
				}
			}

			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::MenuItem("Add folder"))
				{
					const std::string NewDirectoryName = VIRTUAL_FILE_SYSTEM.CreateDirectory(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
					UpdateDirectoryDragAndDropTargets();

					for (size_t i = 0; i < FilteredResourcesContentBrowser.size(); i++)
					{
						if (FilteredResourcesContentBrowser[i]->GetName() == NewDirectoryName)
						{
							ContentBrowserRenameIndex = int(i);
							strcpy_s(ContentBrowserRename, FilteredResourcesContentBrowser[i]->GetName().size() + 1, FilteredResourcesContentBrowser[i]->GetName().c_str());
							bLastFrameRenameEditWasVisiable = false;
							break;
						}
					}
				}

				if (ImGui::BeginMenu("Texture"))
				{
					if (ImGui::MenuItem("Combine channels..."))
						CombineChannelsToTexturePopUp::getInstance().Show();

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Create new material"))
				{
					FEMaterial* NewMat = RESOURCE_MANAGER.CreateMaterial("");
					if (NewMat)
					{
						PROJECT_MANAGER.GetCurrent()->SetModified(true);
						NewMat->Shader = RESOURCE_MANAGER.GetShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

						NewMat->SetAlbedoMap(RESOURCE_MANAGER.NoTexture);

						VIRTUAL_FILE_SYSTEM.CreateFile(NewMat, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
					}
				}

				if (ImGui::MenuItem("Create new game model"))
				{
					FEGameModel* NewGameModel = RESOURCE_MANAGER.CreateGameModel();
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					VIRTUAL_FILE_SYSTEM.CreateFile(NewGameModel, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
				}

				if (ImGui::MenuItem("Create new prefab"))
				{
					SelectFeObjectPopUp::getInstance().SetOneObjectSelectonMode(false);
					SelectFeObjectPopUp::getInstance().Show(FE_GAMEMODEL, CreateNewPrefabCallBack);
				}

				ImGui::EndMenu();
			}
		}
		else
		{
			std::string FullPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (FullPath.back() != '/')
				FullPath += '/';
			FullPath += FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetName();

			const bool ReadOnlyItem = VIRTUAL_FILE_SYSTEM.IsReadOnly(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse], FullPath);

			if (ReadOnlyItem)
				ImGui::MenuItem("Read Only");
			
			if (!ReadOnlyItem)
			{
				if (ImGui::MenuItem("Rename"))
				{
					ContentBrowserRenameIndex = ContentBrowserItemUnderMouse;

					strcpy_s(ContentBrowserRename, FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetName().size() + 1, FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetName().c_str());
					bLastFrameRenameEditWasVisiable = false;
				}
			}

			if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_MATERIAL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					EditMaterialPopup::getInstance().Show(RESOURCE_MANAGER.GetMaterial(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
				}
			}

			if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_GAMEMODEL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					EditGameModelPopup::getInstance().Show(RESOURCE_MANAGER.GetGameModel(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
				}

				if (ImGui::MenuItem("Create Prefab out of this Game Model"))
				{
					FEPrefab* NewPrefab = RESOURCE_MANAGER.CreatePrefab(RESOURCE_MANAGER.GetGameModel(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					VIRTUAL_FILE_SYSTEM.CreateFile(NewPrefab, VIRTUAL_FILE_SYSTEM.GetCurrentPath());
				}
			}

			if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_PREFAB)
			{
				if (ImGui::MenuItem("Edit"))
				{
					PrefabEditorWindow::getInstance().Show(RESOURCE_MANAGER.GetPrefab(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
				}
			}

			if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_SHADER)
			{
				if (ImGui::MenuItem("Edit"))
				{
					shaderEditorWindow::getInstance().Show(RESOURCE_MANAGER.GetShader(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
				}
			}

			if (!ReadOnlyItem)
			{
				if (ImGui::MenuItem("Delete"))
				{
					if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_NULL)
					{
						DeleteDirectoryPopup::getInstance().Show(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetName());
					}
					else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_MESH)
					{
						DeleteMeshPopup::getInstance().Show(RESOURCE_MANAGER.GetMesh(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
					}
					else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_TEXTURE)
					{
						DeleteTexturePopup::getInstance().Show(RESOURCE_MANAGER.GetTexture(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
					}
					else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_MATERIAL)
					{
						DeleteMaterialPopup::getInstance().Show(RESOURCE_MANAGER.GetMaterial(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
					}
					else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_GAMEMODEL)
					{
						DeleteGameModelPopup::getInstance().Show(RESOURCE_MANAGER.GetGameModel(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
					}
					else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_PREFAB)
					{
						DeletePrefabPopup::getInstance().Show(RESOURCE_MANAGER.GetPrefab(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
					}
				}
			}

			if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_TEXTURE)
			{
				if (ImGui::BeginMenu("Convert"))
				{
					if (ImGui::MenuItem("Texture channels to individual textures"))
					{
						const std::vector<FETexture*> NewTextures = RESOURCE_MANAGER.ChannelsToFETextures(RESOURCE_MANAGER.GetTexture(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));

						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTextures[0]);
						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTextures[1]);
						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTextures[2]);
						PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(NewTextures[3]);

						PROJECT_MANAGER.GetCurrent()->SetModified(true);

						VIRTUAL_FILE_SYSTEM.CreateFile(NewTextures[0], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
						VIRTUAL_FILE_SYSTEM.CreateFile(NewTextures[1], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
						VIRTUAL_FILE_SYSTEM.CreateFile(NewTextures[2], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
						VIRTUAL_FILE_SYSTEM.CreateFile(NewTextures[3], VIRTUAL_FILE_SYSTEM.GetCurrentPath());
					}

					if (ImGui::MenuItem("Resize"))
					{
						FETexture* TextureToResize = RESOURCE_MANAGER.GetTexture(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID());
						resizeTexturePopup::getInstance().Show(TextureToResize);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Add transparency"))
				{
					if (ImGui::MenuItem("Choose transparency mask"))
					{
						TempTexture = reinterpret_cast<FETexture*>(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]);
						SelectFeObjectPopUp::getInstance().Show(FE_TEXTURE, AddTransparencyToTextureCallBack);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Export"))
				{
					FETexture* TextureToExport = RESOURCE_MANAGER.GetTexture(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID());

					if (ImGui::MenuItem("as PNG"))
					{
						std::string FilePath;
						FILE_SYSTEM.ShowFileSaveDialog(FilePath, TEXTURE_LOAD_FILTER, 1);

						if (!FilePath.empty())
						{
							FilePath += ".png";
							RESOURCE_MANAGER.ExportFETextureToPNG(TextureToExport, FilePath.c_str());
						}
					}

					ImGui::EndMenu();
				}
			}
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();
}

void FEEditor::ChooseTexturesForContentBrowserItem(FETexture*& PreviewTexture, FETexture*& SmallAdditionTypeIcon, ImVec2& UV0, ImVec2& UV1, FEObject* Item)
{
	if (Item->GetType() == FE_NULL)
	{
		UV0 = ImVec2(0.0f, 0.0f);
		UV1 = ImVec2(1.0f, 1.0f);

		PreviewTexture = FolderIcon;

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
	}
	else if (Item->GetType() == FE_SHADER)
	{
		UV0 = ImVec2(0.0f, 0.0f);
		UV1 = ImVec2(1.0f, 1.0f);

		PreviewTexture = ShaderIcon;

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
	}
	else if (Item->GetType() == FE_MESH)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetMeshPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = MeshContentBrowserIcon;
	}
	else if (Item->GetType() == FE_TEXTURE)
	{
		PreviewTexture = RESOURCE_MANAGER.GetTexture(Item->GetObjectID());
		SmallAdditionTypeIcon = TextureContentBrowserIcon;
	}
	else if (Item->GetType() == FE_MATERIAL)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetMaterialPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = MaterialContentBrowserIcon;
	}
	else if (Item->GetType() == FE_GAMEMODEL)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetGameModelPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = GameModelContentBrowserIcon;
	}
	else if (Item->GetType() == FE_PREFAB)
	{
		UV0 = ImVec2(0.0f, 1.0f);
		UV1 = ImVec2(1.0f, 0.0f);

		PreviewTexture = PREVIEW_MANAGER.GetPrefabPreview(Item->GetObjectID());
		SmallAdditionTypeIcon = PrefabContentBrowserIcon;
	}
}

void FEEditor::DisplayContentBrowserItems()
{
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	float CurrentY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(CurrentY);
	if (ImGui::ImageButton((void*)(intptr_t)VFSBackIcon->GetTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		VIRTUAL_FILE_SYSTEM.SetCurrentPath(VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));
		strcpy_s(FilterForResourcesContentBrowser, "");
	}
	VFSBackButtonTarget->StickToItem();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	static std::string LastFramePath;
	AllResourcesContentBrowser.clear();
	AllResourcesContentBrowser = VIRTUAL_FILE_SYSTEM.GetDirectoryContent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());

	ImGui::SetCursorPosX(100);
	ImGui::SetCursorPosY(CurrentY + 5);
	ImGui::Text("Filter by type: ");

	ImGui::SetCursorPosX(120 + 140);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResourcesContentBrowser.empty() ? SetSelectedStyle(FilterAllTypesButton) : SetDefaultStyle(FilterAllTypesButton);
	FilterAllTypesButton->Render();
	if (FilterAllTypesButton->IsClicked())
		ObjTypeFilterForResourcesContentBrowser = "";

	ImGui::SetCursorPosX(120 + 140 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResourcesContentBrowser == FEObjectTypeToString(FE_TEXTURE) ? SetSelectedStyle(FilterTextureTypeButton) : SetDefaultStyle(FilterTextureTypeButton);
	FilterTextureTypeButton->Render();
	if (FilterTextureTypeButton->IsClicked())
		ObjTypeFilterForResourcesContentBrowser = FEObjectTypeToString(FE_TEXTURE);

	ImGui::SetCursorPosX(120 + 140 + 48 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResourcesContentBrowser == FEObjectTypeToString(FE_MESH) ? SetSelectedStyle(FilterMeshTypeButton) : SetDefaultStyle(FilterMeshTypeButton);
	FilterMeshTypeButton->Render();
	if (FilterMeshTypeButton->IsClicked())
		ObjTypeFilterForResourcesContentBrowser = FEObjectTypeToString(FE_MESH);

	ImGui::SetCursorPosX(120 + 140 + 48 + 48 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResourcesContentBrowser == FEObjectTypeToString(FE_MATERIAL) ? SetSelectedStyle(FilterMaterialTypeButton) : SetDefaultStyle(FilterMaterialTypeButton);
	FilterMaterialTypeButton->Render();
	if (FilterMaterialTypeButton->IsClicked())
		ObjTypeFilterForResourcesContentBrowser = FEObjectTypeToString(FE_MATERIAL);

	ImGui::SetCursorPosX(120 + 140 + 48 + 48 + 48 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResourcesContentBrowser == FEObjectTypeToString(FE_GAMEMODEL) ? SetSelectedStyle(FilterGameModelTypeButton) : SetDefaultStyle(FilterGameModelTypeButton);
	FilterGameModelTypeButton->Render();
	if (FilterGameModelTypeButton->IsClicked())
		ObjTypeFilterForResourcesContentBrowser = FEObjectTypeToString(FE_GAMEMODEL);

	ImGui::SetCursorPosX(120 + 140 + 48 + 48 + 48 + 48 + 48);
	ImGui::SetCursorPosY(CurrentY - 5);
	ObjTypeFilterForResourcesContentBrowser == FEObjectTypeToString(FE_PREFAB) ? SetSelectedStyle(FilterPrefabTypeButton) : SetDefaultStyle(FilterPrefabTypeButton);
	FilterPrefabTypeButton->Render();
	if (FilterPrefabTypeButton->IsClicked())
		ObjTypeFilterForResourcesContentBrowser = FEObjectTypeToString(FE_PREFAB);

	ImGui::SetCursorPosX(100);
	ImGui::SetCursorPosY(CurrentY + 50);
	ImGui::Text("Filter by name: ");

	ImGui::SetCursorPosX(120 + 140);
	ImGui::SetCursorPosY(CurrentY + 47);
	ImGui::InputText("##filter", FilterForResourcesContentBrowser, IM_ARRAYSIZE(FilterForResourcesContentBrowser));

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25);
	ImGui::Separator();

	UpdateFilterForResourcesContentBrowser();
	UpdateDirectoryDragAndDropTargets();

	// ************** Drag&Drop **************
	if (VIRTUAL_FILE_SYSTEM.GetCurrentPath() != LastFramePath)
	{
		LastFramePath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
		UpdateDirectoryDragAndDropTargets();
	}
	int DirectoryIndex = 0;
	// ************** Drag&Drop END **************

	int IconsPerWindowWidth = (int)(ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (ContentBrowserItemIconSize + 8 + 32));
	// Possibly window is minimized anyway ImGui::Columns can't take 0 as columns count!
	if (IconsPerWindowWidth == 0)
		return;

	if (!bShouldOpenContextMenuInContentBrowser) ContentBrowserItemUnderMouse = -1;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
	ImGui::Columns(IconsPerWindowWidth, "mycolumns3", false);

	for (size_t i = 0; i < FilteredResourcesContentBrowser.size(); i++)
	{
		ImGui::PushID(int(std::hash<std::string>{}(FilteredResourcesContentBrowser[i]->GetObjectID())));

		if (ItemInFocus != nullptr && ItemInFocus->GetObjectID() == FilteredResourcesContentBrowser[i]->GetObjectID())
		{
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
		}
		
 		ImVec2 uv0 = ImVec2(0.0f, 0.0f);
		ImVec2 uv1 = ImVec2(1.0f, 1.0f);

		FETexture* PreviewTexture = nullptr;
		FETexture* SmallAdditionTypeIcon = nullptr;

		ChooseTexturesForContentBrowserItem(PreviewTexture, SmallAdditionTypeIcon, uv0, uv1, FilteredResourcesContentBrowser[i]);

		if (PreviewTexture != nullptr)
			ImGui::ImageButton((void*)(intptr_t)PreviewTexture->GetTextureID(), ImVec2(ContentBrowserItemIconSize, ContentBrowserItemIconSize), uv0, uv1, 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		
		if (FilteredResourcesContentBrowser[i]->GetType() == FE_NULL && ContentBrowserDirectoriesTargets.size() > (size_t)DirectoryIndex)
			ContentBrowserDirectoriesTargets[DirectoryIndex++]->StickToItem();

		if (ImGui::IsItemHovered())
		{
			if (!bShouldOpenContextMenuInContentBrowser && !DRAG_AND_DROP_MANAGER.ObjectIsDraged())
			{
				std::string AdditionalTypeInfo;
				if (FilteredResourcesContentBrowser[i]->GetType() == FE_TEXTURE)
				{
					AdditionalTypeInfo += "\nTexture type: ";
					AdditionalTypeInfo += FETexture::TextureInternalFormatToString(RESOURCE_MANAGER.GetTexture(FilteredResourcesContentBrowser[i]->GetObjectID())->GetInternalFormat());
				}

				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(("Name: " + FilteredResourcesContentBrowser[i]->GetName() +
										"\nType: " + FEObjectTypeToString(FilteredResourcesContentBrowser[i]->GetType()) +
										AdditionalTypeInfo +
										"\nPath: " + VIRTUAL_FILE_SYSTEM.GetCurrentPath()
										).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();

				ContentBrowserItemUnderMouse = int(i);

				if (ImGui::IsMouseDragging(0))
					DRAG_AND_DROP_MANAGER.SetObject(FilteredResourcesContentBrowser[i], PreviewTexture, uv0, uv1);
			}
		}

		if (SmallAdditionTypeIcon != nullptr)
		{
			ImVec2 CursorPosBefore = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(CursorPosBefore.x + 10, CursorPosBefore.y - 48));
			ImGui::Image((void*)(intptr_t)SmallAdditionTypeIcon->GetTextureID(), ImVec2(32, 32));
			ImGui::SetCursorPos(CursorPosBefore);
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopID();
		
		if (ContentBrowserRenameIndex == i)
		{
			if (!bLastFrameRenameEditWasVisiable)
			{
				ImGui::SetKeyboardFocusHere(0);
				ImGui::SetFocusID(ImGui::GetID("##newNameEditor"), ImGui::GetCurrentWindow());
				ImGui::SetItemDefaultFocus();
				bLastFrameRenameEditWasVisiable = true;
			}

			ImGui::SetNextItemWidth(ContentBrowserItemIconSize + 8.0f + 8.0f);
			if (ImGui::InputText("##newNameEditor", ContentBrowserRename, IM_ARRAYSIZE(ContentBrowserRename), ImGuiInputTextFlags_EnterReturnsTrue) ||
				ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##newNameEditor"))
			{
				if (FilteredResourcesContentBrowser[ContentBrowserRenameIndex]->GetType() == FE_NULL)
				{
					std::string PathToDirectory = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
					if (PathToDirectory.back() != '/')
						PathToDirectory += '/';

					PathToDirectory += FilteredResourcesContentBrowser[ContentBrowserRenameIndex]->GetName();
					VIRTUAL_FILE_SYSTEM.RenameDirectory(ContentBrowserRename, PathToDirectory);

					UpdateDirectoryDragAndDropTargets();
				}
				else
				{
					FilteredResourcesContentBrowser[ContentBrowserRenameIndex]->SetDirtyFlag(true);
					PROJECT_MANAGER.GetCurrent()->SetModified(true);
					FilteredResourcesContentBrowser[ContentBrowserRenameIndex]->SetName(ContentBrowserRename);
				}
				
				ContentBrowserRenameIndex = -1;
			}
		}
		else
		{
			ImVec2 TextSize = ImGui::CalcTextSize(FilteredResourcesContentBrowser[i]->GetName().c_str());
			if (TextSize.x < ContentBrowserItemIconSize + 8 + 8)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ContentBrowserItemIconSize + 8.0f + 8.0f) / 2.0f - TextSize.x / 2.0f);
				ImGui::Text(FilteredResourcesContentBrowser[i]->GetName().c_str());
			}
			else
			{
				ImGui::Text(FilteredResourcesContentBrowser[i]->GetName().c_str());
			}
		}
		
		ImGui::NextColumn();
	}

	if (ImGui::IsMouseDoubleClicked(0) && ContentBrowserItemUnderMouse != -1)
	{
		if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_NULL)
		{
			std::string CurrentPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (CurrentPath.back() != '/')
				CurrentPath += '/';

			CurrentPath += FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetName();
			VIRTUAL_FILE_SYSTEM.SetCurrentPath(CurrentPath);
		}
		else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_MESH)
		{
			std::string MeshInfo = "Vertex count: ";
			MeshInfo += std::to_string(RESOURCE_MANAGER.GetMesh(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID())->GetVertexCount());
			MeshInfo += "\n";
			MeshInfo += "Sub material socket: ";
			MeshInfo += RESOURCE_MANAGER.GetMesh(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID())->GetMaterialCount() == 2 ? "Yes" : "No";
			MessagePopUp::getInstance().Show("Mesh info", MeshInfo.c_str());
		}
		else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_MATERIAL)
		{
			EditMaterialPopup::getInstance().Show(RESOURCE_MANAGER.GetMaterial(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
		}
		else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_GAMEMODEL)
		{
			if (!bShouldOpenContextMenuInContentBrowser && !EditGameModelPopup::getInstance().IsVisible())
			{
				EditGameModelPopup::getInstance().Show(RESOURCE_MANAGER.GetGameModel(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
			}
		}
		else if (FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetType() == FE_PREFAB)
		{
			if (!bShouldOpenContextMenuInContentBrowser && !PrefabEditorWindow::getInstance().IsVisible())
			{
				PrefabEditorWindow::getInstance().Show(RESOURCE_MANAGER.GetPrefab(FilteredResourcesContentBrowser[ContentBrowserItemUnderMouse]->GetObjectID()));
			}
		}
	}

	ImGui::Columns(1);
}

void FEEditor::UpdateDirectoryDragAndDropTargets()
{
	ContentBrowserDirectoriesTargets.clear();
	DirectoryDragAndDropInfo.clear();
	AllResourcesContentBrowser.clear();
	AllResourcesContentBrowser = VIRTUAL_FILE_SYSTEM.GetDirectoryContent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());

	UpdateFilterForResourcesContentBrowser();

	DirectoryDragAndDropInfo.resize(VIRTUAL_FILE_SYSTEM.SubDirectoriesCount(VIRTUAL_FILE_SYSTEM.GetCurrentPath()));
	int SubDirectoryIndex = 0;
	for (size_t i = 0; i < FilteredResourcesContentBrowser.size(); i++)
	{
		if (FilteredResourcesContentBrowser[i]->GetType() == FE_NULL)
		{
			DirectoryDragAndDropCallbackInfo info;

			info.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetCurrentPath();
			if (VIRTUAL_FILE_SYSTEM.GetCurrentPath().back() != '/')
				info.DirectoryPath += "/";

			info.DirectoryPath += FilteredResourcesContentBrowser[i]->GetName() + "/";
			DirectoryDragAndDropInfo[SubDirectoryIndex] = info;

			ContentBrowserDirectoriesTargets.push_back(DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_NULL, FE_SHADER, FE_TEXTURE, FE_MESH, FE_MATERIAL, FE_GAMEMODEL, FE_PREFAB },
				DirectoryDragAndDropCallback, reinterpret_cast<void**>(&DirectoryDragAndDropInfo[SubDirectoryIndex]),
				std::vector<std::string> { "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder" }));
			SubDirectoryIndex++;
		}
	}

	if (VFSBackButtonTarget == nullptr)
	{
		VFSBackButtoninfo.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
		VFSBackButtonTarget = DRAG_AND_DROP_MANAGER.AddTarget(std::vector<FE_OBJECT_TYPE> { FE_NULL, FE_SHADER, FE_TEXTURE, FE_MESH, FE_MATERIAL, FE_GAMEMODEL, FE_PREFAB },
			DirectoryDragAndDropCallback, reinterpret_cast<void**>(&VFSBackButtoninfo),
			std::vector<std::string> { "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder" });
	}
	else
	{
		VFSBackButtoninfo.DirectoryPath = VIRTUAL_FILE_SYSTEM.GetDirectoryParent(VIRTUAL_FILE_SYSTEM.GetCurrentPath());
		//VFSBackButton->setNewUserData();
	}
}

void FEEditor::UpdateFilterForResourcesContentBrowser()
{
	FilteredResourcesContentBrowser.clear();

	// Filter by name.
	if (strlen(FilterForResourcesContentBrowser) == 0)
	{
		FilteredResourcesContentBrowser = AllResourcesContentBrowser;
	}
	else
	{
		for (size_t i = 0; i < AllResourcesContentBrowser.size(); i++)
		{
			if (AllResourcesContentBrowser[i]->GetName().find(FilterForResourcesContentBrowser) != -1)
			{
				FilteredResourcesContentBrowser.push_back(AllResourcesContentBrowser[i]);
			}
		}
	}

	// Filter by type.
	if (!ObjTypeFilterForResourcesContentBrowser.empty())
	{
		std::vector<FEObject*> FinalFilteredList;
		for (size_t i = 0; i < FilteredResourcesContentBrowser.size(); i++)
		{
			if (FEObjectTypeToString(FilteredResourcesContentBrowser[i]->GetType()) == ObjTypeFilterForResourcesContentBrowser ||
				// Add folders
				FilteredResourcesContentBrowser[i]->GetType() == FE_NULL)
			{
				FinalFilteredList.push_back(FilteredResourcesContentBrowser[i]);
			}
		}

		FilteredResourcesContentBrowser = FinalFilteredList;
	}
}