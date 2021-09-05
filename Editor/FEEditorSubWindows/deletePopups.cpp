#include "deletePopups.h"

deleteTexturePopup* deleteTexturePopup::_instance = nullptr;

deleteTexturePopup::deleteTexturePopup()
{
	popupCaption = "Delete texture";
	objToWorkWith = nullptr;
}

void deleteTexturePopup::show(FETexture* TextureToDelete)
{
	shouldOpen = true;
	objToWorkWith = TextureToDelete;
}

void deleteTexturePopup::render()
{
	ImGuiModalPopup::render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (objToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this texture is used in some materials
		// to-do: should be done through counter, not by searching each time.
		std::vector<FEMaterial*> materialsThatUseTexture = materialsThatUsesTexture(objToWorkWith);

		ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" texture ?").c_str());
		if (materialsThatUseTexture.size() > 0)
			ImGui::Text(("It is used in " + std::to_string(materialsThatUseTexture.size()) + " materials !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			deleteTexture(objToWorkWith);
			objToWorkWith = nullptr;
			ImGuiModalPopup::close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

std::vector<FEMaterial*> deleteTexturePopup::materialsThatUsesTexture(FETexture* texture)
{
	std::vector<FEMaterial*> result;
	std::vector<std::string> materiasList = RESOURCE_MANAGER.getMaterialList();

	for (size_t i = 0; i < materiasList.size(); i++)
	{
		FEMaterial* currentMaterial = RESOURCE_MANAGER.getMaterial(materiasList[i]);

		for (size_t j = 0; j < currentMaterial->textures.size(); j++)
		{
			if (currentMaterial->textures[j] == texture)
			{
				result.push_back(currentMaterial);
				break;
			}
		}
	}

	std::vector<std::string> terrainList = SCENE.getTerrainList();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* currentTerrain = SCENE.getTerrain(terrainList[i]);
		if (currentTerrain->heightMap != nullptr && currentTerrain->heightMap->getObjectID() == texture->getObjectID())
		{
			continue;
			result.push_back(nullptr);
		}
	}

	return result;
}

void deleteTexturePopup::deleteTexture(FETexture* texture)
{
	VIRTUAL_FILE_SYSTEM.locateAndDeleteFile(texture);

	// check if this texture is used in some materials
	// to-do: should be done through counter, not by searching each time.
	std::vector<FEMaterial*> materialsThatUseTexture = materialsThatUsesTexture(texture);
	std::vector<std::string> gameModelListToUpdate;

	std::string name = texture->getName();
	// re-create game model preview that was using material that uses this texture
	if (materialsThatUseTexture.size() > 0)
	{
		std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
		for (size_t i = 0; i < gameModelList.size(); i++)
		{
			FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);
			for (size_t j = 0; j < materialsThatUseTexture.size(); j++)
			{
				if (currentGameModel->material == materialsThatUseTexture[j])
					gameModelListToUpdate.push_back(currentGameModel->getObjectID());
			}
		}
	}

	texture->setDirtyFlag(true);
	PROJECT_MANAGER.getCurrent()->setModified(true);
	PROJECT_MANAGER.getCurrent()->addFileToDeleteList(PROJECT_MANAGER.getCurrent()->getProjectFolder() + texture->getObjectID() + ".texture");
	RESOURCE_MANAGER.deleteFETexture(texture);

	// re-create game model preview
	for (size_t i = 0; i < gameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.createGameModelPreview(gameModelListToUpdate[i]);
}

deleteMeshPopup* deleteMeshPopup::_instance = nullptr;

deleteMeshPopup::deleteMeshPopup()
{
	popupCaption = "Delete mesh";
	objToWorkWith = nullptr;
}

void deleteMeshPopup::show(FEMesh* MeshToDelete)
{
	shouldOpen = true;
	objToWorkWith = MeshToDelete;
}

void deleteMeshPopup::render()
{
	ImGuiModalPopup::render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (objToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this mesh is used in some game model
		// to-do: should be done through counter, not by searching each time.
		int result = timesMeshUsed(objToWorkWith);

		ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" mesh ?").c_str());
		if (result > 0)
			ImGui::Text(("It is used in " + std::to_string(result) + " game models !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			deleteMesh(objToWorkWith);
			objToWorkWith = nullptr;
			ImGuiModalPopup::close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

int deleteMeshPopup::timesMeshUsed(FEMesh* mesh)
{
	int result = 0;
	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();

	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);
		if (currentGameModel->mesh == mesh)
			result++;
	}

	return result;
}

void deleteMeshPopup::deleteMesh(FEMesh* mesh)
{
	VIRTUAL_FILE_SYSTEM.locateAndDeleteFile(mesh);

	std::string name = mesh->getName();

	// re-create game model preview
	std::vector<std::string> gameModelListToUpdate;
	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);
		if (currentGameModel->mesh == mesh)
			gameModelListToUpdate.push_back(currentGameModel->getObjectID());
	}

	mesh->setDirtyFlag(true);
	PROJECT_MANAGER.getCurrent()->setModified(true);
	PROJECT_MANAGER.getCurrent()->addFileToDeleteList(PROJECT_MANAGER.getCurrent()->getProjectFolder() + mesh->getObjectID() + ".model");
	RESOURCE_MANAGER.deleteFEMesh(mesh);

	// re-create game model preview
	for (size_t i = 0; i < gameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.createGameModelPreview(gameModelListToUpdate[i]);

	delete PREVIEW_MANAGER.meshPreviewTextures[name];
	PREVIEW_MANAGER.meshPreviewTextures.erase(name);
}

deleteGameModelPopup* deleteGameModelPopup::_instance = nullptr;

deleteGameModelPopup::deleteGameModelPopup()
{
	popupCaption = "Delete game model";
	objToWorkWith = nullptr;
}

void deleteGameModelPopup::show(FEGameModel* GameModel)
{
	shouldOpen = true;
	objToWorkWith = GameModel;
}

void deleteGameModelPopup::render()
{
	ImGuiModalPopup::render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (objToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this game model is used in some entities
		// to-do: should be done through counter, not by searching each time.
		int result = timesGameModelUsed(objToWorkWith);

		ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" game model ?").c_str());
		if (result > 0)
			ImGui::Text(("It is used in " + std::to_string(result) + " entities !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			deleteGameModel(objToWorkWith);

			objToWorkWith = nullptr;
			ImGuiModalPopup::close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

int deleteGameModelPopup::timesGameModelUsed(FEGameModel* gameModel)
{
	int result = 0;
	std::vector<std::string> entitiesList = SCENE.getEntityList();

	for (size_t i = 0; i < entitiesList.size(); i++)
	{
		FEEntity* currentEntity = SCENE.getEntity(entitiesList[i]);
		if (currentEntity->gameModel == gameModel)
			result++;
	}

	return result;
}

void deleteGameModelPopup::deleteGameModel(FEGameModel* gameModel)
{
	VIRTUAL_FILE_SYSTEM.locateAndDeleteFile(gameModel);

	std::string name = gameModel->getName();
	FEScene::getInstance().prepareForGameModelDeletion(gameModel);
	RESOURCE_MANAGER.deleteGameModel(gameModel);
	PROJECT_MANAGER.getCurrent()->setModified(true);
}

deleteMaterialPopup* deleteMaterialPopup::_instance = nullptr;

deleteMaterialPopup::deleteMaterialPopup()
{
	popupCaption = "Delete material";
	objToWorkWith = nullptr;
}

void deleteMaterialPopup::show(FEMaterial* material)
{
	shouldOpen = true;
	objToWorkWith = material;
}

void deleteMaterialPopup::render()
{
	ImGuiModalPopup::render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (objToWorkWith == nullptr)
		{
			ImGui::PopStyleVar();
			ImGuiModalPopup::close();
			return;
		}

		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		// check if this material is used in some game model
		// to-do: should be done through counter, not by searching each time.
		int result = timesMaterialUsed(objToWorkWith);

		ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" material ?").c_str());
		if (result > 0)
			ImGui::Text(("It is used in " + std::to_string(result) + " game models !").c_str());

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			deleteMaterial(objToWorkWith);

			objToWorkWith = nullptr;
			ImGuiModalPopup::close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

int deleteMaterialPopup::timesMaterialUsed(FEMaterial* material)
{
	int result = 0;
	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();

	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);
		if (currentGameModel->material == material)
			result++;
	}

	return result;
}

void deleteMaterialPopup::deleteMaterial(FEMaterial* material)
{
	VIRTUAL_FILE_SYSTEM.locateAndDeleteFile(material);

	std::string name = material->getName();
	// re-create game model preview
	std::vector<std::string> gameModelListToUpdate;
	std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
	for (size_t i = 0; i < gameModelList.size(); i++)
	{
		FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);
		if (currentGameModel->material == material)
			gameModelListToUpdate.push_back(currentGameModel->getObjectID());
	}

	material->setDirtyFlag(true);
	PROJECT_MANAGER.getCurrent()->setModified(true);
	RESOURCE_MANAGER.deleteMaterial(material);

	// re-create game model preview
	for (size_t i = 0; i < gameModelListToUpdate.size(); i++)
		PREVIEW_MANAGER.createGameModelPreview(gameModelListToUpdate[i]);

	delete PREVIEW_MANAGER.materialPreviewTextures[name];
	PREVIEW_MANAGER.materialPreviewTextures.erase(name);
}

deleteDirectoryPopup* deleteDirectoryPopup::_instance = nullptr;

deleteDirectoryPopup::deleteDirectoryPopup()
{
	popupCaption = "Delete folder";
	objToWorkWith = "";
}

void deleteDirectoryPopup::show(std::string directoryName)
{
	shouldOpen = true;
	objToWorkWith = directoryName;

	pathToDirectory = VIRTUAL_FILE_SYSTEM.getCurrentPath();
	if (pathToDirectory.back() != '/')
		pathToDirectory += '/';

	pathToDirectory += directoryName;
}

void deleteDirectoryPopup::recursiveDeletion(std::string path)
{
	auto content = VIRTUAL_FILE_SYSTEM.getDirectoryContent(path);
	for (size_t i = 0; i < content.size(); i++)
	{
		if (content[i]->getType() == FE_NULL)
		{
			std::string tempPath = path;
			if (tempPath.back() != '/')
				tempPath += '/';

			tempPath += content[i]->getName();
			recursiveDeletion(tempPath);
		}
		else if (content[i]->getType() == FE_SHADER)
		{
			//RESOURCE_MANAGER.deleteShader(RESOURCE_MANAGER.getShader(content[i]->getObjectID()));
		}
		else if (content[i]->getType() == FE_MESH)
		{
			deleteMeshPopup::deleteMesh(RESOURCE_MANAGER.getMesh(content[i]->getObjectID()));
		}
		else if (content[i]->getType() == FE_TEXTURE)
		{
			deleteTexturePopup::deleteTexture(RESOURCE_MANAGER.getTexture(content[i]->getObjectID()));
		}
		else if (content[i]->getType() == FE_MATERIAL)
		{
			deleteMaterialPopup::deleteMaterial(RESOURCE_MANAGER.getMaterial(content[i]->getObjectID()));
		}
		else if (content[i]->getType() == FE_GAMEMODEL)
		{
			deleteGameModelPopup::deleteGameModel(RESOURCE_MANAGER.getGameModel(content[i]->getObjectID()));
		}
	}

	VIRTUAL_FILE_SYSTEM.deleteEmptyDirectory(path);
}

void deleteDirectoryPopup::render()
{
	ImGuiModalPopup::render();

	if (pathToDirectory != "" && VIRTUAL_FILE_SYSTEM.getDirectoryContent(pathToDirectory).size() == 0)
	{
		VIRTUAL_FILE_SYSTEM.deleteEmptyDirectory(pathToDirectory);
		objToWorkWith = "";
		return;
		ImGuiModalPopup::close();
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text(("Do you want to delete \"" + objToWorkWith + "\" folder ?").c_str());
		ImGui::Text("It is not empty !");

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 4.0f - 120 / 2.0f);
		if (ImGui::Button("Delete", ImVec2(120, 0)))
		{
			recursiveDeletion(pathToDirectory);
			//VIRTUAL_FILE_SYSTEM.deleteDirectory(pathToDirectory);
			PROJECT_MANAGER.getCurrent()->setModified(true);

			// I should do it in a way as windows FS is doing it.
			// You can't delete non empty folder
			// Firstly you recursively have to delete all files and folders in it


			//PREVIEW_MANAGER. recreateAll ?
			//FEScene::getInstance().prepareForGameModelDeletion(objToWorkWith);
			//PROJECT_MANAGER.getCurrent()->addFileToDeleteList(PROJECT_MANAGER.getCurrent()->getProjectFolder() + objToWorkWith->getObjectID() + ".model");

			objToWorkWith = "";
			ImGuiModalPopup::close();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 4.0f - 120.0f / 2.0f);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGuiModalPopup::close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}