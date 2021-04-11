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
			if (ImGui::BeginMenu("Add resource"))
			{
				if (ImGui::MenuItem("Load mesh..."))
				{
					std::string filePath = "";
					FILESYSTEM.openDialog(filePath, meshLoadFilter, 1);
					if (filePath != "")
					{
						FEMesh* loadedMesh = RESOURCE_MANAGER.LoadOBJMesh(filePath.c_str());
						// checking material count in this mesh
						if (loadedMesh != nullptr && loadedMesh->getMaterialCount() > 2)
						{
							messagePopUpObj.show("Error!", "Mesh that you was trying to load has more than 2 materials, currently it is not supported!");
							RESOURCE_MANAGER.deleteFEMesh(loadedMesh);
						}
						else
						{
							PROJECT_MANAGER.getCurrent()->modified = true;
							loadedMesh->setDirtyFlag(true);
						}
					}
				}

				if (ImGui::MenuItem("Load texture..."))
				{
					std::string filePath = "";
					FILESYSTEM.openDialog(filePath, textureLoadFilter, 1);
					if (filePath != "")
					{
						loadTextureWindow.show(filePath);
						PROJECT_MANAGER.getCurrent()->modified = true;
					}
				}

				if (ImGui::MenuItem("Load texture and combine with opacity mask..."))
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
					}
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
					}
				}

				if (ImGui::MenuItem("Create new game model"))
				{
					RESOURCE_MANAGER.createGameModel();
					PROJECT_MANAGER.getCurrent()->modified = true;
				}

				ImGui::EndMenu();
			}
		}
		else
		{
			if (ImGui::MenuItem("Rename"))
			{
				renamePopUp::getInstance().show(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]);
			}

			if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_MATERIAL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					editMaterialWindow.show(RESOURCE_MANAGER.getMaterial(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
			}

			if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_GAMEMODEL)
			{
				if (ImGui::MenuItem("Edit"))
				{
					editGameModelWindow.show(RESOURCE_MANAGER.getGameModel(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
			}

			if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_SHADER)
			{
				if (ImGui::MenuItem("Edit"))
				{
					shadersEditorWindow.show(RESOURCE_MANAGER.getShader(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
			}

			if (ImGui::MenuItem("Delete"))
			{
				if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_MESH)
				{
					deleteMeshWindow.show(RESOURCE_MANAGER.getMesh(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
				else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_TEXTURE)
				{
					deleteTextureWindow.show(RESOURCE_MANAGER.getTexture(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
				else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_MATERIAL)
				{
					deleteMaterialWindow.show(RESOURCE_MANAGER.getMaterial(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
				else if (filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getType() == FE_GAMEMODEL)
				{
					deleteGameModelWindow.show(RESOURCE_MANAGER.getGameModel(filteredResourcesContentBrowser[contentBrowserItemUnderMouse]->getObjectID()));
				}
			}
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
	ImGui::PopStyleVar();
	ImGui::End();

	loadTextureWindow.render();
	renameFailedPopUp::getInstance().render();
	deleteTextureWindow.render();
	deleteMeshWindow.render();
	deleteGameModelWindow.render();
	editGameModelWindow.render();
	editMaterialWindow.render();
	deleteMaterialWindow.render();
	messagePopUpObj.render();
	shadersEditorWindow.render();
}

void FEEditor::displayContentBrowserItems()
{
	allResourcesContentBrowser.clear();

	std::vector<std::string> shaderList = RESOURCE_MANAGER.getStandardShadersList();
	for (size_t i = 0; i < shaderList.size(); i++)
	{
		if (FEObjectManager::getInstance().getFEObject(shaderList[i]) == nullptr)
			continue;
		allResourcesContentBrowser.push_back(FEObjectManager::getInstance().getFEObject(shaderList[i]));
	}

	std::vector<std::string> standardShaderList = RESOURCE_MANAGER.getShadersList();
	for (size_t i = 0; i < standardShaderList.size(); i++)
	{
		if (FEObjectManager::getInstance().getFEObject(standardShaderList[i]) == nullptr)
			continue;
		allResourcesContentBrowser.push_back(FEObjectManager::getInstance().getFEObject(standardShaderList[i]));
	}

	std::vector<std::string> meshList = RESOURCE_MANAGER.getMeshList();
	for (size_t i = 0; i < meshList.size(); i++)
	{
		allResourcesContentBrowser.push_back(FEObjectManager::getInstance().getFEObject(meshList[i]));
	}

	std::vector<std::string> textureList = RESOURCE_MANAGER.getTextureList();
	for (size_t i = 0; i < textureList.size(); i++)
	{
		allResourcesContentBrowser.push_back(FEObjectManager::getInstance().getFEObject(textureList[i]));
	}

	std::vector<std::string> materialList = RESOURCE_MANAGER.getMaterialList();
	for (size_t i = 0; i < materialList.size(); i++)
	{
		allResourcesContentBrowser.push_back(FEObjectManager::getInstance().getFEObject(materialList[i]));
	}

	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		allResourcesContentBrowser.push_back(FEObjectManager::getInstance().getFEObject(gameModelList[i]));
	}

	float currentY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(currentY + 5);
	ImGui::Text("Filter: ");

	ImGui::SetCursorPosY(currentY);
	ImGui::SetCursorPosX(97);
	ImGui::InputText("##filter", filterForResourcesContentBrowser, IM_ARRAYSIZE(filterForResourcesContentBrowser));

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

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	int iconsPerWindowWidth = (int)ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (itemIconSide + 32);
	if (!isOpenContextMenuInContentBrowser) contentBrowserItemUnderMouse = -1;
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
	ImGui::Columns(iconsPerWindowWidth, "mycolumns3", false);

	for (size_t i = 0; i < filteredResourcesContentBrowser.size(); i++)
	{
		ImGui::PushID(std::hash<std::string>{}(filteredResourcesContentBrowser[i]->getObjectID()));

		FETexture* previewTexture = nullptr;
		if (filteredResourcesContentBrowser[i]->getType() == FE_SHADER)
		{
			previewTexture = RESOURCE_MANAGER.noTexture;
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

		if (ImGui::ImageButton((void*)(intptr_t)previewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			if (filteredResourcesContentBrowser[i]->getType() == FE_MESH)
			{
				std::string meshInfo = "Vertex count: ";
				meshInfo += std::to_string(RESOURCE_MANAGER.getMesh(filteredResourcesContentBrowser[i]->getObjectID())->getVertexCount());
				meshInfo += "\n";
				meshInfo += "Sub material socket: ";
				meshInfo += RESOURCE_MANAGER.getMesh(filteredResourcesContentBrowser[i]->getObjectID())->getMaterialCount() == 2 ? "Yes" : "No";
				messagePopUpObj.show("Mesh info", meshInfo.c_str());
			}
			else if (filteredResourcesContentBrowser[i]->getType() == FE_GAMEMODEL)
			{
				if (!isOpenContextMenuInContentBrowser && !editGameModelWindow.isVisible())
				{
					editGameModelWindow.show(RESOURCE_MANAGER.getGameModel(filteredResourcesContentBrowser[i]->getObjectID()));
				}
			}
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser)
			{
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::TextUnformatted(("Name: " + filteredResourcesContentBrowser[i]->getName() + "\nType: " + FEObjectTypeToString(filteredResourcesContentBrowser[i]->getType())).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();

				contentBrowserItemUnderMouse = i;
			}
		}

		ImGui::PopID();
		ImGui::Text(filteredResourcesContentBrowser[i]->getName().c_str());
		ImGui::NextColumn();
	}

	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}