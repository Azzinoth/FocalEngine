#pragma once

#include "FEngine.h"
#include <direct.h>

#define PROJECTS_FOLDER "C:/Users/kandr/Downloads/FEProjects"

static double mouseX, mouseY;
static bool isCameraInputActive = true;
static std::pair<std::string, float > entityUnderMouse;
static std::string selectedEntity = "";
static bool selectedEntityWasChanged;

void determineEntityUnderMouse();
void mouseButtonCallback(int button, int action, int mods);
glm::dvec3 mouseRay();
void mouseMoveCallback(double xpos, double ypos);
void keyButtonCallback(int key, int scancode, int action, int mods);

void toolTip(const char* text);
void showPosition(std::string objectName, glm::vec3& position);
void showRotation(std::string objectName, glm::vec3& rotation);
void showScale(std::string objectName, glm::vec3& scale);
void showTransformConfiguration(std::string objectName, FocalEngine::FETransformComponent* transform);

void displayMaterialPrameter(FocalEngine::FEShaderParam* param);
void displayMaterialPrameters(FocalEngine::FEMaterial* material);
void displayLightProperties(FocalEngine::FELight* light);
void displayLightsProperties();
void displaySceneEntities();
void displayMaterialEditor();
void displayContentBrowser();
void displayPostProcess();

void addEntityButton();

void saveMaterials(const char* fileName = nullptr);
void loadMaterials(const char* fileName = nullptr);

void writeTransformToJSON(Json::Value& root, FocalEngine::FETransformComponent* transform);
void readTransformToJSON(Json::Value& root, FocalEngine::FETransformComponent* transform);

void saveScene(const char* fileName = nullptr);
void loadScene(const char* fileName = nullptr);

bool checkFolder(const char* dirPath);
bool createFolder(const char* dirPath);

void loadEditor();