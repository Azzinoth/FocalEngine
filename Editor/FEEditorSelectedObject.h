#pragma once

#include "FEEditorHaloSelectionEffect.h"
using namespace FocalEngine;

class FEEditor;

class FEEditorSelectedObject
{
	friend FEEditor;

public:
	SINGLETON_PUBLIC_PART(FEEditorSelectedObject)
	SINGLETON_PRIVATE_PART(FEEditorSelectedObject)

	void initializeResources();
	void setOnUpdateFunc(void(*func)());

	bool isAnyObjectSelected();

	bool getDirtyFlag();
	void setDirtyFlag(bool newValue);

	void setEntity(FEEntity* selected);
	FEEntity* getEntity();
	std::string getEntityName();

	void setTerrain(FETerrain* selected);
	FETerrain* getTerrain();
	std::string getTerrainName();

	void clear();

	glm::dvec3 mouseRay(double mouseX, double mouseY);
	void determineEntityUnderMouse(double mouseX, double mouseY);
	std::vector<std::string> entitiesUnderMouse;

	bool checkForSelectionisNeeded = false;
	int getIndexOfObjectUnderMouse(double mouseX, double mouseY);

	void onCameraUpdate();

	int debugGetLastColorIndex();
private:
	void* obj = nullptr;
	int type = 0;
	bool dirtyFlag = false;

	void(*onUpdateFunc)() = nullptr;

	unsigned char* colorUnderMouse = new unsigned char[3];
	FEFramebuffer* pixelAccurateSelectionFB;
	FEEntity* potentiallySelectedEntity = nullptr;
	FEMaterial* pixelAccurateSelectionMaterial;

	int colorIndex = -1;
};

#define SELECTED FEEditorSelectedObject::getInstance()