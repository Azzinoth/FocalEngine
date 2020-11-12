#pragma once

//#include "../FEEditorInternalResources.h"
#include "../FEProject.h"

class deleteTexturePopup : public ImGuiModalPopup
{
	FETexture* objToWorkWith;
	std::vector<FEMaterial*> materialsThatUseTexture;
public:

	std::vector<FEMaterial*> materialsThatUsesTexture(FETexture* texture)
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

		std::vector<std::string> terrainList = RESOURCE_MANAGER.getTerrainList();
		for (size_t i = 0; i < terrainList.size(); i++)
		{
			FETerrain* currentTerrain = RESOURCE_MANAGER.getTerrain(terrainList[i]);

			if (currentTerrain->heightMap != nullptr && currentTerrain->heightMap->getName() == texture->getName())
			{
				continue;
				result.push_back(nullptr);
			}
		}

		return result;
	}

	deleteTexturePopup()
	{
		popupCaption = "Delete texture";
		objToWorkWith = nullptr;
	}

	void show(FETexture* TextureToDelete)
	{
		shouldOpen = true;
		objToWorkWith = TextureToDelete;

		// check if this texture is used in some materials
		// to-do: should be done through counter, not by searching each time.
		materialsThatUseTexture = materialsThatUsesTexture(objToWorkWith);
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGuiModalPopup::close();
				return;
			}

			ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" texture ?").c_str());
			if (materialsThatUseTexture.size() > 0)
				ImGui::Text(("It is used in " + std::to_string(materialsThatUseTexture.size()) + " materials !").c_str());

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120 / 2.0f);
			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				std::vector<std::string> gameModelListToUpdate;

				std::string name = objToWorkWith->getName();
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
								gameModelListToUpdate.push_back(currentGameModel->getName());
						}
					}
				}

				objToWorkWith->setDirtyFlag(true);
				PROJECT_MANAGER.getCurrent()->modified = true;
				PROJECT_MANAGER.getCurrent()->addFileToDeleteList(PROJECT_MANAGER.getCurrent()->getProjectFolder() + objToWorkWith->getAssetID() + ".texture");
				RESOURCE_MANAGER.deleteFETexture(objToWorkWith);

				// re-create game model preview
				for (size_t i = 0; i < gameModelListToUpdate.size(); i++)
					PREVIEW_MANAGER.createGameModelPreview(gameModelListToUpdate[i]);

				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static deleteTexturePopup deleteTextureWindow;

class deleteMeshPopup : public ImGuiModalPopup
{
	FEMesh* objToWorkWith;
public:

	int timesMeshUsed(FEMesh* mesh)
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

	deleteMeshPopup()
	{
		popupCaption = "Delete mesh";
		objToWorkWith = nullptr;
	}

	void show(FEMesh* MeshToDelete)
	{
		shouldOpen = true;
		objToWorkWith = MeshToDelete;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGuiModalPopup::close();
				return;
			}

			// check if this mesh is used in some game model
			// to-do: should be done through counter, not by searching each time.
			int result = timesMeshUsed(objToWorkWith);

			ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" mesh ?").c_str());
			if (result > 0)
				ImGui::Text(("It is used in " + std::to_string(result) + " game models !").c_str());

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120 / 2.0f);
			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				std::string name = objToWorkWith->getName();

				// re-create game model preview
				std::vector<std::string> gameModelListToUpdate;
				std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
				for (size_t i = 0; i < gameModelList.size(); i++)
				{
					FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);

					if (currentGameModel->mesh == objToWorkWith)
						gameModelListToUpdate.push_back(currentGameModel->getName());
				}

				objToWorkWith->setDirtyFlag(true);
				PROJECT_MANAGER.getCurrent()->modified = true;
				PROJECT_MANAGER.getCurrent()->addFileToDeleteList(PROJECT_MANAGER.getCurrent()->getProjectFolder() + objToWorkWith->getAssetID() + ".model");
				RESOURCE_MANAGER.deleteFEMesh(objToWorkWith);

				// re-create game model preview
				for (size_t i = 0; i < gameModelListToUpdate.size(); i++)
					PREVIEW_MANAGER.createGameModelPreview(gameModelListToUpdate[i]);


				delete PREVIEW_MANAGER.meshPreviewTextures[name];
				PREVIEW_MANAGER.meshPreviewTextures.erase(name);
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static deleteMeshPopup deleteMeshWindow;

class deleteGameModelPopup : public ImGuiModalPopup
{
	FEGameModel* objToWorkWith;
public:

	int timesGameModelUsed(FEGameModel* gameModel)
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

	deleteGameModelPopup()
	{
		popupCaption = "Delete game model";
		objToWorkWith = nullptr;
	}

	void show(FEGameModel* GameModel)
	{
		shouldOpen = true;
		objToWorkWith = GameModel;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGuiModalPopup::close();
				return;
			}

			// check if this game model is used in some entities
			// to-do: should be done through counter, not by searching each time.
			int result = timesGameModelUsed(objToWorkWith);

			ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" game model ?").c_str());
			if (result > 0)
				ImGui::Text(("It is used in " + std::to_string(result) + " entities !").c_str());

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				std::string name = objToWorkWith->getName();
				FEScene::getInstance().prepareForGameModelDeletion(objToWorkWith);
				RESOURCE_MANAGER.deleteGameModel(objToWorkWith);
				PROJECT_MANAGER.getCurrent()->modified = true;

				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static deleteGameModelPopup deleteGameModelWindow;

class deleteMaterialPopup : public ImGuiModalPopup
{
	FEMaterial* objToWorkWith;
public:

	int timesMaterialUsed(FEMaterial* material)
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

	deleteMaterialPopup()
	{
		popupCaption = "Delete material";
		objToWorkWith = nullptr;
	}

	void show(FEMaterial* material)
	{
		shouldOpen = true;
		objToWorkWith = material;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGuiModalPopup::close();
				return;
			}

			// check if this material is used in some game model
			// to-do: should be done through counter, not by searching each time.
			int result = timesMaterialUsed(objToWorkWith);

			ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" material ?").c_str());
			if (result > 0)
				ImGui::Text(("It is used in " + std::to_string(result) + " game models !").c_str());

			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 4.0f - 120 / 2.0f);
			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				std::string name = objToWorkWith->getName();

				// re-create game model preview
				std::vector<std::string> gameModelListToUpdate;
				std::vector<std::string> gameModelList = RESOURCE_MANAGER.getGameModelList();
				for (size_t i = 0; i < gameModelList.size(); i++)
				{
					FEGameModel* currentGameModel = RESOURCE_MANAGER.getGameModel(gameModelList[i]);

					if (currentGameModel->material == objToWorkWith)
						gameModelListToUpdate.push_back(currentGameModel->getName());
				}

				objToWorkWith->setDirtyFlag(true);
				PROJECT_MANAGER.getCurrent()->modified = true;
				RESOURCE_MANAGER.deleteMaterial(objToWorkWith);

				// re-create game model preview
				for (size_t i = 0; i < gameModelListToUpdate.size(); i++)
					PREVIEW_MANAGER.createGameModelPreview(gameModelListToUpdate[i]);


				delete PREVIEW_MANAGER.materialPreviewTextures[name];
				PREVIEW_MANAGER.materialPreviewTextures.erase(name);
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() / 2.0f + ImGui::GetWindowContentRegionWidth() / 4.0f - 120.0f / 2.0f);
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static deleteMaterialPopup deleteMaterialWindow;