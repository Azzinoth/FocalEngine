#pragma once

#include "FEProject.h"
#include "FEDearImguiWrapper/FEDearImguiWrapper.h"
#include "../Editor/FEPixelAccurateSelectionShader.h"
#include "../Editor/FEMeshPreviewShader.h"
#include "../Editor/FESelectionHaloEffect.h"
#include "../ThirdParty/textEditor/TextEditor.h"
#ifdef FE_WIN_32
	#include <direct.h> // file system
	#include <shobjidl.h> // openDialog
#endif

using namespace FocalEngine;

#define PROJECTS_FOLDER "C:/Users/kandr/Downloads/FEProjects"

static double lastMouseX, lastMouseY;
static double mouseX, mouseY;
static bool isCameraInputActive = false;

//static TextEditor editor;
// **************************** entity selection ****************************
glm::dvec3 mouseRay();

void determineEntityUnderMouse();
static std::vector<std::string> entitiesUnderMouse;
static std::string selectedEntity = "";
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

static std::unordered_map<int, FEEntity*> internalEditorEntities;
static void addEntityToInternalEditorList(FEEntity* entity);
static bool isInInternalEditorList(FEEntity* entity);

static std::unordered_map<int, FEGameModel*> internalEditorGameModels;
static void addGameModelToInternalEditorList(FEGameModel* gameModel);
static bool isInInternalEditorList(FEGameModel* gameModel)
{
	return !(internalEditorGameModels.find(std::hash<std::string>{}(gameModel->getName())) == internalEditorGameModels.end());
}

static std::unordered_map<int, FEMesh*> internalEditorMesh;
static void addMeshToInternalEditorList(FEMesh* mesh);
static bool isInInternalEditorList(FEMesh* mesh)
{
	return !(internalEditorMesh.find(std::hash<std::string>{}(mesh->getName())) == internalEditorMesh.end());
}

// **************************** Gizmos ****************************
static float gizmosScale = 0.00175f;

static const int TRANSFORM_GIZMOS = 0;
static const int SCALE_GIZMOS = 1;
static const int ROTATE_GIZMOS = 2;

static int gizmosState = TRANSFORM_GIZMOS;
static void loadGizmos();

static FEEntity* transformationXGizmoEntity = nullptr;
static FEEntity* transformationYGizmoEntity = nullptr;
static FEEntity* transformationZGizmoEntity = nullptr;

static std::string transformationXGizmoName = "transformationXGizmoEntity";
static int transformationXGizmoNameHash = std::hash<std::string>{}(transformationXGizmoName);

static std::string transformationYGizmoName = "transformationYGizmoEntity";
static int transformationYGizmoNameHash = std::hash<std::string>{}(transformationYGizmoName);

static std::string transformationZGizmoName = "transformationZGizmoEntity";
static int transformationZGizmoNameHash = std::hash<std::string>{}(transformationZGizmoName);

static bool transformationXGizmoActive = false;
static bool transformationYGizmoActive = false;
static bool transformationZGizmoActive = false;

static FEEntity* transformationXYGizmoEntity = nullptr;
static FEEntity* transformationYZGizmoEntity = nullptr;
static FEEntity* transformationXZGizmoEntity = nullptr;

static std::string transformationXYGizmoName = "transformationXYGizmoEntity";
static int transformationXYGizmoNameHash = std::hash<std::string>{}(transformationXYGizmoName);

static std::string transformationYZGizmoName = "transformationYZGizmoEntity";
static int transformationYZGizmoNameHash = std::hash<std::string>{}(transformationYZGizmoName);

static std::string transformationXZGizmoName = "transformationXZGizmoEntity";
static int transformationXZGizmoNameHash = std::hash<std::string>{}(transformationXZGizmoName);

static bool transformationXYGizmoActive = false;
static bool transformationYZGizmoActive = false;
static bool transformationXZGizmoActive = false;

// scale part
static FEEntity* scaleXGizmoEntity = nullptr;
static FEEntity* scaleYGizmoEntity = nullptr;
static FEEntity* scaleZGizmoEntity = nullptr;

static bool scaleXGizmoActive = false;
static bool scaleYGizmoActive = false;
static bool scaleZGizmoActive = false;

static std::string scaleXGizmoName = "scaleXGizmoEntity";
static int scaleXGizmoNameHash = std::hash<std::string>{}(scaleXGizmoName);

static std::string scaleYGizmoName = "scaleYGizmoEntity";
static int scaleYGizmoNameHash = std::hash<std::string>{}(scaleYGizmoName);

static std::string scaleZGizmoName = "scaleZGizmoEntity";
static int scaleZGizmoNameHash = std::hash<std::string>{}(scaleZGizmoName);

// rotate part
static FEEntity* rotateXGizmoEntity = nullptr;
static FEEntity* rotateYGizmoEntity = nullptr;
static FEEntity* rotateZGizmoEntity = nullptr;

static bool rotateXGizmoActive = false;
static bool rotateYGizmoActive = false;
static bool rotateZGizmoActive = false;

static glm::vec3 rotateXStandardRotation = glm::vec3(0.0f, 0.0f, -90.0f);
static glm::vec3 rotateYStandardRotation = glm::vec3(0.0f);
static glm::vec3 rotateZStandardRotation = glm::vec3(90.0f, 0.0f, 90.0f);

static std::string rotateXGizmoName = "rotateXGizmoEntity";
static int rotateXGizmoNameHash = std::hash<std::string>{}(rotateXGizmoName);

static std::string rotateYGizmoName = "rotateYGizmoEntity";
static int rotateYGizmoNameHash = std::hash<std::string>{}(rotateYGizmoName);

static std::string rotateZGizmoName = "rotateZGizmoEntity";
static int rotateZGizmoNameHash = std::hash<std::string>{}(rotateZGizmoName);

static void changeGizmoState(int newState)
{
	if (newState < 0 || newState > 2)
		newState = 0;

	gizmosState = newState;

	transformationXGizmoEntity->setVisibility(false);
	transformationYGizmoEntity->setVisibility(false);
	transformationZGizmoEntity->setVisibility(false);

	transformationXYGizmoEntity->setVisibility(false);
	transformationYZGizmoEntity->setVisibility(false);
	transformationXZGizmoEntity->setVisibility(false);

	transformationXGizmoActive = false;
	transformationYGizmoActive = false;
	transformationZGizmoActive = false;

	transformationXYGizmoActive = false;
	transformationYZGizmoActive = false;
	transformationXZGizmoActive = false;

	scaleXGizmoEntity->setVisibility(false);
	scaleYGizmoEntity->setVisibility(false);
	scaleZGizmoEntity->setVisibility(false);

	scaleXGizmoActive = false;
	scaleYGizmoActive = false;
	scaleZGizmoActive = false;

	rotateXGizmoEntity->setVisibility(false);
	rotateYGizmoEntity->setVisibility(false);
	rotateZGizmoEntity->setVisibility(false);

	rotateXGizmoActive = false;
	rotateYGizmoActive = false;
	rotateZGizmoActive = false;

	switch (newState)
	{
	case TRANSFORM_GIZMOS:
	{
		if (selectedEntity.size() != 0)
		{
			transformationXGizmoEntity->setVisibility(true);
			transformationYGizmoEntity->setVisibility(true);
			transformationZGizmoEntity->setVisibility(true);

			transformationXYGizmoEntity->setVisibility(true);
			transformationYZGizmoEntity->setVisibility(true);
			transformationXZGizmoEntity->setVisibility(true);
		}
		break;
	}
	case SCALE_GIZMOS:
	{
		if (selectedEntity.size() != 0)
		{
			scaleXGizmoEntity->setVisibility(true);
			scaleYGizmoEntity->setVisibility(true);
			scaleZGizmoEntity->setVisibility(true);
		}
		break;
	}
	case ROTATE_GIZMOS:
	{
		if (selectedEntity.size() != 0)
		{
			rotateXGizmoEntity->setVisibility(true);
			rotateYGizmoEntity->setVisibility(true);
			rotateZGizmoEntity->setVisibility(true);
		}
		break;
	}
	default:
		break;
	}
}

static FETexture* transformationGizmoIcon = nullptr;
static FETexture* scaleGizmoIcon = nullptr;
static FETexture* rotateGizmoIcon = nullptr;

//static std::string transformationGizmoIconName = "transformationGizmoIcon";
//static int transformationGizmoIconNameHash = std::hash<std::string>{}(transformationGizmoIconName);
//static std::string scaleGizmoIconName = "scaleGizmoIcon";
//static int scaleGizmoIconNameHash = std::hash<std::string>{}(scaleGizmoIconName);
//static std::string rotateGizmoIconName = "rotateGizmoIcon";
//static int rotateGizmoIconNameHash = std::hash<std::string>{}(rotateGizmoIconName);

void editorOnCameraUpdate(FEBasicCamera* camera);

// **************************** Gizmos END ****************************

// **************************** Default GUI Colors ****************************

static ImVec4 defaultColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
static ImVec4 hoveredColor = ImVec4(0.95f, 0.90f, 0.0f, 1.0f);
static ImVec4 activeColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);

static ImVec4 selectedStyle = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);

static void setSelectedStyle(ImGuiImageButton* button)
{
	button->setDefaultColor(selectedStyle);
	button->setHoveredColor(selectedStyle);
	button->setActiveColor(selectedStyle);
}

static void setDefaultStyle(ImGuiImageButton* button)
{
	button->setDefaultColor(defaultColor);
	button->setHoveredColor(hoveredColor);
	button->setActiveColor(activeColor);
}

// **************************** Default GUI Colors END ****************************
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
static FEMaterial* meshPreviewMaterial;

void createMeshPreview(std::string meshName);
FETexture* getMeshPreview(std::string meshName);

void displayMeshesContentBrowser();
// **************************** Meshes Content Browser END ****************************

// **************************** Halo selection ****************************
static FEFramebuffer* haloObjectsFB = nullptr;
static FEEntity* selectedEntityObject = nullptr;
static FEMaterial* haloMaterial = nullptr;
static FEPostProcess* selectionHaloEffect = nullptr;
// **************************** Halo selection END ****************************
void displayTexturesContentBrowser();
void displayTextureInMaterialEditor(FEMaterial* material, FETexture*& texture);

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
				currentProject->saveScene(&internalEditorEntities);

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
	ImGuiButton* okButton = nullptr;
public:
	renameFailedPopUp()
	{
		popupCaption = "Invalid name";
		okButton = new ImGuiButton("OK");
		okButton->setDefaultColor(ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		okButton->setHoveredColor(ImVec4(0.95f, 0.90f, 0.0f, 1.0f));
		okButton->setPosition(ImVec2(33, 50));
	}

	~renameFailedPopUp()
	{
		if (okButton != nullptr)
			delete okButton;
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

			okButton->render();
			if (okButton->getWasClicked())
			{
				ImGuiModalPopup::close();
			}

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
					currentProject->saveScene(&internalEditorEntities);

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
				currentProject->saveScene(&internalEditorEntities);

				ImGuiModalPopup::close();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("No", ImVec2(120, 0)))
			{
				FETexture* newTexture = FEResourceManager::getInstance().LoadPngTextureAndCompress(filePath.c_str(), false);
				FEResourceManager::getInstance().saveFETexture((currentProject->getProjectFolder() + newTexture->getName() + ".FETexture").c_str(), newTexture);
				// add asset list saving....
				currentProject->saveScene(&internalEditorEntities);

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
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
	std::vector<std::string> list;
	std::vector<std::string> filteredList;
	char filter[512];

	ImGuiButton* selectButton = nullptr;
	ImGuiButton* cancelButton = nullptr;
	ImGuiImageButton* iconButton = nullptr;
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
		selectButton->setPosition(ImVec2(300, 25));
		cancelButton = new ImGuiButton("Cancel");
		cancelButton->setSize(ImVec2(140, 24));
		cancelButton->setPosition(ImVec2(460, 25));
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

	void show(FETexture** texture)
	{
		shouldOpen = true;
		objToWorkWith = texture;
		list = FEResourceManager::getInstance().getTextureList();
		filteredList = list;
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
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectTexturePopupColumns", false);
			for (size_t i = 0; i < filteredList.size(); i++)
			{
				ImGui::PushID(filteredList[i].c_str());
				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1)
					{
						*objToWorkWith = FEResourceManager::getInstance().getTexture(filteredList[IndexUnderMouse]);
						close();
					}
				}

				IndexSelected == i ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
				iconButton->setTexture(FEResourceManager::getInstance().getTexture(filteredList[i]));
				iconButton->render();
				if (iconButton->getWasClicked())
				{
					IndexSelected = i;
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
					*objToWorkWith = FEResourceManager::getInstance().getTexture(filteredList[IndexSelected]);
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
					currentProject->saveScene(&internalEditorEntities);

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
				currentProject->saveScene(&internalEditorEntities);

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
	int IndexUnderMouse = -1;
	int IndexSelected = -1;
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
		selectButton->setPosition(ImVec2(300, 25));
		cancelButton = new ImGuiButton("Cancel");
		cancelButton->setSize(ImVec2(140, 24));
		cancelButton->setPosition(ImVec2(460, 25));
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
		list = FEResourceManager::getInstance().getMeshList();
		std::vector<std::string> standardMeshList = FEResourceManager::getInstance().getStandardMeshList();
		for (size_t i = 0; i < standardMeshList.size(); i++)
		{
			if (isInInternalEditorList(FEResourceManager::getInstance().getMesh(standardMeshList[i])))
				continue;
			list.insert(list.begin(), standardMeshList[i]);
		}

		filteredList = list;
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
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectMeshPopupColumns", false);
			for (size_t i = 0; i < filteredList.size(); i++)
			{
				ImGui::PushID(filteredList[i].c_str());

				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1)
					{
						*objToWorkWith = FEResourceManager::getInstance().getMesh(filteredList[IndexUnderMouse]);
						close();
					}
				}

				IndexSelected == i ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
				iconButton->setTexture(getMeshPreview(filteredList[i]));
				iconButton->render();
				if (iconButton->getWasClicked())
				{
					IndexSelected = i;
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
					*objToWorkWith = FEResourceManager::getInstance().getMesh(filteredList[IndexSelected]);
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
		selectButton->setPosition(ImVec2(300, 25));
		cancelButton = new ImGuiButton("Cancel");
		cancelButton->setSize(ImVec2(140, 24));
		cancelButton->setPosition(ImVec2(460, 25));
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
		list = FEResourceManager::getInstance().getMaterialList();
		list.insert(list.begin(), "SolidColorMaterial");
		
		filteredList = list;
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
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectMeshPopupColumns", false);
			for (size_t i = 0; i < filteredList.size(); i++)
			{
				ImGui::PushID(filteredList[i].c_str());
				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1)
					{
						*objToWorkWith = FEResourceManager::getInstance().getMaterial(filteredList[IndexUnderMouse]);
						close();
					}
				}

				IndexSelected == i ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
				iconButton->setTexture(getMaterialPreview(filteredList[i]));
				iconButton->render();
				if (iconButton->getWasClicked())
				{
					IndexSelected = i;
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
					*objToWorkWith = FEResourceManager::getInstance().getMaterial(filteredList[IndexSelected]);
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
	std::vector<std::string> list;
	std::vector<std::string> filteredlList;
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
		selectButton->setPosition(ImVec2(300, 25));
		cancelButton = new ImGuiButton("Cancel");
		cancelButton->setSize(ImVec2(140, 24));
		cancelButton->setPosition(ImVec2(460, 25));
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

		list = FEResourceManager::getInstance().getGameModelList();
		std::vector<std::string> standardGameModelList = FEResourceManager::getInstance().getStandardGameModelList();
		for (size_t i = 0; i < standardGameModelList.size(); i++)
		{
			if (isInInternalEditorList(FEResourceManager::getInstance().getGameModel(standardGameModelList[i])))
				continue;
			list.insert(list.begin(), standardGameModelList[i]);
		}

		filteredlList = list;
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
					filteredlList = list;
				}
				else
				{
					filteredlList.clear();
					for (size_t i = 0; i < list.size(); i++)
					{
						if (list[i].find(filter) != -1)
						{
							filteredlList.push_back(list[i]);
						}
					}
				}
			}
			ImGui::Separator();

			ImGui::SetCursorPosX(0);
			ImGui::SetCursorPosY(60);
			ImGui::Columns(5, "selectGameModelPopupColumns", false);
			for (size_t i = 0; i < filteredlList.size(); i++)
			{
				ImGui::PushID(filteredlList[i].c_str());
				if (ImGui::IsMouseDoubleClicked(0))
				{
					if (IndexUnderMouse != -1 && !wasSelectedAlready)
					{
						if (newEntityFlag)
						{
							FEScene::getInstance().addEntity(FEResourceManager::getInstance().getGameModel(filteredlList[IndexUnderMouse]));
							wasSelectedAlready = true;
						}
						else
						{
							*objToWorkWith = FEResourceManager::getInstance().getGameModel(filteredlList[IndexUnderMouse]);
						}
						
						close();
					}
				}

				IndexSelected == i ? setSelectedStyle(iconButton) : setDefaultStyle(iconButton);
				iconButton->setTexture(getGameModelPreview(filteredlList[i]));
				iconButton->render();
				if (iconButton->getWasClicked())
				{
					IndexSelected = i;
				}

				if (iconButton->isHovered())
				{
					IndexUnderMouse = i;
				}

				ImGui::Text(filteredlList[i].c_str());
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
						FEScene::getInstance().addEntity(FEResourceManager::getInstance().getGameModel(filteredlList[IndexSelected]));
					}
					else
					{
						*objToWorkWith = FEResourceManager::getInstance().getGameModel(filteredlList[IndexUnderMouse]);
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
					currentProject->saveScene(&internalEditorEntities);

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
	FEGameModel* objToWorkWith;
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
				currentProject->saveScene(&internalEditorEntities);

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
					currentProject->saveScene(&internalEditorEntities);

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

class editGameModelPopup : public ImGuiWindow
{
	FEGameModel* objToWorkWith;
	FEGameModel* tempModel = nullptr;
	FETexture* tempPreview = nullptr;

	FEMesh* previousMesh;
	FEMaterial* previousMaterial;

	ImGuiButton* cancelButton;
	ImGuiButton* applyButton;
	ImGuiButton* changeMaterialButton;
	ImGuiButton* changeMeshButton;
public:
	editGameModelPopup()
	{
		tempModel = new FEGameModel(nullptr, nullptr, "tempGameModel");
		objToWorkWith = nullptr;
		flags = ImGuiWindowFlags_NoResize;

		cancelButton = new ImGuiButton("Cancel");
		cancelButton->setDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
		cancelButton->setHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
		cancelButton->setActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));

		applyButton = new ImGuiButton("Apply");
		changeMaterialButton = new ImGuiButton("Change Material");
		changeMeshButton = new ImGuiButton("Change Mesh");
	}

	~editGameModelPopup()
	{
		if (cancelButton != nullptr)
			delete cancelButton;

		if (applyButton != nullptr)
			delete applyButton;

		if (changeMaterialButton != nullptr)
			delete changeMaterialButton;

		if (changeMeshButton != nullptr)
			delete changeMeshButton;
	}

	void show(FEGameModel* GameModel)
	{
		if (GameModel != nullptr)
		{
			objToWorkWith = GameModel;
			tempModel->mesh = objToWorkWith->mesh;
			tempModel->material = objToWorkWith->material;

			std::string tempCaption = "Edit game model:";
			tempCaption += " " + objToWorkWith->getName();
			strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());
			size = ImVec2(350.0f, 400.0f);
			position = ImVec2(FEngine::getInstance().getWindowWidth() / 2 - size.x / 2, FEngine::getInstance().getWindowHeight() / 2 - size.y / 2);
			ImGuiWindow::show();

			previousMesh = objToWorkWith->mesh;
			previousMaterial = objToWorkWith->material;

			createGameModelPreview(tempModel, &tempPreview);

			changeMaterialButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - 120 / 2, 336.0f));
			changeMeshButton->setPosition(ImVec2(size.x / 4 - 120 / 2, -1.0f));
			applyButton->setPosition(ImVec2(size.x / 4 - 120 / 2, size.y - 30));
			cancelButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - 120 / 2, -1.0f));
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
		changeMeshButton->render();
		if (changeMeshButton->getWasClicked())
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
		changeMaterialButton->render();
		if (changeMaterialButton->getWasClicked())
		{
			selectMaterialWindow.show(&tempModel->material);
		}

		ImGui::Separator();
		ImGui::SetItemDefaultFocus();
		applyButton->render();
		if (applyButton->getWasClicked())
		{
			objToWorkWith->mesh = tempModel->mesh;
			objToWorkWith->material = tempModel->material;
			createGameModelPreview(objToWorkWith->getName());

			ImGuiWindow::close();
			return;
		}

		ImGui::SameLine();
		cancelButton->render();
		if (cancelButton->getWasClicked())
		{
			ImGuiWindow::close();
			return;
		}

		ImGuiWindow::onRenderEnd();
	}

	void close()
	{
		ImGuiWindow::close();
	}
};
static editGameModelPopup editGameModelWindow;

class gyzmosSettingsWindow : public ImGuiWindow
{
	ImGuiImageButton* transformationGizmoButton = nullptr;
	ImGuiImageButton* scaleGizmoButton = nullptr;
	ImGuiImageButton* rotateGizmoButton = nullptr;
public:
	gyzmosSettingsWindow()
	{
		flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;
	}

	~gyzmosSettingsWindow() override
	{
		if (transformationGizmoButton != nullptr)
			delete transformationGizmoButton;

		if (scaleGizmoButton != nullptr)
			delete scaleGizmoButton;

		if (rotateGizmoButton != nullptr)
			delete rotateGizmoButton;
	}

	void show() override
	{
		std::string tempCaption = "##GizmosSettingsWindow";
		strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());
		size = ImVec2(146.0f, 48.0f);
		position = ImVec2(FEngine::getInstance().getWindowWidth() / 2.0f - 150.0f / 2.0f, 10.0f);
		ImGuiWindow::show();

		if (transformationGizmoButton == nullptr)
			transformationGizmoButton = new ImGuiImageButton(transformationGizmoIcon);

		if (scaleGizmoButton == nullptr)
			scaleGizmoButton = new ImGuiImageButton(scaleGizmoIcon);

		if (rotateGizmoButton == nullptr)
			rotateGizmoButton = new ImGuiImageButton(rotateGizmoIcon);

		float currentX = 5.0f;
		transformationGizmoButton->setPosition(ImVec2(currentX, 5.0f));
		currentX += 32.0f + 16.0f;
		scaleGizmoButton->setPosition(ImVec2(currentX, 5.0f));
		currentX += 32.0f + 16.0f;
		rotateGizmoButton->setPosition(ImVec2(currentX, 5.0f));
	}

	void render() override
	{
		ImGuiWindow::render();

		if (!isVisible())
			return;

		gizmosState == TRANSFORM_GIZMOS ? setSelectedStyle(transformationGizmoButton) : setDefaultStyle(transformationGizmoButton);
		ImGui::PushID(0);
		transformationGizmoButton->render();
		if (transformationGizmoButton->getWasClicked())
		{
			if (gizmosState != TRANSFORM_GIZMOS)
			{
				changeGizmoState(TRANSFORM_GIZMOS);
				ImGui::PopID();
				ImGuiWindow::onRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		toolTip("Translate objects. key = shift");

		gizmosState == SCALE_GIZMOS ? setSelectedStyle(scaleGizmoButton) : setDefaultStyle(scaleGizmoButton);
		ImGui::PushID(1);
		scaleGizmoButton->render();
		if (scaleGizmoButton->getWasClicked())
		{
			if (gizmosState != SCALE_GIZMOS)
			{
				changeGizmoState(SCALE_GIZMOS);
				ImGui::PopID();
				ImGuiWindow::onRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		toolTip("Scale objects. key = shift");

		gizmosState == ROTATE_GIZMOS ? setSelectedStyle(rotateGizmoButton) : setDefaultStyle(rotateGizmoButton);
		ImGui::PushID(2);
		rotateGizmoButton->render();
		if (rotateGizmoButton->getWasClicked())
		{
			if (gizmosState != ROTATE_GIZMOS)
			{
				changeGizmoState(ROTATE_GIZMOS);
				ImGui::PopID();
				ImGuiWindow::onRenderEnd();
				return;
			}
		}
		ImGui::PopID();
		toolTip("Rotate objects. key = shift");

		ImGuiWindow::onRenderEnd();
	}
};
static gyzmosSettingsWindow gyzmosSettingsWindowObject;

class justTextWindow : public ImGuiWindow
{
	TextEditor editor;
	ImGuiButton* okButton = nullptr;
public:
	justTextWindow()
	{
		flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
		editor.SetShowWhitespaces(false);
		editor.SetReadOnly(true);
		size = ImVec2(800, 600);
		okButton = new ImGuiButton("OK");
		editor.SetPalette(TextEditor::GetLightPalette());
	}

	~justTextWindow()
	{
		if (okButton != nullptr)
			delete okButton;
	}

	void show(std::string text, std::string caption)
	{
		editor.SetText(text);

		if (caption.size() == 0)
			caption = "Text view";

		strcpy_s(this->caption, caption.size() + 1, caption.c_str());
		ImGuiWindow::show();
	}

	void render() override
	{
		ImGuiWindow::render();

		if (!isVisible())
			return;

		okButton->render();
		if (okButton->getWasClicked())
		{
			ImGuiWindow::close();
		}

		//auto cpos = editor.GetCursorPosition();
		/*ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
			editor.IsOverwrite() ? "Ovr" : "Ins",
			editor.CanUndo() ? "*" : " ",
			editor.GetLanguageDefinition().mName.c_str(), "none");*/

		editor.Render("TextEditor");

		ImGuiWindow::onRenderEnd();
	}
};
static justTextWindow justTextWindowObj;

class shaderEditorWindow : public ImGuiWindow
{
	FEShader* shaderToEdit = nullptr;
	FEShader* dummyShader = nullptr;
	TextEditor* currentEditor = nullptr;
	TextEditor vertexShaderEditor;
	TextEditor fragmentShaderEditor;

	ImGuiButton* compileButton = nullptr;
	//std::vector<std::string> list;
	int activeTab = 0;

	void replaceShader(FEShader* oldShader, FEShader* newShader)
	{
		FEResourceManager& resourceManager = FEResourceManager::getInstance();

		std::vector<std::string> materialList = resourceManager.getMaterialList();
		for (size_t i = 0; i < materialList.size(); i++)
		{
			FEMaterial* tempMaterial = resourceManager.getMaterial(materialList[i]);
			if (tempMaterial->shader->getNameHash() == oldShader->getNameHash())
			{
				tempMaterial->shader = newShader;
			}
		}

		materialList = resourceManager.getStandardMaterialList();
		for (size_t i = 0; i < materialList.size(); i++)
		{
			FEMaterial* tempMaterial = resourceManager.getMaterial(materialList[i]);
			if (tempMaterial->shader->getNameHash() == oldShader->getNameHash())
			{
				tempMaterial->shader = newShader;
			}
		}
	}
public:
	shaderEditorWindow()
	{
		flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
		currentEditor = &vertexShaderEditor;
		vertexShaderEditor.SetShowWhitespaces(false);
		fragmentShaderEditor.SetShowWhitespaces(false);
		size = ImVec2(800, 600);
		compileButton = new ImGuiButton("Compile");
	}

	~shaderEditorWindow()
	{
		if (compileButton != nullptr)
			delete compileButton;
	}

	void show(FEShader* shader)
	{
		shaderToEdit = shader;

		vertexShaderEditor.SetText(shaderToEdit->getVertexShaderText());
		fragmentShaderEditor.SetText(shaderToEdit->getFragmentShaderText());
		currentEditor = &vertexShaderEditor;

		/*list = FEResourceManager::getInstance().getStandardShadersList();
		std::string text;
		for (size_t i = 0; i < list.size(); i++)
		{
			text += list[i];
			text += '\n';
		}
		ImGui::SetClipboardText(text.c_str());
		editor.Paste();*/
		std::string tempCaption = "Edit shader: ";
		tempCaption += shaderToEdit->getName();
		strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());
		ImGuiWindow::show();
	}

	void render() override
	{
		ImGuiWindow::render();

		if (!isVisible())
			return;

		compileButton->render();
		if (compileButton->getWasClicked())
		{
			if (dummyShader != nullptr)
				delete dummyShader;

			dummyShader = new FEShader(vertexShaderEditor.GetText().c_str(), fragmentShaderEditor.GetText().c_str(), "dummyShader", true);
			std::string errors = dummyShader->getCompilationErrors();
			if (errors.size() != 0)
			{
				justTextWindowObj.show(errors, "Shader compilation error!");
			}
			else
			{
				FEShader* reCompiledShader = new FEShader(vertexShaderEditor.GetText().c_str(), fragmentShaderEditor.GetText().c_str(), shaderToEdit->getName() + "1");
				replaceShader(shaderToEdit, reCompiledShader);
				FEResourceManager::getInstance().deleteShader(shaderToEdit->getName());
				shaderToEdit = reCompiledShader;
			}
		}

		
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Edit"))
			{
				bool ro = currentEditor->IsReadOnly();
				if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
					currentEditor->SetReadOnly(ro);
				ImGui::Separator();

				if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && currentEditor->CanUndo()))
					currentEditor->Undo();
				if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && currentEditor->CanRedo()))
					currentEditor->Redo();

				ImGui::Separator();

				if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, currentEditor->HasSelection()))
					currentEditor->Copy();
				if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && currentEditor->HasSelection()))
					currentEditor->Cut();
				if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && currentEditor->HasSelection()))
					currentEditor->Delete();
				if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
					currentEditor->Paste();

				ImGui::Separator();

				if (ImGui::MenuItem("Select all", nullptr, nullptr))
					currentEditor->SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(currentEditor->GetTotalLines(), 0));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Dark palette"))
					currentEditor->SetPalette(TextEditor::GetDarkPalette());
				if (ImGui::MenuItem("Light palette"))
					currentEditor->SetPalette(TextEditor::GetLightPalette());
				if (ImGui::MenuItem("Retro blue palette"))
					currentEditor->SetPalette(TextEditor::GetRetroBluePalette());
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		auto cpos = currentEditor->GetCursorPosition();
		ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, currentEditor->GetTotalLines(),
			currentEditor->IsOverwrite() ? "Ovr" : "Ins",
			currentEditor->CanUndo() ? "*" : " ",
			currentEditor->GetLanguageDefinition().mName.c_str(), "none");

		ImGui::PushStyleColor(ImGuiCol_TabActive, (ImVec4)ImColor::ImColor(0.4f, 0.9f, 0.4f, 1.0f));
		if (ImGui::BeginTabBar("##Shaders Editors", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Vertex Shader"))
			{
				activeTab = 0;
				currentEditor = &vertexShaderEditor;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Fragment Shader"))
			{
				activeTab = 1;
				currentEditor = &fragmentShaderEditor;
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
		ImGui::PopStyleColor();
		
		currentEditor->Render("Editor");

		justTextWindowObj.render();
		ImGuiWindow::onRenderEnd();
	}
};
static shaderEditorWindow shadersEditorWindow;
