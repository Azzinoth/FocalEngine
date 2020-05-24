#pragma once

#include "FEProject.h"
#include <direct.h>

using namespace FocalEngine;

#define PROJECTS_FOLDER "C:/Users/kandr/Downloads/FEProjects"

static double mouseX, mouseY;
static bool isCameraInputActive = false;
static std::pair<std::string, float > entityUnderMouse;
static std::string selectedEntity = "";
static bool selectedEntityWasChanged;
static std::vector<FEProject*> projectList;
static int projectChosen = -1;

static FEProject* currentProject = nullptr;

void determineEntityUnderMouse();
void mouseButtonCallback(int button, int action, int mods);
glm::dvec3 mouseRay();
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
void displayMaterialEditor();
void displayContentBrowser();
void displayPostProcess();
void displayProjectSelection();

void addEntityButton();

void saveMaterials(const char* fileName = nullptr);
void loadMaterials(const char* fileName = nullptr);

// file system
bool checkFolder(const char* dirPath);
bool createFolder(const char* dirPath);
bool deleteFolder(const char* dirPath);
std::vector<std::string> getFolderList(const char* dirPath);
// file system END

void renderEditor();
void loadProjectList();
void loadEditor();



