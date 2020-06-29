#pragma once

#include "FEProject.h"
#include "../Editor/FEPixelAccurateSelectionShader.h"
#include "../Editor/FEMeshPreviewShader.h"
#ifdef FE_WIN_32
	#include <direct.h> // file system
	#include <shobjidl.h> // openDialog
#endif

using namespace FocalEngine;

#define PROJECTS_FOLDER "C:/Users/kandr/Downloads/FEProjects"

static double mouseX, mouseY;
static bool isCameraInputActive = false;
// **************************** entity selection ****************************
glm::dvec3 mouseRay();

void determineEntityUnderMouse();
static std::vector<std::string> entitiesUnderMouse;
static std::string selectedEntity = "";
std::string getSelectedEntity();
void setSelectedEntity(std::string newEntity);
static bool selectedEntityWasChanged;

static std::string clipboardEntity = "";

// pixel accurate part
static bool checkPixelAccurateSelection = false;
static unsigned char* colorUnderMouse = new unsigned char[3];
static FocalEngine::FEFramebuffer* pixelAccurateSelectionFB;
static FocalEngine::FEEntity* potentiallySelectedEntity = nullptr;
static FocalEngine::FEMaterial* pixelAccurateSelectionMaterial;
// **************************** entity selection END ****************************

static std::vector<FEProject*> projectList;
static int projectChosen = -1;

static FEProject* currentProject = nullptr;

void mouseButtonCallback(int button, int action, int mods);

void mouseMoveCallback(double xpos, double ypos);
void keyButtonCallback(int key, int scancode, int action, int mods);

void toolTip(const char* text);
void showPosition(std::string objectName, glm::vec3& position);
void showRotation(std::string objectName, glm::vec3& rotation);
void showScale(std::string objectName, glm::vec3& scale);
void showTransformConfiguration(std::string objectName, FETransformComponent* transform);

void displayMaterialPrameter(FEShaderParam* param);
void displayMaterialPrameters(FEMaterial* material);
void displayLightProperties(FELight* light);
void displayLightsProperties();
void displaySceneEntities();

// **************************** General preview variables ****************************
static FocalEngine::FEFramebuffer* previewFB;
static FocalEngine::FEEntity* previewEntity;
static FocalEngine::FEGameModel* previewGameModel;
// **************************** General preview variables END ****************************

// **************************** Material Content Browser ****************************
static std::unordered_map<std::string, FETexture*> materialPreviewTextures;

void createMaterialPreview(std::string materialName);
FETexture* getMaterialPreview(std::string materialName);

void displayMaterialContentBrowser();
// **************************** Material Content Browser END ****************************

// **************************** Meshes Content Browser ****************************
static std::unordered_map<std::string, FETexture*> meshPreviewTextures;
static FocalEngine::FEMaterial* meshPreviewMaterial;

void createMeshPreview(std::string meshName);
FETexture* getMeshPreview(std::string meshName);

void displayMeshesContentBrowser();
// **************************** Meshes Content Browser END ****************************
void displayTexturesContentBrowser();
void displayTextureInMaterialEditor(FETexture*& texture);

void displayGameModelContentBrowser();
static std::unordered_map<std::string, FETexture*> gameModelPreviewTextures;
void createGameModelPreview(std::string gameModelName);
void createGameModelPreview(FEGameModel* gameModel, FETexture** resultingTexture);
FETexture* getGameModelPreview(std::string gameModelName);

void displayContentBrowser();
void displayPostProcessContentBrowser();
void displayProjectSelection();
void openProject(int projectIndex);

void addEntityButton();

// file system
bool checkFolder(const char* dirPath);
bool createFolder(const char* dirPath);
bool deleteFolder(const char* dirPath);
std::vector<std::string> getFolderList(const char* dirPath);
bool changeFileName(const char* filePath, const char* newName);
bool deleteFile(const char* filePath);
// file system END

void renderEditor();
void loadProjectList();
void loadEditor();

int timesTextureUsed(FETexture* texture);
int timesMeshUsed(FEMesh* mesh);

#ifdef FE_WIN_32
	// open dialog staff
	const COMDLG_FILTERSPEC meshLoadFilter[] =
	{
		{ L"Wavefront OBJ files (*.obj)", L"*.obj" },
	};

	const COMDLG_FILTERSPEC textureLoadFilter[] =
	{
		{ L"PNG files (*.png)", L"*.png" },
	};

	std::string toString(PWSTR wString);
	void openDialog(std::string& filePath, const COMDLG_FILTERSPEC* filter, int filterCount = 1);

#endif


class ImGuiModalPopup
{
protected:
	bool shouldOpen;
	bool opened;
	char* popupCaption;

	virtual void close()
	{
		opened = false;
		ImGui::CloseCurrentPopup();
	}
public:
	ImGuiModalPopup()
	{
		popupCaption = "";
		shouldOpen = false;
		opened = false;
	}

	virtual void show()
	{
		shouldOpen = true;
		opened = true;
	}

	virtual void render()
	{
		if (shouldOpen)
		{
			ImGui::OpenPopup(popupCaption);
			shouldOpen = false;
		}
	}

	bool isOpened()
	{
		return opened;
	}
};

class deleteTexturePopup : public ImGuiModalPopup
{
	FETexture* objToWorkWith;
public:
	deleteTexturePopup()
	{
		popupCaption = "Delete texture";
		objToWorkWith = nullptr;
	}

	void show(FETexture* TextureToDelete)
	{
		shouldOpen = true;
		objToWorkWith = TextureToDelete;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGuiModalPopup::close();
				return;
			}

			// check if this texture is used in some materials
			// to-do: should be done through counter, not by searching each time.
			FEResourceManager& resourceManager = FEResourceManager::getInstance();
			int result = timesTextureUsed(objToWorkWith);
			
			ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" texture ?").c_str());
			if (result > 0)
				ImGui::Text(("It is used in " + std::to_string(result) + " materials !").c_str());

			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				std::string name = objToWorkWith->getName();
				FEResourceManager::getInstance().deleteFETexture(objToWorkWith);
				currentProject->saveScene();

				deleteFile((currentProject->getProjectFolder() + name + ".FEtexture").c_str());

				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static deleteTexturePopup deleteTextureWindow;

class renameFailedPopUp : public ImGuiModalPopup
{
public:
	renameFailedPopUp()
	{
		popupCaption = "Invalid name";
	}

	void show()
	{
		shouldOpen = true;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Entered name is occupied");

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			ImGui::SetCursorPosX(33);
			ImGui::SetCursorPosY(50);
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::EndPopup();
		}
	}
};
static renameFailedPopUp renameFailedWindow;

class renameTexturePopUp : public ImGuiModalPopup
{
	FETexture* objToWorkWith;
	char newName[512];
public:
	renameTexturePopUp()
	{
		popupCaption = "Rename Texture";
		objToWorkWith = nullptr;
	}

	void show(FETexture* TextureToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = TextureToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGui::EndPopup();
				return;
			}

			FEResourceManager& resourceManager = FEResourceManager::getInstance();
			ImGui::Text("New texture name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();

			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (resourceManager.setTextureName(objToWorkWith, newName))
				{
					// also rename texture filename correspondently
					changeFileName((currentProject->getProjectFolder() + oldName + ".FETexture").c_str(), (currentProject->getProjectFolder() + newName + ".FETexture").c_str());
					// save assets list with new texture name
					currentProject->saveScene();

					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameTexturePopUp renameTextureWindow;

class loadTexturePopUp : public ImGuiModalPopup
{
	std::string filePath;
public:
	loadTexturePopUp()
	{
		popupCaption = "Choose Texture Type";
	}

	void show(std::string FilePath)
	{
		shouldOpen = true;
		filePath = FilePath;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Does this texture uses alpha channel ?");

			if (ImGui::Button("Yes", ImVec2(120, 0)))
			{
				FETexture* newTexture = FEResourceManager::getInstance().LoadPngTextureAndCompress(filePath.c_str(), true);
				FEResourceManager::getInstance().saveFETexture((currentProject->getProjectFolder() + newTexture->getName() + ".FETexture").c_str(), newTexture);
				// add asset list saving....
				currentProject->saveScene();

				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0)))
			{
				FETexture* newTexture = FEResourceManager::getInstance().LoadPngTextureAndCompress(filePath.c_str(), false);
				FEResourceManager::getInstance().saveFETexture((currentProject->getProjectFolder() + newTexture->getName() + ".FETexture").c_str(), newTexture);
				// add asset list saving....
				currentProject->saveScene();

				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static loadTexturePopUp loadTextureWindow;

class selectTexturePopUp : public ImGuiModalPopup
{
	FETexture** objToWorkWith;
	int textureIndexUnderMouse = -1;
	int textureIndexSelected = -1;
	bool pushedStyle = false;
	std::vector<std::string> textureList;
	std::vector<std::string> filteredTextureList;
	char filter[512];
public:
	selectTexturePopUp()
	{
		popupCaption = "Select texture";
	}

	void show(FETexture** texture)
	{
		shouldOpen = true;
		pushedStyle = false;
		objToWorkWith = texture;
		textureList = FEResourceManager::getInstance().getTextureList();
		filteredTextureList = textureList;
		strcpy_s(filter, "");
	}

	void close() override
	{
		ImGuiModalPopup::close();
		textureIndexUnderMouse = -1;
		textureIndexSelected = -1;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredTextureList = textureList;
				}
				else
				{
					filteredTextureList.clear();
					for (size_t i = 0; i < textureList.size(); i++)
					{
						if (textureList[i].find(filter) != -1)
						{
							filteredTextureList.push_back(textureList[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectTexturePopupColumns", false);
			for (size_t i = 0; i < filteredTextureList.size(); i++)
			{
				ImGui::PushID(filteredTextureList[i].c_str());
				pushedStyle = false;
				if (textureIndexSelected == i)
				{
					pushedStyle = true;
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				}

				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (textureIndexUnderMouse != -1)
					{
						*objToWorkWith = FEResourceManager::getInstance().getTexture(filteredTextureList[textureIndexUnderMouse]);
						close();
					}
				}

				if (ImGui::ImageButton((void*)(intptr_t)FEResourceManager::getInstance().getTexture(filteredTextureList[i])->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					textureIndexSelected = i;
				}

				if (pushedStyle)
				{
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}

				if (ImGui::IsItemHovered())
				{
					textureIndexUnderMouse = i;
				}

				ImGui::Text(filteredTextureList[i].c_str());
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::SetCursorPosX(300);
			ImGui::SetCursorPosY(25);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			if (ImGui::Button("Select", ImVec2(140, 24)))
			{
				if (textureIndexSelected != -1)
				{
					*objToWorkWith = FEResourceManager::getInstance().getTexture(filteredTextureList[textureIndexSelected]);
					close();
				}
			}

			ImGui::SetCursorPosX(460);
			ImGui::SetCursorPosY(25);

			if (ImGui::Button("Cancel", ImVec2(140, 24)))
			{
				close();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::EndPopup();
		}
	}
};
static selectTexturePopUp selectTextureWindow;

class renameMeshPopUp : public ImGuiModalPopup
{
	FEMesh* objToWorkWith;
	char newName[512];
public:
	renameMeshPopUp()
	{
		popupCaption = "Rename Mesh";
		objToWorkWith = nullptr;
	}

	void show(FEMesh* MeshToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = MeshToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGui::EndPopup();
				return;
			}

			FEResourceManager& resourceManager = FEResourceManager::getInstance();
			ImGui::Text("New mesh name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();

			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (resourceManager.setMeshName(objToWorkWith, newName))
				{
					// also rename mesh filename correspondently
					changeFileName((currentProject->getProjectFolder() + oldName + ".model").c_str(), (currentProject->getProjectFolder() + newName + ".model").c_str());
					// save assets list with new mesh name
					currentProject->saveScene();

					FETexture* tempTexture = meshPreviewTextures[oldName];
					meshPreviewTextures.erase(oldName);
					meshPreviewTextures[newName] = tempTexture;

					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameMeshPopUp renameMeshWindow;

class deleteMeshPopup : public ImGuiModalPopup
{
	FEMesh* objToWorkWith;
public:
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

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGuiModalPopup::close();
				return;
			}

			// check if this mesh is used in some entity
			// to-do: should be done through counter, not by searching each time.
			FEResourceManager& resourceManager = FEResourceManager::getInstance();
			int result = timesMeshUsed(objToWorkWith);

			ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" mesh ?").c_str());
			if (result > 0)
				ImGui::Text(("It is used in " + std::to_string(result) + " game models !").c_str());

			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				std::string name = objToWorkWith->getName();

				// re-create game model preview
				std::vector<std::string> gameModelListToUpdate;
				std::vector<std::string> gameModelList = FEResourceManager::getInstance().getGameModelList();
				for (size_t i = 0; i < gameModelList.size(); i++)
				{
					FEGameModel* currentGameModel = FEResourceManager::getInstance().getGameModel(gameModelList[i]);

					if (currentGameModel->mesh == objToWorkWith)
						gameModelListToUpdate.push_back(currentGameModel->getName());
				}

				FEResourceManager::getInstance().deleteFEMesh(objToWorkWith);
				currentProject->saveScene();

				// re-create game model preview
				for (size_t i = 0; i < gameModelListToUpdate.size(); i++)
					createGameModelPreview(gameModelListToUpdate[i]);

				deleteFile((currentProject->getProjectFolder() + name + ".model").c_str());

				delete meshPreviewTextures[name];
				meshPreviewTextures.erase(name);
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static deleteMeshPopup deleteMeshWindow;

class selectMeshPopUp : public ImGuiModalPopup
{
	FEMesh** objToWorkWith;
	int meshIndexUnderMouse = -1;
	int meshIndexSelected = -1;
	bool pushedStyle = false;
	std::vector<std::string> meshList;
	std::vector<std::string> filteredMeshList;
	char filter[512];
public:
	selectMeshPopUp()
	{
		popupCaption = "Select mesh";
	}

	void show(FEMesh** mesh)
	{
		shouldOpen = true;
		pushedStyle = false;
		objToWorkWith = mesh;
		meshList = FEResourceManager::getInstance().getMeshList();
		std::vector<std::string> standardMeshList = FEResourceManager::getInstance().getStandardMeshList();
		for (size_t i = 0; i < standardMeshList.size(); i++)
		{
			meshList.insert(meshList.begin(), standardMeshList[i]);
		}

		filteredMeshList = meshList;
		strcpy_s(filter, "");
	}

	void close() override
	{
		ImGuiModalPopup::close();
		meshIndexUnderMouse = -1;
		meshIndexSelected = -1;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredMeshList = meshList;
				}
				else
				{
					filteredMeshList.clear();
					for (size_t i = 0; i < meshList.size(); i++)
					{
						if (meshList[i].find(filter) != -1)
						{
							filteredMeshList.push_back(meshList[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectMeshPopupColumns", false);
			for (size_t i = 0; i < filteredMeshList.size(); i++)
			{
				ImGui::PushID(filteredMeshList[i].c_str());
				pushedStyle = false;
				if (meshIndexSelected == i)
				{
					pushedStyle = true;
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				}

				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (meshIndexUnderMouse != -1)
					{
						*objToWorkWith = FEResourceManager::getInstance().getMesh(filteredMeshList[meshIndexUnderMouse]);
						close();
					}
				}

				if (ImGui::ImageButton((void*)(intptr_t)getMeshPreview(filteredMeshList[i])->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					meshIndexSelected = i;
				}

				if (pushedStyle)
				{
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}

				if (ImGui::IsItemHovered())
				{
					meshIndexUnderMouse = i;
				}

				ImGui::Text(filteredMeshList[i].c_str());
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::SetCursorPosX(300);
			ImGui::SetCursorPosY(25);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			if (ImGui::Button("Select", ImVec2(140, 24)))
			{
				if (meshIndexSelected != -1)
				{
					*objToWorkWith = FEResourceManager::getInstance().getMesh(filteredMeshList[meshIndexSelected]);
					close();
				}
			}

			ImGui::SetCursorPosX(460);
			ImGui::SetCursorPosY(25);

			if (ImGui::Button("Cancel", ImVec2(140, 24)))
			{
				close();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::EndPopup();
		}
	}
};
static selectMeshPopUp selectMeshWindow;

class selectMaterialPopUp : public ImGuiModalPopup
{
	FEMaterial** objToWorkWith;
	int materialIndexUnderMouse = -1;
	int materialIndexSelected = -1;
	bool pushedStyle = false;
	std::vector<std::string> materialList;
	std::vector<std::string> filteredMaterialList;
	char filter[512];
public:
	selectMaterialPopUp()
	{
		popupCaption = "Select material";
	}

	void show(FEMaterial** material)
	{
		shouldOpen = true;
		pushedStyle = false;
		objToWorkWith = material;
		materialList = FEResourceManager::getInstance().getMaterialList();

		filteredMaterialList = materialList;
		strcpy_s(filter, "");
	}

	void close() override
	{
		ImGuiModalPopup::close();
		materialIndexUnderMouse = -1;
		materialIndexSelected = -1;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredMaterialList = materialList;
				}
				else
				{
					filteredMaterialList.clear();
					for (size_t i = 0; i < materialList.size(); i++)
					{
						if (materialList[i].find(filter) != -1)
						{
							filteredMaterialList.push_back(materialList[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectMeshPopupColumns", false);
			for (size_t i = 0; i < filteredMaterialList.size(); i++)
			{
				ImGui::PushID(filteredMaterialList[i].c_str());
				pushedStyle = false;
				if (materialIndexSelected == i)
				{
					pushedStyle = true;
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				}

				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (materialIndexUnderMouse != -1)
					{
						*objToWorkWith = FEResourceManager::getInstance().getMaterial(filteredMaterialList[materialIndexUnderMouse]);
						close();
					}
				}

				if (ImGui::ImageButton((void*)(intptr_t)getMaterialPreview(filteredMaterialList[i])->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					materialIndexSelected = i;
				}

				if (pushedStyle)
				{
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}

				if (ImGui::IsItemHovered())
				{
					materialIndexUnderMouse = i;
				}

				ImGui::Text(filteredMaterialList[i].c_str());
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::SetCursorPosX(300);
			ImGui::SetCursorPosY(25);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			if (ImGui::Button("Select", ImVec2(140, 24)))
			{
				if (materialIndexSelected != -1)
				{
					*objToWorkWith = FEResourceManager::getInstance().getMaterial(filteredMaterialList[materialIndexSelected]);
					close();
				}
			}

			ImGui::SetCursorPosX(460);
			ImGui::SetCursorPosY(25);

			if (ImGui::Button("Cancel", ImVec2(140, 24)))
			{
				close();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

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
	bool pushedStyle = false;
	std::vector<std::string> gameModelList;
	std::vector<std::string> filteredGameModelList;
	char filter[512];
	bool newEntityFlag = false;
	bool wasSelectedAlready = false;
public:
	selectGameModelPopUp()
	{
		popupCaption = "Select game model";
	}

	void show(FEGameModel** gameModel, bool newEntityFlag = false)
	{
		wasSelectedAlready = false;
		shouldOpen = true;
		pushedStyle = false;
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

		gameModelList = FEResourceManager::getInstance().getGameModelList();
		std::vector<std::string> standardGameModelList = FEResourceManager::getInstance().getStandardGameModelList();
		for (size_t i = 0; i < standardGameModelList.size(); i++)
		{
			gameModelList.insert(gameModelList.begin(), standardGameModelList[i]);
		}

		filteredGameModelList = gameModelList;
		strcpy_s(filter, "");
	}

	void close() override
	{
		ImGuiModalPopup::close();
		IndexUnderMouse = -1;
		IndexSelected = -1;
	}

	void render() override
	{
		ImGuiModalPopup::render();

		ImGui::SetNextWindowSize(ImVec2(128 * 7, 800));
		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Filter: ");
			ImGui::SameLine();

			if (ImGui::InputText("", filter, IM_ARRAYSIZE(filter)))
			{
				if (strlen(filter) == 0)
				{
					filteredGameModelList = gameModelList;
				}
				else
				{
					filteredGameModelList.clear();
					for (size_t i = 0; i < gameModelList.size(); i++)
					{
						if (gameModelList[i].find(filter) != -1)
						{
							filteredGameModelList.push_back(gameModelList[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectMeshPopupColumns", false);
			for (size_t i = 0; i < filteredGameModelList.size(); i++)
			{
				ImGui::PushID(filteredGameModelList[i].c_str());
				pushedStyle = false;
				if (IndexSelected == i)
				{
					pushedStyle = true;
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));
				}

				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1 && !wasSelectedAlready)
					{
						if (newEntityFlag)
						{
							FEScene::getInstance().addEntity(FEResourceManager::getInstance().getGameModel(filteredGameModelList[IndexUnderMouse]));
							wasSelectedAlready = true;
						}
						else
						{
							*objToWorkWith = FEResourceManager::getInstance().getGameModel(filteredGameModelList[IndexUnderMouse]);
						}
						
						close();
					}
				}

				if (ImGui::ImageButton((void*)(intptr_t)getGameModelPreview(filteredGameModelList[i])->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), 8, ImColor(0.0f, 0.0f, 0.0f, 0.0f), ImColor(1.0f, 1.0f, 1.0f, 1.0f)))
				{
					IndexSelected = i;
				}

				if (pushedStyle)
				{
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}

				if (ImGui::IsItemHovered())
				{
					IndexUnderMouse = i;
				}

				ImGui::Text(filteredGameModelList[i].c_str());
				ImGui::PopID();
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::SetCursorPosX(300);
			ImGui::SetCursorPosY(25);

			ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

			if (ImGui::Button("Select", ImVec2(140, 24)))
			{
				if (IndexSelected != -1)
				{
					if (newEntityFlag)
					{
						FEScene::getInstance().addEntity(FEResourceManager::getInstance().getGameModel(filteredGameModelList[IndexSelected]));
					}
					else
					{
						*objToWorkWith = FEResourceManager::getInstance().getGameModel(filteredGameModelList[IndexUnderMouse]);
					}
					close();
				}
			}

			ImGui::SetCursorPosX(460);
			ImGui::SetCursorPosY(25);

			if (ImGui::Button("Cancel", ImVec2(140, 24)))
			{
				close();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGui::EndPopup();
		}
	}
};
static selectGameModelPopUp selectGameModelWindow;

class renameGameModelPopUp : public ImGuiModalPopup
{
	FEGameModel* objToWorkWith;
	char newName[512];
public:
	renameGameModelPopUp()
	{
		popupCaption = "Rename game model";
		objToWorkWith = nullptr;
	}

	void show(FEGameModel* ObjToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = ObjToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGui::EndPopup();
				return;
			}

			FEResourceManager& resourceManager = FEResourceManager::getInstance();
			ImGui::Text("New game model name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();

			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (resourceManager.setGameModelName(objToWorkWith, newName))
				{
					// save assets list with new texture name
					currentProject->saveScene();

					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameGameModelPopUp renameGameModelWindow;

int timesGameModelUsed(FEGameModel* gameModel);
class deleteGameModelPopup : public ImGuiModalPopup
{
	FEGameModel *objToWorkWith;
public:
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

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGuiModalPopup::close();
				return;
			}

			// check if this game model is used in some entities
			// to-do: should be done through counter, not by searching each time.
			FEResourceManager& resourceManager = FEResourceManager::getInstance();
			int result = timesGameModelUsed(objToWorkWith);

			ImGui::Text(("Do you want to delete \"" + objToWorkWith->getName() + "\" game model ?").c_str());
			if (result > 0)
				ImGui::Text(("It is used in " + std::to_string(result) + " entities !").c_str());

			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				std::string name = objToWorkWith->getName();
				FEScene::getInstance().prepareForGameModelDeletion(objToWorkWith);
				FEResourceManager::getInstance().deleteGameModel(objToWorkWith);
				currentProject->saveScene();

				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGuiModalPopup::close();
			}

			ImGui::EndPopup();
		}
	}
};
static deleteGameModelPopup deleteGameModelWindow;

class renameEntityPopUp : public ImGuiModalPopup
{
	FEEntity* objToWorkWith;
	char newName[512];
public:
	renameEntityPopUp()
	{
		popupCaption = "Rename entity";
		objToWorkWith = nullptr;
	}

	void show(FEEntity* ObjToWorkWith)
	{
		shouldOpen = true;
		objToWorkWith = ObjToWorkWith;
		strcpy_s(newName, objToWorkWith->getName().size() + 1, objToWorkWith->getName().c_str());
	}

	void render() override
	{
		ImGuiModalPopup::render();

		if (ImGui::BeginPopupModal(popupCaption, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (objToWorkWith == nullptr)
			{
				ImGui::EndPopup();
				return;
			}

			FEScene& scene = FEScene::getInstance();
			ImGui::Text("New entity name :");
			ImGui::InputText("", newName, IM_ARRAYSIZE(newName));
			ImGui::Separator();

			if (ImGui::Button("Apply", ImVec2(120, 0)))
			{
				std::string oldName = objToWorkWith->getName();
				// if new name is acceptable
				if (scene.setEntityName(objToWorkWith, newName))
				{
					// save assets list with new entity name
					currentProject->saveScene();

					ImGuiModalPopup::close();
					strcpy_s(newName, "");
				}
				else
				{
					objToWorkWith = nullptr;
					ImGuiModalPopup::close();
					renameFailedWindow.show();
				}
			}
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				objToWorkWith = nullptr;
				ImGuiModalPopup::close();
			}
			ImGui::EndPopup();
		}
	}
};
static renameEntityPopUp renameEntityWindow;

class ImGuiWindow
{
protected:
	bool visible;
	char popupCaption[512];
	ImVec2 position;
	ImVec2 size;
	int flags = ImGuiWindowFlags_None;
	bool wasClosedLastFrame = false;

	virtual void close()
	{
		visible = false;
		ImGui::End();
	}
public:
	ImGuiWindow()
	{
		position = ImVec2(0.0f, 0.0f);
		size = ImVec2(100.0f, 100.0f);
		visible = false;
	}

	virtual void show()
	{
		visible = true;
		wasClosedLastFrame = true;
	}

	virtual void render()
	{
		if (visible)
		{
			if (wasClosedLastFrame)
			{
				ImGui::SetNextWindowPos(position);
				wasClosedLastFrame = false;
			}
			ImGui::SetNextWindowSize(size);
			ImGui::Begin(popupCaption, nullptr, flags);
		}
	}

	virtual void onRenderEnd()
	{
		if (visible)
			ImGui::End();
	}

	bool isVisible()
	{
		return visible;
	}
};

class editGameModelPopup : public ImGuiWindow
{
	FEGameModel* objToWorkWith;
	FEGameModel* tempModel = nullptr;
	FETexture* tempPreview = nullptr;

	FEMesh* previousMesh;
	FEMaterial* previousMaterial;
public:
	editGameModelPopup()
	{
		tempModel = new FEGameModel(nullptr, nullptr, "tempGameModel");
		objToWorkWith = nullptr;
		flags = ImGuiWindowFlags_NoResize;
	}

	void show(FEGameModel* GameModel)
	{
		if (GameModel != nullptr)
		{
			objToWorkWith = GameModel;
			tempModel->mesh = objToWorkWith->mesh;
			tempModel->material = objToWorkWith->material;

			std::string caption = "Edit game model:";
			caption += " " + objToWorkWith->getName();
			strcpy_s(popupCaption, caption.size() + 1, caption.c_str());
			size = ImVec2(350.0f, 400.0f);
			position = ImVec2(FEngine::getInstance().getWindowWidth() / 2 - size.x / 2, FEngine::getInstance().getWindowHeight() / 2 - size.y / 2);
			ImGuiWindow::show();

			previousMesh = objToWorkWith->mesh;
			previousMaterial = objToWorkWith->material;

			createGameModelPreview(tempModel, &tempPreview);
		}
	}

	void render() override
	{
		ImGuiWindow::render();

		if (!isVisible())
			return;

		// if we change something we will update preview.
		if (previousMesh != tempModel->mesh || previousMaterial != tempModel->material)
		{
			createGameModelPreview(tempModel, &tempPreview);
			previousMesh = tempModel->mesh;
			previousMaterial = tempModel->material;
		}

		if (objToWorkWith == nullptr)
		{
			ImGuiWindow::close();
			return;
		}

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.5f, 0.5f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.90f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

		ImVec2 textSize = ImGui::CalcTextSize("Preview of game model:");
		ImGui::SetCursorPosX(size.x / 2 - textSize.x / 2);
		ImGui::SetCursorPosY(30);
		ImGui::Text("Preview of game model:");
		ImGui::SetCursorPosX(size.x / 2 - 128 / 2);
		ImGui::SetCursorPosY(50);
		ImGui::Image((void*)(intptr_t)tempPreview->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

		ImGui::Separator();
		textSize = ImGui::CalcTextSize("Mesh component:");
		ImGui::SetCursorPosX(size.x / 4 - textSize.x / 2);
		ImGui::Text("Mesh component:");
		ImGui::SetCursorPosX(size.x / 4 - 128 / 2);
		ImGui::Image((void*)(intptr_t)getMeshPreview(tempModel->mesh->getName())->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosX(size.x / 4 - 120 / 2);
		if (ImGui::Button("Change Mesh", ImVec2(120, 0)))
		{
			selectMeshWindow.show(&tempModel->mesh);
		}

		textSize = ImGui::CalcTextSize("Material component:");
		ImGui::SetCursorPosX(size.x / 2 + size.x / 4 - textSize.x / 2);
		ImGui::SetCursorPosY(187.0f);
		ImGui::Text("Material component:");
		ImGui::SetCursorPosX(size.x / 2 + size.x / 4 - 128 / 2);
		ImGui::SetCursorPosY(203.0f);
		ImGui::Image((void*)(intptr_t)getMaterialPreview(tempModel->material->getName())->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		ImGui::SetCursorPosX(size.x / 2 + size.x / 4 - 120 / 2);
		ImGui::SetCursorPosY(336.0f);
		if (ImGui::Button("Change Material", ImVec2(120, 0)))
		{
			selectMaterialWindow.show(&tempModel->material);
		}

		ImGui::Separator();
		ImGui::SetItemDefaultFocus();
		ImGui::SetCursorPosX(size.x / 4 - 120 / 2);
		ImGui::SetCursorPosY(size.y - 30);
		if (ImGui::Button("Apply", ImVec2(120, 0)))
		{
			objToWorkWith->mesh = tempModel->mesh;
			objToWorkWith->material = tempModel->material;
			createGameModelPreview(objToWorkWith->getName());

			ImGuiWindow::close();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			return;
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(0.7f, 0.5f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.95f, 0.5f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.1f, 1.0f, 0.1f, 1.0f));

		ImGui::SameLine();
		ImGui::SetCursorPosX(size.x / 2 + size.x / 4 - 120 / 2);
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();
			ImGui::PopStyleColor();

			ImGuiWindow::close();
			return;
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGuiWindow::onRenderEnd();
	}
};
static editGameModelPopup editGameModelWindow;
