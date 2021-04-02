#include "../Editor/FEEditor.h"

static int itemIconSide = 128;
void FEEditor::displayContentBrowser()
{
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_None);
	if (ImGui::BeginTabBar("##Content Browser", ImGuiTabBarFlags_None))
	{
		ImGui::PushStyleColor(ImGuiCol_TabActive, (ImVec4)ImColor::ImColor(0.4f, 0.9f, 0.4f, 1.0f));

		if (ImGui::BeginTabItem("Meshes"))
		{
			activeTabContentBrowser = 0;
			displayMeshesContentBrowser();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Textures"))
		{
			activeTabContentBrowser = 1;
			displayTexturesContentBrowser();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Shaders"))
		{
			activeTabContentBrowser = 2;
			displayShadersContentBrowser();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Materials"))
		{
			activeTabContentBrowser = 3;
			displayMaterialContentBrowser();
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Game Models"))
		{
			activeTabContentBrowser = 4;
			displayGameModelContentBrowser();
			ImGui::EndTabItem();
		}

		/*if (ImGui::BeginTabItem("PostProcess"))
		{
			activeTabContentBrowser = 6;
			displayPostProcessContentBrowser();
			ImGui::EndTabItem();
		}*/

		if (ImGui::BeginTabItem("Effects"))
		{
			activeTabContentBrowser = 7;
			displayEffectsContentBrowser();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
		ImGui::PopStyleColor();
	}

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
		switch (activeTabContentBrowser)
		{
		case 0:
		{
			if (meshUnderMouse == -1)
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
			}

			if (meshUnderMouse != -1)
			{
				if (ImGui::MenuItem("Delete"))
				{
					deleteMeshWindow.show(RESOURCE_MANAGER.getMesh(RESOURCE_MANAGER.getMeshList()[meshUnderMouse]));
				}

				if (ImGui::MenuItem("Rename"))
				{
					renamePopUp::getInstance().show(RESOURCE_MANAGER.getMesh(RESOURCE_MANAGER.getMeshList()[meshUnderMouse]));
				}
			}

			break;
		}
		case 1:
		{
			if (textureUnderMouse == -1)
			{
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
						//RESOURCE_MANAGER.saveFETexture(newTexture, (PROJECT_MANAGER.getCurrent()->getProjectFolder() + newTexture->getName() + ".FETexture").c_str());
						PROJECT_MANAGER.getCurrent()->modified = true;
					}
				}
			}

			if (textureUnderMouse != -1)
			{
				if (ImGui::MenuItem("Delete"))
				{
					deleteTextureWindow.show(RESOURCE_MANAGER.getTexture(RESOURCE_MANAGER.getTextureList()[textureUnderMouse]));
				}

				if (ImGui::MenuItem("Rename"))
				{
					renamePopUp::getInstance().show(RESOURCE_MANAGER.getTexture(RESOURCE_MANAGER.getTextureList()[textureUnderMouse]));
				}
			}
			break;
		}
		case 2:
		{
			if (shaderIDUnderMouse != "")
			{
				if (ImGui::MenuItem("Edit..."))
				{
					shadersEditorWindow.show(RESOURCE_MANAGER.getShader(shaderIDUnderMouse));
				}
			}
			break;
		}
		case 3:
		{
			if (materialUnderMouse == -1)
			{
				if (ImGui::MenuItem("Create new material..."))
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
				break;
			}

			if (materialUnderMouse != -1)
			{
				if (ImGui::MenuItem("Edit"))
				{
					editMaterialWindow.show(RESOURCE_MANAGER.getMaterial(RESOURCE_MANAGER.getMaterialList()[materialUnderMouse]));
				}

				if (ImGui::MenuItem("Delete"))
				{
					deleteMaterialWindow.show(RESOURCE_MANAGER.getMaterial(RESOURCE_MANAGER.getMaterialList()[materialUnderMouse]));
				}

				if (ImGui::MenuItem("Rename"))
				{
					renamePopUp::getInstance().show(RESOURCE_MANAGER.getMaterial(RESOURCE_MANAGER.getMaterialList()[materialUnderMouse]));
				}
			}
			break;
		}
		case 4:
		{
			if (gameModelUnderMouse == -1)
			{
				if (ImGui::MenuItem("Create new game model"))
				{
					RESOURCE_MANAGER.createGameModel();
					PROJECT_MANAGER.getCurrent()->modified = true;
				}
			}

			if (gameModelUnderMouse != -1)
			{
				if (ImGui::MenuItem("Edit"))
				{
					editGameModelWindow.show(RESOURCE_MANAGER.getGameModel(RESOURCE_MANAGER.getGameModelList()[gameModelUnderMouse]));
				}

				if (ImGui::MenuItem("Delete"))
				{
					deleteGameModelWindow.show(RESOURCE_MANAGER.getGameModel(RESOURCE_MANAGER.getGameModelList()[gameModelUnderMouse]));
				}

				if (ImGui::MenuItem("Rename"))
				{
					renamePopUp::getInstance().show(RESOURCE_MANAGER.getGameModel(RESOURCE_MANAGER.getGameModelList()[gameModelUnderMouse]));
				}
			}
			break;
		}
		default:
			break;
		}
		ImGui::EndPopup();
	}

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

void FEEditor::displayMeshesContentBrowser()
{
	std::vector<std::string> meshList = RESOURCE_MANAGER.getMeshList();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	int iconsPerWindowWidth = (int)ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (itemIconSide + 32);
	if (!isOpenContextMenuInContentBrowser) meshUnderMouse = -1;
	ImGui::Columns(iconsPerWindowWidth, "mycolumns3", false);
	for (size_t i = 0; i < meshList.size(); i++)
	{
		FETexture* meshPreviewTexture = PREVIEW_MANAGER.getMeshPreview(meshList[i]);

		if (ImGui::ImageButton((void*)(intptr_t)meshPreviewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			std::string meshInfo = "Vertex count: ";
			meshInfo += std::to_string(RESOURCE_MANAGER.getMesh(meshList[i])->getVertexCount());
			meshInfo += "\n";
			meshInfo += "Sub material socket: ";
			meshInfo += RESOURCE_MANAGER.getMesh(meshList[i])->getMaterialCount() == 2 ? "Yes" : "No";
			messagePopUpObj.show("Mesh info", meshInfo.c_str());
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) meshUnderMouse = i;
		}

		ImGui::Text(RESOURCE_MANAGER.getMesh(meshList[i])->getName().c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void FEEditor::displayTexturesContentBrowser()
{
	std::vector<std::string> textureList = RESOURCE_MANAGER.getTextureList();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	int iconsPerWindowWidth = (int)ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (itemIconSide + 32);
	if (!isOpenContextMenuInContentBrowser) textureUnderMouse = -1;
	ImGui::Columns(iconsPerWindowWidth, "mycolumns3", false);
	for (size_t i = 0; i < textureList.size(); i++)
	{
		if (ImGui::ImageButton((void*)(intptr_t)RESOURCE_MANAGER.getTexture(textureList[i])->getTextureID(), ImVec2((float)itemIconSide, (float)itemIconSide), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			//
		}
		
		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) textureUnderMouse = i;
		}

		ImGui::Text(RESOURCE_MANAGER.getTexture(textureList[i])->getName().c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void FEEditor::displayShadersContentBrowser()
{
	//static std::string selectedShader = "";
	std::vector<std::string> shaderList = RESOURCE_MANAGER.getStandardShadersList();
	std::vector<std::string> standardShaderList = RESOURCE_MANAGER.getShadersList();
	for (size_t i = 0; i < standardShaderList.size(); i++)
	{
		shaderList.push_back(standardShaderList[i]);
	}

	/*if (ImGui::BeginCombo("Shaders", selectedShader.c_str(), ImGuiWindowFlags_None))
	{
		for (size_t n = 0; n < shaderList.size(); n++)
		{
			bool is_selected = (selectedShader == shaderList[n]);
			if (ImGui::Selectable(shaderList[n].c_str(), is_selected))
				selectedShader = shaderList[n].c_str();
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Edit selected above shader", ImVec2(220, 0)) && selectedShader != "")
		shadersEditorWindow.show(RESOURCE_MANAGER.getShader(selectedShader));*/

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	int iconsPerWindowWidth = (int)ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (64 + 32);
	if (!isOpenContextMenuInContentBrowser) shaderIDUnderMouse = "";
	ImGui::Columns(iconsPerWindowWidth, "mycolumns3", false);
	for (size_t i = 0; i < shaderList.size(); i++)
	{
		ImGui::PushID(i);
		FETexture* shaderPreviewTexture = RESOURCE_MANAGER.noTexture;
		if (ImGui::ImageButton((void*)(intptr_t)shaderPreviewTexture->getTextureID(), ImVec2(64, 64), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			shadersEditorWindow.show(RESOURCE_MANAGER.getShader(shaderList[i]));
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) shaderIDUnderMouse = shaderList[i];
		}

		ImGui::Text(RESOURCE_MANAGER.getShader(shaderList[i])->getName().c_str());
		ImGui::NextColumn();

		ImGui::PopID();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void FEEditor::displayMaterialContentBrowser()
{
	std::vector<std::string> materialList = RESOURCE_MANAGER.getMaterialList();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	int iconsPerWindowWidth = (int)ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (itemIconSide + 32);
	if (!isOpenContextMenuInContentBrowser) materialUnderMouse = -1;
	ImGui::Columns(iconsPerWindowWidth, "mycolumns3", false);
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FETexture* materialPreviewTexture = PREVIEW_MANAGER.getMaterialPreview(materialList[i]);

		if (ImGui::ImageButton((void*)(intptr_t)materialPreviewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			//if (!isOpenContextMenuInContentBrowser && !editMaterialWindow.isVisible())
			//{
				//materialUnderMouse = i;
				//editMaterialWindow.show(RESOURCE_MANAGER.getMaterial(RESOURCE_MANAGER.getMaterialList()[materialUnderMouse]));
			//}
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) materialUnderMouse = i;
		}

		if (ImGui::IsMouseDoubleClicked(0))
		{
			if (materialUnderMouse != -1 && !editMaterialWindow.isVisible())
				editMaterialWindow.show(RESOURCE_MANAGER.getMaterial(RESOURCE_MANAGER.getMaterialList()[materialUnderMouse]));
		}

		ImGui::Text(RESOURCE_MANAGER.getMaterial(materialList[i])->getName().c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void FEEditor::displayGameModelContentBrowser()
{
	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

	int iconsPerWindowWidth = (int)ImGui::GetCurrentContext()->CurrentWindow->Rect().GetWidth() / (itemIconSide + 32);
	if (!isOpenContextMenuInContentBrowser) gameModelUnderMouse = -1;
	ImGui::Columns(iconsPerWindowWidth, "mycolumns3", false);
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FETexture* gameModelPreviewTexture = PREVIEW_MANAGER.getGameModelPreview(gameModelList[i]);

		if (ImGui::ImageButton((void*)(intptr_t)gameModelPreviewTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			if (!isOpenContextMenuInContentBrowser && !editGameModelWindow.isVisible())
			{
				gameModelUnderMouse = i;
				editGameModelWindow.show(RESOURCE_MANAGER.getGameModel(RESOURCE_MANAGER.getGameModelList()[gameModelUnderMouse]));
			}
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) gameModelUnderMouse = i;
		}

		ImGui::Text(RESOURCE_MANAGER.getGameModel(gameModelList[i])->getName().c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

//void FEEditor::displayPostProcessContentBrowser()
//{
//	std::vector<std::string> postProcessList = RENDERER.getPostProcessList();
//
//	for (size_t i = 0; i < postProcessList.size(); i++)
//	{
//		FEPostProcess* PPEffect = RENDERER.getPostProcessEffect(postProcessList[i]);
//		if (ImGui::CollapsingHeader(PPEffect->getName().c_str(), 0)) //ImGuiTreeNodeFlags_DefaultOpen
//		{
//			for (size_t j = 0; j < PPEffect->stages.size(); j++)
//			{
//				ImGui::PushID(j);
//				std::vector<std::string> params = PPEffect->stages[j]->shader->getParameterList();
//				FEShaderParam* param;
//				for (size_t k = 0; k < params.size(); k++)
//				{
//					param = PPEffect->stages[j]->shader->getParameter(params[k]);
//					displayMaterialPrameter(param);
//				}
//
//				for (size_t k = 0; k < PPEffect->stages[j]->stageSpecificUniforms.size(); k++)
//				{
//					ImGui::PushID(j + k + 1);
//					param = &PPEffect->stages[j]->stageSpecificUniforms[k];
//					displayMaterialPrameter(param);
//					ImGui::PopID();
//				}
//
//				ImGui::PopID();
//			}
//		}
//	}
//}

void FEEditor::displayEffectsContentBrowser()
{
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
}