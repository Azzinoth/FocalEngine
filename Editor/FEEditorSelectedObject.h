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

	void InitializeResources();
	void ReInitializeResources();
	void SetOnUpdateFunc(void(*Func)());

	FEObject* GetSelected() const;

	FEEntity* GetEntity() const;
	FETerrain* GetTerrain() const;
	FELight* GetLight() const;

	/*template <class T>
	T* getSelected(FEObjectType type);*/

	bool GetDirtyFlag() const;
	void SetDirtyFlag(bool NewValue);

	void SetSelected(FEObject* SelectedObject);
	void Clear();

	glm::dvec3 MouseRay(double MouseX, double MouseY) const;
	void DetermineEntityUnderMouse(double MouseX, double MouseY);
	std::vector<FEObject*> ObjectsUnderMouse;

	bool CheckForSelectionisNeeded = false;
	int GetIndexOfObjectUnderMouse(double MouseX, double MouseY);

	void OnCameraUpdate() const;

	int DebugGetLastColorIndex() const;
	std::unordered_map<FEEntityInstanced*, std::vector<int>> InstancedSubObjectsInfo;
	int InstancedSubObjectIndexSelected = -1;
	void SetSelectedByIndex(size_t Index);

	FEShader* FEPixelAccurateInstancedSelection = nullptr; 
	FEShader* FEPixelAccurateSelection = nullptr;
private:
	FEObject* Container = nullptr;
	
	bool bDirtyFlag = false;

	void(*OnUpdateFunc)() = nullptr;
#ifdef EDITOR_SELECTION_DEBUG_MODE
public:
#endif
	unsigned char* ColorUnderMouse = new unsigned char[3];
	FEFramebuffer* PixelAccurateSelectionFB;
	int ColorIndex = -1;
#ifdef EDITOR_SELECTION_DEBUG_MODE
private:
#endif
	FEEntity* PotentiallySelectedEntity = nullptr;
	FEMaterial* PixelAccurateSelectionMaterial = nullptr;
};

#define SELECTED FEEditorSelectedObject::getInstance()