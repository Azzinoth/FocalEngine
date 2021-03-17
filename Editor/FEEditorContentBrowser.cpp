#include "../Editor/FEEditor.h"

void FEEditor::displayContentBrowser()
{
	float mainWindowW = float(ENGINE.getWindowWidth());
	float mainWindowH = float(ENGINE.getWindowHeight());
	//#fix this non-sence with proper Imgui docking system 
	float windowW = mainWindowW / 3.7f;
	if (windowW > 600)
		windowW = 600;
	float windowH = mainWindowH;

	ImGui::SetNextWindowPos(ImVec2(mainWindowW - windowW, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(windowW, windowH));
	ImGui::Begin("Content Browser", nullptr, ImGuiWindowFlags_None);
	if (ImGui::BeginTabBar("##Content Browser", ImGuiTabBarFlags_None))
	{
		ImGui::PushStyleColor(ImGuiCol_TabActive, (ImVec4)ImColor::ImColor(0.4f, 0.9f, 0.4f, 1.0f));

		if (SELECTED.getDirtyFlag() && SELECTED.getTerrain() != nullptr)
		{
			auto& ImGuiContext = *ImGui::GetCurrentContext();
			ImGuiContext.CurrentTabBar->SelectedTabId = ImGuiContext.CurrentTabBar->Tabs[5].ID;
			SELECTED.setDirtyFlag(false);
		}

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

		if (ImGui::BeginTabItem("Terrain"))
		{
			activeTabContentBrowser = 5;
			displayTerrainContentBrowser();
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
					renameMeshWindow.show(RESOURCE_MANAGER.getMesh(RESOURCE_MANAGER.getMeshList()[meshUnderMouse]));
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
					renameTextureWindow.show(RESOURCE_MANAGER.getTexture(RESOURCE_MANAGER.getTextureList()[textureUnderMouse]));
				}
			}
			break;
		}
		case 2:
		{
			if (shaderNameUnderMouse != "")
			{
				if (ImGui::MenuItem("Edit..."))
				{
					shadersEditorWindow.show(RESOURCE_MANAGER.getShader(shaderNameUnderMouse));
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
						newMat->shader = RESOURCE_MANAGER.getShader("FEPBRShader");

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
					renameMaterialWindow.show(RESOURCE_MANAGER.getMaterial(RESOURCE_MANAGER.getMaterialList()[materialUnderMouse]));
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
					renameGameModelWindow.show(RESOURCE_MANAGER.getGameModel(RESOURCE_MANAGER.getGameModelList()[gameModelUnderMouse]));
				}
			}
			break;
		}
		case 5:
		{
			if (ImGui::MenuItem("Create new terrain"))
			{
				FETerrain* newTerrain = RESOURCE_MANAGER.createTerrain();
				SCENE.addTerrain(newTerrain);
				newTerrain->heightMap->setDirtyFlag(true);
				PROJECT_MANAGER.getCurrent()->modified = true;
			}
			break;
		}
		case 6:
		{
			break;
		}
		default:
			break;
		}
		ImGui::EndPopup();
	}
	ImGui::End();

	loadTextureWindow.render();
	renameMeshWindow.render();
	renameTextureWindow.render();
	renameFailedWindow.render();
	deleteTextureWindow.render();
	deleteMeshWindow.render();
	renameGameModelWindow.render();
	deleteGameModelWindow.render();
	editGameModelWindow.render();
	editMaterialWindow.render();
	renameMaterialWindow.render();
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

	if (!isOpenContextMenuInContentBrowser) meshUnderMouse = -1;
	ImGui::Columns(3, "mycolumns3", false);
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

		ImGui::Text(meshList[i].c_str());
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

	if (!isOpenContextMenuInContentBrowser) textureUnderMouse = -1;
	ImGui::Columns(3, "mycolumns3", false);
	for (size_t i = 0; i < textureList.size(); i++)
	{
		if (ImGui::ImageButton((void*)(intptr_t)RESOURCE_MANAGER.getTexture(textureList[i])->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
		{
			//
		}

		if (ImGui::IsItemHovered())
		{
			if (!isOpenContextMenuInContentBrowser) textureUnderMouse = i;
		}

		ImGui::Text(textureList[i].c_str());
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

	if (!isOpenContextMenuInContentBrowser) shaderNameUnderMouse = "";
	ImGui::Columns(3, "mycolumns3", false);
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
			if (!isOpenContextMenuInContentBrowser) shaderNameUnderMouse = shaderList[i];
		}

		ImGui::Text(shaderList[i].c_str());
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

	if (!isOpenContextMenuInContentBrowser) materialUnderMouse = -1;
	ImGui::Columns(3, "mycolumns3", false);
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

		ImGui::Text(materialList[i].c_str());
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

	if (!isOpenContextMenuInContentBrowser) gameModelUnderMouse = -1;
	ImGui::Columns(3, "mycolumns3", false);
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

		ImGui::Text(gameModelList[i].c_str());
		ImGui::NextColumn();
	}
	ImGui::Columns(1);

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void FEEditor::displayTerrainContentBrowser()
{
	static ImGuiButton* changeMaterialButton = new ImGuiButton("Change Material");
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
		changeMaterialButton->setSize(ImVec2(180, 0));

		firstCall = false;
	}

	std::vector<std::string> terrainList = SCENE.getTerrainList();
	if (SELECTED.getDirtyFlag() && SELECTED.getTerrain() != nullptr)
	{
		for (size_t i = 0; i < terrainList.size(); i++)
		{
			ImGui::GetStateStorage()->SetInt(ImGui::GetID(terrainList[i].c_str()), 0);
		}
		SELECTED.setDirtyFlag(false);
		ImGui::GetStateStorage()->SetInt(ImGui::GetID(SELECTED.getTerrainName().c_str()), 1);
	}

	static int testCount = 0;

	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* currentTerrain = SCENE.getTerrain(terrainList[i]);
		if (ImGui::TreeNode(currentTerrain->getName().c_str()))
		{
			//testButton->render();
			//if (testButton->getWasClicked())
			//{
			//	int count_mult = 40;
			//	FEEntityInstanced* testInstanced;

			//	// ************** grass_simple_01 **************
			//	FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel("grass_simple_01");
			//	testInstanced = SCENE.addEntityInstanced(currentGameModel, "firstInstanced");
			//	testInstanced->cullingType = FE_CULLING_LODS;

			//	FESpawnInfo spawnInfo;
			//	spawnInfo.objectCount = 1500 * count_mult;
			//	spawnInfo.radius = 200.0f;
			//	spawnInfo.seed = 8774;
			//	spawnInfo.rotationDeviation = glm::vec3(0.01f, 1.0f, 0.01f);

			//	SCENE.getTerrain(SCENE.getTerrainList()[0])->snapInstancedEntity(testInstanced);
			//	testInstanced->populate(spawnInfo);

			//	// ************** grass_simple_02 **************
			//	currentGameModel = RESOURCE_MANAGER.getGameModel("grass_simple_02");
			//	testInstanced = SCENE.addEntityInstanced(currentGameModel, "secondInstanced");
			//	testInstanced->cullingType = FE_CULLING_LODS;

			//	spawnInfo.objectCount = 300 * count_mult;
			//	spawnInfo.radius = 200.0f;
			//	spawnInfo.seed = 234234;
			//	spawnInfo.rotationDeviation = glm::vec3(0.01f, 1.0f, 0.01f);

			//	SCENE.getTerrain(SCENE.getTerrainList()[0])->snapInstancedEntity(testInstanced);
			//	testInstanced->populate(spawnInfo);

			//	// ************** perennials **************
			//	currentGameModel = RESOURCE_MANAGER.getGameModel("perennials");
			//	testInstanced = SCENE.addEntityInstanced(currentGameModel, "second_Instanced");
			//	testInstanced->cullingType = FE_CULLING_LODS;

			//	spawnInfo.objectCount = 30 * count_mult;
			//	spawnInfo.radius = 200.0f;
			//	spawnInfo.seed = 1300;
			//	spawnInfo.rotationDeviation = glm::vec3(0.01f, 1.0f, 0.01f);

			//	SCENE.getTerrain(SCENE.getTerrainList()[0])->snapInstancedEntity(testInstanced);
			//	testInstanced->populate(spawnInfo);
			//	
			//	// ************** SM_forest_heather_01 **************
			//	currentGameModel = RESOURCE_MANAGER.getGameModel("SM_forest_heather_01");
			//	testInstanced = SCENE.addEntityInstanced(currentGameModel, "thirdInstanced");
			//	testInstanced->cullingType = FE_CULLING_LODS;

			//	spawnInfo.objectCount = 300 * count_mult;
			//	spawnInfo.radius = 200.0f;
			//	spawnInfo.seed = 11500;
			//	spawnInfo.rotationDeviation = glm::vec3(0.01f, 1.0f, 0.01f);

			//	SCENE.getTerrain(SCENE.getTerrainList()[0])->snapInstancedEntity(testInstanced);
			//	testInstanced->populate(spawnInfo);
			//	
			//	// ************** SM_Small_Rock_05 **************
			//	currentGameModel = RESOURCE_MANAGER.getGameModel("SM_Small_Rock_05");
			//	testInstanced = SCENE.addEntityInstanced(currentGameModel, "fourthInstanced");
			//	testInstanced->cullingType = FE_CULLING_LODS;

			//	spawnInfo.objectCount = 25 * count_mult;
			//	spawnInfo.radius = 200.0f;
			//	spawnInfo.seed = -11500;
			//	spawnInfo.rotationDeviation = glm::vec3(0.1f, 1.0f, 0.1f);

			//	SCENE.getTerrain(SCENE.getTerrainList()[0])->snapInstancedEntity(testInstanced);
			//	testInstanced->populate(spawnInfo);

			//	// ************** Fir_05_LOD0 **************
			//	currentGameModel = RESOURCE_MANAGER.getGameModel("Fir_05_LOD0");
			//	currentGameModel->material->setRoughtnessMapIntensity(1.5);
			//	testInstanced = SCENE.addEntityInstanced(currentGameModel, "fourthInstanced1");
			//	testInstanced->cullingType = FE_CULLING_LODS;

			//	spawnInfo.objectCount = 600;
			//	spawnInfo.radius = 200.0f;
			//	spawnInfo.seed = -1500;
			//	spawnInfo.rotationDeviation = glm::vec3(0.005f, 1.0f, 0.005f);

			//	SCENE.getTerrain(SCENE.getTerrainList()[0])->snapInstancedEntity(testInstanced);
			//	testInstanced->populate(spawnInfo);

			//	// ************** amurcork **************
			//	currentGameModel = RESOURCE_MANAGER.getGameModel("amurcork");
			//	currentGameModel->material->setRoughtnessMapIntensity(1.5);
			//	testInstanced = SCENE.addEntityInstanced(currentGameModel, "fourthInstanced2");
			//	testInstanced->cullingType = FE_CULLING_LODS;

			//	spawnInfo.objectCount = 100;
			//	spawnInfo.radius = 200.0f;
			//	spawnInfo.seed = -6791500;

			//	testInstanced->populate(spawnInfo);
			//	testInstanced->clear();
			//	SCENE.getTerrain(SCENE.getTerrainList()[0])->snapInstancedEntity(testInstanced);
			//	testInstanced->populate(spawnInfo);
			//}

			//ImGui::Text((std::string("was spawned : ") + std::to_string(testCount)).c_str());

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
						LOG.logError(std::string("can't load height map: ") + filePath);
					}
					else
					{
						loadedTexture->setDirtyFlag(true);
						PROJECT_MANAGER.getCurrent()->modified = true;
						//RESOURCE_MANAGER.saveFETexture(loadedTexture, (PROJECT_MANAGER.getCurrent()->getProjectFolder() + loadedTexture->getName() + ".FETexture").c_str());
					}
				}
			}

			int iData = *(int*)currentTerrain->shader->getParameter("debugFlag")->data;
			ImGui::SliderInt("debugFlag", &iData, 0, 10);
			currentTerrain->shader->getParameter("debugFlag")->updateData(iData);

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

			changeMaterialButton->render();
			if (changeMaterialButton->getWasClicked())
			{
				selectMaterialWindow.show(&currentTerrain->layer0);
			}

			showTransformConfiguration(terrainList[i], &currentTerrain->transform);

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
					SELECTED.setTerrain(SELECTED.getTerrain());

				currentTerrain->setBrushActive(EDITOR.leftMousePressed);
				currentTerrain->setBrushInversed(EDITOR.shiftPressed);
			}
			else
			{
				// to show gizmos
				if (SELECTED.getTerrain() != nullptr)
					SELECTED.setTerrain(SELECTED.getTerrain());
			}

			ImGui::TreePop();
		}
	}

	selectMaterialWindow.render();
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