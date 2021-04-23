#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class DragAndDropManager;

class DragAndDropTarget
{
	friend DragAndDropManager;
private:
	std::vector<FEObjectType> acceptedTypes;
	std::vector<std::string> toolTipTexts;
	bool (*callback)(FEObject*, void**);
	void** userData = nullptr;
	bool active = false;
public:
	DragAndDropTarget();
	DragAndDropTarget(FEObjectType acceptedType, bool (*callback)(FEObject*, void**), void** userData = nullptr, std::string toolTipText = "");

	~DragAndDropTarget();

	void setActive(bool active);
	bool getActive();

	bool accept(FEObject* object);

	void** getUserData();
	void setNewUserData(void** newUserData);

	void stickToItem();
	void stickToCurrentWindow();
};

class DragAndDropManager
{
	friend DragAndDropTarget;
public:
	SINGLETON_PUBLIC_PART(DragAndDropManager)

	void initializeResources();
	DragAndDropTarget* addTarget(FEObjectType acceptedType, bool (*callback)(FEObject*, void**), void** userData = nullptr, std::string toolTipText = "");
	DragAndDropTarget* addTarget(DragAndDropTarget* newTarget);

	void render();
	void dropAction();
	void mouseMove();

	void setObject(FEObject* obj, FETexture* texture = nullptr, ImVec2 uv0 = ImVec2(0.0f, 1.0f), ImVec2 uv1 = ImVec2(0.0f, 1.0f));
	bool objectIsDraged();
	//FEObject* getObject();

	FETexture* getToolTipTexture();
private:
	SINGLETON_PRIVATE_PART(DragAndDropManager)

	std::vector<DragAndDropTarget*> targets;

	FEObject* object = nullptr;
	FETexture* previewTexture = nullptr;
	ImVec2 uv0;
	ImVec2 uv1;

	FETexture* handCursor = nullptr;
	FETexture* handCursorUnavailable = nullptr;
	void drawToolTip();
	bool drawDragAndDropHasAction = false;

	std::string getToolTipText();
	bool objectCanBeDroped();
};


#define DRAG_AND_DROP_MANAGER DragAndDropManager::getInstance()