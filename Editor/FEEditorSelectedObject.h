#pragma once

#include "FEEditorHaloSelectionEffect.h"
using namespace FocalEngine;

class FEEditor;

enum FEEditorSelectedObjectType
{
	SELECTED_NULL = 0,
	SELECTED_ENTITY = 1,
	SELECTED_TERRAIN = 2,
	SELECTED_ENTITY_INSTANCED = 3,
	SELECTED_ENTITY_INSTANCED_SUBOBJECT = 4
};

struct selectedObject
{
	FEEditorSelectedObjectType type = SELECTED_NULL;
	void* objPointer = nullptr;
	int additionalInformation = -1;

	selectedObject(FEEditorSelectedObjectType Type = SELECTED_NULL, void* ObjPointer = nullptr) : type(Type), objPointer(ObjPointer) {};
};

class FEEditorSelectedObject
{
	friend FEEditor;

public:
	SINGLETON_PUBLIC_PART(FEEditorSelectedObject)
	SINGLETON_PRIVATE_PART(FEEditorSelectedObject)

	void initializeResources();
	void reInitializeResources();
	void setOnUpdateFunc(void(*func)());

	bool isAnyObjectSelected();

	bool getDirtyFlag();
	void setDirtyFlag(bool newValue);

	void setEntity(FEEntity* selected);
	void setEntity(FEEntityInstanced* selected);
	FEEntity* getEntity();
	std::string getEntityName();

	void setTerrain(FETerrain* selected);
	FETerrain* getTerrain();
	std::string getTerrainName();

	void clear();

	glm::dvec3 mouseRay(double mouseX, double mouseY);
	void determineEntityUnderMouse(double mouseX, double mouseY);
	std::vector<selectedObject> objectsUnderMouse;

	bool checkForSelectionisNeeded = false;
	int getIndexOfObjectUnderMouse(double mouseX, double mouseY);

	void onCameraUpdate();

	int debugGetLastColorIndex();
	FEEditorSelectedObjectType getType();
	//void setEntityInstancedSubObject(FEEntityInstanced* selected, int subObjectIndex);
	void* getBareObject();
	int getAdditionalInformation();
	void setSelectedByIndex(size_t index);
private:
	selectedObject container;
	bool dirtyFlag = false;

	void(*onUpdateFunc)() = nullptr;

	unsigned char* colorUnderMouse = new unsigned char[3];
	FEFramebuffer* pixelAccurateSelectionFB;
	FEEntity* potentiallySelectedEntity = nullptr;
	FEMaterial* pixelAccurateSelectionMaterial;

	int colorIndex = -1;
};

#define SELECTED FEEditorSelectedObject::getInstance()