#include "../Editor/FEEditor.h"

static int itemIconSide = 128 + 8;
void FEEditor::displayContentBrowser()
{
	if (!contentBrowserVisible)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_None);

	displayContentBrowserItems();

	bool open_context_menu = false;
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
		open_context_menu = true;

	if (open_context_menu)
		ImGui::OpenPopup("##context_menu");

	isOpenContextMenuInContentBrowser = false;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		isOpenContextMenuInContentBrowser = true;

		if (contentBrowserItemUnderMouse == -1)
		{
			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::MenuItem("Add folder"))
				{
					std::string newDirectoryName = VIRTUAL_FILE_SYSTEM.createDirectory(VIRTUAL_FILE_SYSTEM.getCurrentPath());
					updateDirectoryDragAndDropTargets();

					for (size_t i = 0; i < filteredResourcesContentBrowser.size(); i++)
					{
						if (filteredResourcesContentBrowser[i]->getName() == newDirectoryName)
						{
							contentBrowserRenameIndex = i;
							strcpy_s(contentBrowserRename, filteredResourcesContentBrowser[i]->getName().size() + 1, filteredResourcesContentBrowser[i]->getName().c_str());
							lastFrameRenameEditWasVisiable = false;
							break;
						}
					}
				}

				if (ImGui::MenuItem("Load mesh..."))
				{
					std::string filePath = "";
					FILESYSTEM.openDialog(filePath, meshLoadFilter, 1);
					if (filePath != "")
					{
						FEMesh* loadedMesh = RESOURCE_MANAGER.LoadOBJMesh(filePath.c_str());
						VIRTUAL_FILE_SYSTEM.createFile(loadedMesh, VIRTUAL_FILE_SYSTEM.getCurrentPath());
						// checking material count in this mesh
						if (loadedMesh != nullptr && loadedMesh->getMaterialCount() > 2)
						{
							messagePopUp::getInstance().show("Error!", "Mesh that you was trying to load has more than 2 materials, currently it is not supported!");
							RESOURCE_MANAGER.deleteFEMesh(loadedMesh);
						}
						else
						{
							PROJECT_MANAGER.getCurrent()->modified = true;
							loadedMesh->setDirtyFlag(true);
						}
					}
				}

				if (ImGui::BeginMenu("Texture"))
				{
					if (ImGui::MenuItem("Load..."))
					{
						std::string filePath = "";
						FILESYSTEM.openDialog(filePath, textureLoadFilter, 1);
						if (filePath != "")
						{
							loadTexturePopUp::getInstance().show(filePath);
							PROJECT_MANAGER.getCurrent()->modified = true;
						}
					}

					if (ImGui::MenuItem("Load and combine with opacity mask..."))
					{
						std::string filePath = "";
						FILESYSTEM.openDialog(filePath, textureLoadFilter, 1);

						std::string maskFilePath = "";
						FILESYSTEM.openDialog(maskFilePath, textureLoadFilter, 1);

						if (filePath != "" && maskFilePath != "")
						{
							FETexture* newTexture = RESOURCE_MANAGER.LoadPNGTextureWithTransparencyMask(filePath.c_str(), maskFilePath.c_str(), "");
							newTexture->setDirtyFlag(true);
							PROJECT_MANAGER.getCurrent()->modified = true;
							VIRTUAL_FILE_SYSTEM.createFile(newTexture, VIRTUAL_FILE_SYSTEM.getCurrentPath());
						}
					}

					if (ImGui::MenuItem("Combine channels..."))
					{
						CombineChannelsToTexturePopUp::getInstance().show("");
					}

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Create new material"))
				{
					FEMaterial* newMat = RESOURCE_MANAGER.createMaterial("");
					if (newMat)
					{
						PROJECT_MANAGER.getCurrent()->modified = true;
						newMat->shader = RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/);

						newMat->setAlbedoMap(RESOURCE_MANAGER.noTexture);
						newMat->setNormalMap(RESOURCE_MANAGER.noTexture);

						VIRTUAL_FILE_SYSTEM.createFile(newMat, VIRTUAL_FILE_SYSTEM.getCurrentPath());
					}
				}

				if (ImGui::MenuItem("Create new game model"))
				{
					FEGameModel* newGameModel = RESOURCE_MANAGER.createGameModel();
					PROJECT_MANAGER.getCurrent()->modified = true;
					VIRTUAL_FILE_SYSTEM.createFile(newGameModel, VIRTUAL_FILE_SYSTEM.getCurrentPath());
				}

				ImGui::EndMenu();
			}
		}
		else
		{
			std::string fullPath = VIRTUAL_FILE_SYSTEM.getCurrentPath();
			if (fullPath.back() != '/')
				fullPath += '/';
			fullPath += filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getName();

			bool readOnlyItem = VIRTUAL_FILE_SYSTEM.isReadOnly(filteredResourcesContentBrowser[contentBrowserItemUnderMouse], fullPath);

			if (readOnlyItem)
				ImGui::MenuItem("Read Only");
			
			if (!readOnlyItem)
			{
				if (ImGui::MenuItem("Rename"))
				{
					contentBrowserRenameIndex = contentBrowserItemUnderMouse;

					strcpy_s(contentBrowserRename, filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getName().size() + 1, filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getName().c_str());
					lastFrameRenameEditWasVisiable = false;
				}
			}

			if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_MATERIAL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					editMaterialPopup::getInstance().show(RESOURCE_MANAGER.getMaterial(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
			}

			if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_GAMEMODEL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					editGameModelPopup::getInstance().show(RESOURCE_MANAGER.getGameModel(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
			}

			if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_SHADER)
			{
				if (ImGui::MenuItem("Edit"))
				{
					shaderEditorWindow::getInstance().show(RESOURCE_MANAGER.getShader(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
			}

			if (!readOnlyItem)
			{
				if (ImGui::MenuItem("Delete"))
				{
					if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_NULL)
					{
						deleteDirectoryPopup::getInstance().show(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getName());
					}
					else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_MESH)
					{
						deleteMeshPopup::getInstance().show(RESOURCE_MANAGER.getMesh(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
					}
					else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_TEXTURE)
					{
						deleteTexturePopup::getInstance().show(RESOURCE_MANAGER.getTexture(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
					}
					else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_MATERIAL)
					{
						deleteMaterialPopup::getInstance().show(RESOURCE_MANAGER.getMaterial(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
					}
					else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_GAMEMODEL)
					{
						deleteGameModelPopup::getInstance().show(RESOURCE_MANAGER.getGameModel(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
					}
				}
			}

			if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_TEXTURE)
			{
				if (ImGui::BeginMenu("Convert"))
				{
					if (ImGui::MenuItem("Texture channels to individual textures"))
					{
						std::vector<FETexture*> newTextures = RESOURCE_MANAGER.channelsToFETextures(RESOURCE_MANAGER.getTexture(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
						newTextures[0]->setDirtyFlag(true);
						PROJECT_MANAGER.getCurrent()->modified = true;

						VIRTUAL_FILE_SYSTEM.createFile(newTextures[0], VIRTUAL_FILE_SYSTEM.getCurrentPath());
						VIRTUAL_FILE_SYSTEM.createFile(newTextures[1], VIRTUAL_FILE_SYSTEM.getCurrentPath());
						VIRTUAL_FILE_SYSTEM.createFile(newTextures[2], VIRTUAL_FILE_SYSTEM.getCurrentPath());
						VIRTUAL_FILE_SYSTEM.createFile(newTextures[3], VIRTUAL_FILE_SYSTEM.getCurrentPath());
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

	loadTexturePopUp::getInstance().render();
	renameFailedPopUp::getInstance().render();
	deleteTexturePopup::getInstance().render();
	deleteMeshPopup::getInstance().render();
	deleteGameModelPopup::getInstance().render();
	editGameModelPopup::getInstance().render();
	editMaterialPopup::getInstance().render();
	deleteMaterialPopup::getInstance().render();
	messagePopUp::getInstance().render();
	shaderEditorWindow::getInstance().render();
	deleteDirectoryPopup::getInstance().render();
	CombineChannelsToTexturePopUp::getInstance().render();
}

void FEEditor::displayContentBrowserItems()
{
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	float currentY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(currentY);
	if (ImGui::ImageButton((void*)(intptr_t)VFSBackIcon->getTextureID(), ImVec2(64, 64), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
	{
		VIRTUAL_FILE_SYSTEM.setCurrentPath(VIRTUAL_FILE_SYSTEM.getDirectoryParent(VIRTUAL_FILE_SYSTEM.getCurrentPath()));
		strcpy_s(filterForResourcesContentBrowser, "");
	}
	VFSBackButtonTarget->stickToItem();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	static std::string lastFramePath = "";
	allResourcesContentBrowser.clear();
	allResourcesContentBrowser = VIRTUAL_FILE_SYSTEM.getDirectoryContent(VIRTUAL_FILE_SYSTEM.getCurrentPath());

	ImGui::SetCursorPosX(120);
	ImGui::SetCursorPosY(currentY + 30);
	ImGui::Text("Filter: ");

	ImGui::SetCursorPosX(120 + 90);
	ImGui::SetCursorPosY(currentY + 27);
	ImGui::InputText("##filter", filterForResourcesContentBrowser, IM_ARRAYSIZE(filterForResourcesContentBrowser));

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25);
	ImGui::Separator();

	if (strlen(filterForResourcesContentBrowser) == 0)
	{
		filteredResourcesContentBrowser = allResourcesContentBrowser;
		updateDirectoryDragAndDropTargets();
	}
	else
	{
		filteredResourcesContentBrowser.clear();
		for (size_t i = 0; i < allResourcesContentBrowser.size(); i++)
		{
			if (allResourcesContentBrowser[i]->getName().find(filterForResourcesContentBrowser) != -1)
			{
				filteredResourcesContentBrowser.push_back(allResourcesContentBrowser[i]);
			}
		}
		updateDirectoryDragAndDropTargets();
	}

	// ************** Drag&Drop **************
	if (VIRTUAL_FILE_SYSTEM.getCurrentPath() != lastFramePath)
	{
		lastFramePath = VIRTUAL_FILE_SYSTEM.getCurrentPath();
		updateDirectoryDragAndDropTargets();
	}
	int directoryIndex = 0;
	// ************** Drag&Drop END **************

	int iconsPerWindowWidth = (int)ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (itemIconSide + 32);
	// Possibly window is minimized anyway ImGui::Columns can't take 0 as columns count!
	if (iconsPerWindowWidth == 0)
		return;

	if (!isOpenContextMenuInContentBrowser) contentBrowserItemUnderMouse = -1;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
	ImGui::Columns(iconsPerWindowWidth, "mycolumns3", false);

	for (size_t i = 0; i < filteredResourcesContentBrowser.size(); i++)
	{
		ImGui::PushID(std::hash<std::string>{}(filteredResourcesContentBrowser[i]->getObjectID()));
		
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

		ImVec2 uv0 = ImVec2(0.0f, 1.0f);
		ImVec2 uv1 = ImVec2(1.0f, 0.0f);
		FETexture* previewTexture = nullptr;

		if (filteredResourcesContentBrowser[i]->getType() == FE_NULL)
		{
			uv0 = ImVec2(0.0f, 0.0f);
			uv1 = ImVec2(1.0f, 1.0f);

			previewTexture = folderIcon;

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
		}
		if (filteredResourcesContentBrowser[i]->getType() == FE_SHADER)
		{
			uv0 = ImVec2(0.0f, 0.0f);
			uv1 = ImVec2(1.0f, 1.0f);

			previewTexture = shaderIcon;

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
		}
		else if (filteredResourcesContentBrowser[i]->getType() == FE_MESH)
		{
			previewTexture = PREVIEW_MANAGER.getMeshPreview(filteredResourcesContentBrowser[i]->getObjectID());
		}
		else if (filteredResourcesContentBrowser[i]->getType() == FE_TEXTURE)
		{
			previewTexture = RESOURCE_MANAGER.getTexture(filteredResourcesContentBrowser[i]->getObjectID());
		}
		else if (filteredResourcesContentBrowser[i]->getType() == FE_MATERIAL)
		{
			previewTexture = PREVIEW_MANAGER.getMaterialPreview(filteredResourcesContentBrowser[i]->getObjectID());
		}
		else if (filteredResourcesContentBrowser[i]->getType() == FE_GAMEMODEL)
		{
			previewTexture = PREVIEW_MANAGER.getGameModelPreview(filteredResourcesContentBrowser[i]->getObjectID());
		}

		ImGui::ImageButton((void*)(intptr_t)previewTexture->getTextureID(), ImVec2(128, 128), uv0, uv1, 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		if (filteredResourcesContentBrowser[i]->getType() == FE_NULL && contentBrowserDirectoriesTargets.size() > (size_t)directoryIndex)
			contentBrowserDirectoriesTargets[directoryIndex++]->stickToItem();

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser && !DRAG_AND_DROP_MANAGER.objectIsDraged())
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(("Name: " + filteredResourcesContentBrowser[i]->getName() +
										"\nType: " + FEObjectTypeToString(filteredResourcesContentBrowser[i]->getType()) +
										"\nPath: " + VIRTUAL_FILE_SYSTEM.getCurrentPath()
										).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();

				contentBrowserItemUnderMouse = i;

				if (ImGui::IsMouseDragging(0))
					DRAG_AND_DROP_MANAGER.setObject(filteredResourcesContentBrowser[i], previewTexture, uv0, uv1);
			}
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopID();
		
		if (contentBrowserRenameIndex == i)
		{
			if (!lastFrameRenameEditWasVisiable)
			{
				ImGui::SetKeyboardFocusHere(0);
				ImGui::SetFocusID(ImGui::GetID("##newNameEditor"), ImGui::GetCurrentWindow());
				ImGui::SetItemDefaultFocus();
				lastFrameRenameEditWasVisiable = true;
			}

			ImGui::SetNextItemWidth(itemIconSide + 8.0f);
			if (ImGui::InputText("##newNameEditor", contentBrowserRename, IM_ARRAYSIZE(contentBrowserRename), ImGuiInputTextFlags_EnterReturnsTrue) ||
				ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered() || ImGui::GetFocusID() != ImGui::GetID("##newNameEditor"))
			{
				if (filteredResourcesContentBrowser[contentBrowserRenameIndex]->getType() == FE_NULL)
				{
					std::string pathToDirectory = VIRTUAL_FILE_SYSTEM.getCurrentPath();
					if (pathToDirectory.back() != '/')
						pathToDirectory += '/';

					pathToDirectory += filteredResourcesContentBrowser[contentBrowserRenameIndex]->getName();
					VIRTUAL_FILE_SYSTEM.renameDirectory(contentBrowserRename, pathToDirectory);

					updateDirectoryDragAndDropTargets();
				}
				else
				{
					filteredResourcesContentBrowser[contentBrowserRenameIndex]->setDirtyFlag(true);
					PROJECT_MANAGER.getCurrent()->modified = true;
					filteredResourcesContentBrowser[contentBrowserRenameIndex]->setName(contentBrowserRename);
				}
				
				contentBrowserRenameIndex = -1;
			}
		}
		else
		{
			ImVec2 textSize = ImGui::CalcTextSize(filteredResourcesContentBrowser[i]->getName().c_str());
			if (textSize.x < itemIconSide + 8)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (itemIconSide + 8.0f) / 2.0f - textSize.x / 2.0f);
				ImGui::Text(filteredResourcesContentBrowser[i]->getName().c_str());
			}
			else
			{
				ImGui::Text(filteredResourcesContentBrowser[i]->getName().c_str());
			}
		}
		
		ImGui::NextColumn();
	}

	if (ImGui::IsMouseDoubleClicked(0) && contentBrowserItemUnderMouse != -1)
	{
		if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_NULL)
		{
			std::string currentPath = VIRTUAL_FILE_SYSTEM.getCurrentPath();
			if (currentPath.back() != '/')
				currentPath += '/';

			currentPath += filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getName();
			VIRTUAL_FILE_SYSTEM.setCurrentPath(currentPath);
		}
		else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_MESH)
		{
			std::string meshInfo = "Vertex count: ";
			meshInfo += std::to_string(RESOURCE_MANAGER.getMesh(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID())->getVertexCount());
			meshInfo += "\n";
			meshInfo += "Sub material socket: ";
			meshInfo += RESOURCE_MANAGER.getMesh(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID())->getMaterialCount() == 2 ? "Yes" : "No";
			messagePopUp::getInstance().show("Mesh info", meshInfo.c_str());
		}
		else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_MATERIAL)
		{
			editMaterialPopup::getInstance().show(RESOURCE_MANAGER.getMaterial(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
		}
		else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_GAMEMODEL)
		{
			if (!isOpenContextMenuInContentBrowser && !editGameModelPopup::getInstance().isVisible())
			{
				editGameModelPopup::getInstance().show(RESOURCE_MANAGER.getGameModel(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
			}
		}
	}

	ImGui::Columns(1);
}

void FEEditor::updateDirectoryDragAndDropTargets()
{
	contentBrowserDirectoriesTargets.clear();
	directoryDragAndDropInfo.clear();
	allResourcesContentBrowser.clear();
	allResourcesContentBrowser = VIRTUAL_FILE_SYSTEM.getDirectoryContent(VIRTUAL_FILE_SYSTEM.getCurrentPath());

	if (strlen(filterForResourcesContentBrowser) == 0)
	{
		filteredResourcesContentBrowser = allResourcesContentBrowser;
	}
	else
	{
		filteredResourcesContentBrowser.clear();
		for (size_t i = 0; i < allResourcesContentBrowser.size(); i++)
		{
			if (allResourcesContentBrowser[i]->getName().find(filterForResourcesContentBrowser) != -1)
			{
				filteredResourcesContentBrowser.push_back(allResourcesContentBrowser[i]);
			}
		}
	}

	directoryDragAndDropInfo.resize(VIRTUAL_FILE_SYSTEM.subDirectoriesCount(VIRTUAL_FILE_SYSTEM.getCurrentPath()));
	int subDirectoryIndex = 0;
	for (size_t i = 0; i < filteredResourcesContentBrowser.size(); i++)
	{
		if (filteredResourcesContentBrowser[i]->getType() == FE_NULL)
		{
			directoryDragAndDropCallbackInfo info;
			if (VIRTUAL_FILE_SYSTEM.getCurrentPath().back() != '/')
				info.directoryPath += "/";

			info.directoryPath += VIRTUAL_FILE_SYSTEM.getCurrentPath() + filteredResourcesContentBrowser[i]->getName() + "/";
			directoryDragAndDropInfo[subDirectoryIndex] = info;

			contentBrowserDirectoriesTargets.push_back(DRAG_AND_DROP_MANAGER.addTarget(std::vector<FEObjectType> { FE_NULL, FE_SHADER, FE_TEXTURE, FE_MESH, FE_MATERIAL, FE_GAMEMODEL },
				directoryDragAndDropCallback, reinterpret_cast<void**>(&directoryDragAndDropInfo[subDirectoryIndex]),
				std::vector<std::string> { "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder", "Drop to move to folder" }));
			subDirectoryIndex++;
		}
	}

	if (VFSBackButtonTarget == nullptr)
	{
		VFSBackButtoninfo.directoryPath = VIRTUAL_FILE_SYSTEM.getDirectoryParent(VIRTUAL_FILE_SYSTEM.getCurrentPath());
		VFSBackButtonTarget = DRAG_AND_DROP_MANAGER.addTarget(std::vector<FEObjectType> { FE_NULL, FE_SHADER, FE_TEXTURE, FE_MESH, FE_MATERIAL, FE_GAMEMODEL },
			directoryDragAndDropCallback, reinterpret_cast<void**>(&VFSBackButtoninfo),
			std::vector<std::string> { "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder", "Drop to move to parent folder" });
	}
	else
	{
		VFSBackButtoninfo.directoryPath = VIRTUAL_FILE_SYSTEM.getDirectoryParent(VIRTUAL_FILE_SYSTEM.getCurrentPath());
		//VFSBackButton->setNewUserData();
	}
}