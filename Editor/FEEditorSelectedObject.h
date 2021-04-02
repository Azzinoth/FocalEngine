#pragma once

#include "FEEditorHaloSelectionEffect.h"
using namespace FocalEngine;

//#define EDITOR_SELECTION_DEBUG_MODE

class FEEditor;

class FEEditorSelectedObject
{
	friend FEEditor;
	SINGLETON_PRIVATE_PART(FEEditorSelectedObject)
public:
	SINGLETON_PUBLIC_PART(FEEditorSelectedObject)

	void initializeResources();
	void reInitializeResources();
	void setOnUpdateFunc(void(*func)());

	FEObject* getSelected();

	FEEntity* getEntity();
	FETerrain* getTerrain();
	FELight* getLight();

	/*template <class T>
	T* getSelected(FEObjectType type);*/

	bool getDirtyFlag();
	void setDirtyFlag(bool newValue);

	void setSelected(FEObject* selectedObject);
	void clear();

	glm::dvec3 mouseRay(double mouseX, double mouseY);
	void determineEntityUnderMouse(double mouseX, double mouseY);
	std::vector<FEObject*> objectsUnderMouse;

	bool checkForSelectionisNeeded = false;
	int getIndexOfObjectUnderMouse(double mouseX, double mouseY);

	void onCameraUpdate();

	int debugGetLastColorIndex();
	std::unordered_map<FEEntityInstanced*, std::vector<int>> instancedSubObjectsInfo;
	int instancedSubObjectIndexSelected = -1;
	void setSelectedByIndex(size_t index);

	FEShader* FEPixelAccurateInstancedSelection = nullptr; 
	FEShader* FEPixelAccurateSelection = nullptr;
private:
	FEObject* container;
	
	bool dirtyFlag = false;

	void(*onUpdateFunc)() = nullptr;
#ifdef EDITOR_SELECTION_DEBUG_MODE
public:
#endif
	unsigned char* colorUnderMouse = new unsigned char[3];
	FEFramebuffer* pixelAccurateSelectionFB;
	int colorIndex = -1;
#ifdef EDITOR_SELECTION_DEBUG_MODE
private:
#endif
	FEEntity* potentiallySelectedEntity = nullptr;
	FEMaterial* pixelAccurateSelectionMaterial;
};

#define SELECTED FEEditorSelectedObject::getInstance()