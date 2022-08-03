#pragma once

#include "../FEngine.h"
using namespace FocalEngine;

class DragAndDropManager;

class DragAndDropTarget
{
	friend DragAndDropManager;
private:
	std::vector<FE_OBJECT_TYPE> AcceptedTypes;
	std::vector<std::string> ToolTipTexts;
	bool (*Callback)(FEObject*, void**);
	void** UserData = nullptr;
	bool bActive = false;
public:
	DragAndDropTarget();
	DragAndDropTarget(FE_OBJECT_TYPE AcceptedType, bool (*Callback)(FEObject*, void**), void** UserData = nullptr, std::string ToolTipText = "");
	DragAndDropTarget(std::vector<FE_OBJECT_TYPE>& AcceptedTypes, bool (*Callback)(FEObject*, void**), void** UserData, std::vector<std::string>& ToolTipTexts);

	~DragAndDropTarget();

	void SetActive(bool Active);
	bool GetActive() const;

	bool Accept(const FEObject* Object) const;

	void** GetUserData() const;
	void SetNewUserData(void** NewUserData);

	void StickToItem();
	void StickToCurrentWindow();
};

class DragAndDropManager
{
	friend DragAndDropTarget;
public:
	SINGLETON_PUBLIC_PART(DragAndDropManager)

	void InitializeResources();
	DragAndDropTarget* AddTarget(FE_OBJECT_TYPE AcceptedType, bool (*Callback)(FEObject*, void**), void** UserData = nullptr, std::string ToolTipText = "");
	DragAndDropTarget* AddTarget(std::vector<FE_OBJECT_TYPE>& AcceptedTypes, bool (*Callback)(FEObject*, void**), void** UserData, std::vector<std::string>& ToolTipTexts);
	DragAndDropTarget* AddTarget(DragAndDropTarget* NewTarget);

	void Render() const;
	void DropAction();
	void MouseMove();

	void SetObject(FEObject* Obj, FETexture* Texture = nullptr, ImVec2 UV0 = ImVec2(0.0f, 1.0f), ImVec2 UV1 = ImVec2(0.0f, 1.0f));
	bool ObjectIsDraged() const;

	FETexture* GetToolTipTexture() const;
private:
	SINGLETON_PRIVATE_PART(DragAndDropManager)

	std::vector<DragAndDropTarget*> Targets;

	FEObject* Object = nullptr;
	FETexture* PreviewTexture = nullptr;
	ImVec2 UV0;
	ImVec2 UV1;

	FETexture* HandCursor = nullptr;
	FETexture* HandCursorUnavailable = nullptr;
	void DrawToolTip() const;
	bool bDrawDragAndDropHasAction = false;

	std::string GetToolTipText() const;
	bool ObjectCanBeDroped() const;
};

#define DRAG_AND_DROP_MANAGER DragAndDropManager::getInstance()