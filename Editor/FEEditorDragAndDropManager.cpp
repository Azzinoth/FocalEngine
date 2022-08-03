#include "FEEditorDragAndDropManager.h"
using namespace FocalEngine;

DragAndDropManager* DragAndDropManager::Instance = nullptr;
DragAndDropManager::DragAndDropManager() {}
DragAndDropManager::~DragAndDropManager() {}

void DragAndDropManager::InitializeResources()
{
	HandCursor = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/handCursor.png", "handCursor");
	RESOURCE_MANAGER.MakeTextureStandard(HandCursor);

	HandCursorUnavailable = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/handCursorUnavailable.png", "handCursorUnavailable");
	RESOURCE_MANAGER.MakeTextureStandard(HandCursorUnavailable);
}

DragAndDropTarget* DragAndDropManager::AddTarget(const FE_OBJECT_TYPE AcceptedType, bool (*Callback)(FEObject*, void**), void** UserData, const std::string ToolTipText)
{
	Targets.push_back(new DragAndDropTarget(AcceptedType, Callback, UserData, ToolTipText));
	return Targets.back();
}

DragAndDropTarget* DragAndDropManager::AddTarget(std::vector<FE_OBJECT_TYPE>& AcceptedTypes, bool (*Callback)(FEObject*, void**), void** UserData, std::vector<std::string>& ToolTipTexts)
{
	Targets.push_back(new DragAndDropTarget(AcceptedTypes, Callback, UserData, ToolTipTexts));
	return Targets.back();
}

DragAndDropTarget* DragAndDropManager::AddTarget(DragAndDropTarget* NewTarget)
{
	Targets.push_back(NewTarget);
	return Targets.back();
}

void DragAndDropManager::DrawToolTip() const
{
	ImGui::Begin("dragAndDrop info", nullptr, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

	const std::string ActionText = GetToolTipText();

	ImGui::Text(ActionText.c_str());
	ImGui::TextUnformatted(("Name: " + Object->GetName() + "\nType: " + FEObjectTypeToString(Object->GetType())).c_str());
	if (PreviewTexture != nullptr)
		ImGui::Image((void*)(intptr_t)PreviewTexture->GetTextureID(), ImVec2(128, 128), UV0, UV1);

	ImGui::End();
}

std::string DragAndDropManager::GetToolTipText() const
{
	std::string Result = "No action available";
	if (Object == nullptr)
		return Result;

	for (size_t i = 0; i < Targets.size(); i++)
	{
		if (Targets[i]->GetActive())
		{
			for (size_t j = 0; j < Targets[i]->AcceptedTypes.size(); j++)
			{
				if (Targets[i]->AcceptedTypes[j] == Object->GetType())
				{
					Result = Targets[i]->ToolTipTexts[j];
					break;
				}
			}
		}

		if (Result != "No action available")
			break;
	}

	return Result;
}

void DragAndDropManager::Render() const
{
	if (Object == nullptr)
		return;

	ImGui::SetMouseCursor(ImGuiMouseCursor_None);

	const auto Cursor = ImGui::GetCurrentContext()->IO.MousePos;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(Cursor);
	const auto OldImGuiColPopupBg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	const auto OldImGuiColBorder = ImGui::GetStyle().Colors[ImGuiCol_Border];
	ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

	ImGui::BeginTooltip();

	if (ObjectCanBeDroped())
	{
		ImGui::Image((void*)(intptr_t)HandCursor->GetTextureID(), ImVec2(32, 32), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}
	else
	{
		ImGui::Image((void*)(intptr_t)HandCursorUnavailable->GetTextureID(), ImVec2(32, 32), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}
	ImGui::End();

	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = OldImGuiColPopupBg;
	ImGui::GetStyle().Colors[ImGuiCol_Border] = OldImGuiColBorder;
	ImGui::PopStyleVar();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	ImVec2 NewPosition = Cursor;
	NewPosition.x += 0;
	NewPosition.y += 48;
	ImGui::SetNextWindowPos(NewPosition);

	DrawToolTip();

	for (size_t i = 0; i < Targets.size(); i++)
		Targets[i]->bActive = false;

	ImGui::PopStyleVar();
}

void DragAndDropManager::DropAction()
{
	if (Object == nullptr)
		return;

	for (size_t i = 0; i < Targets.size(); i++)
	{
		if (Targets[i]->bActive && Targets[i]->Accept(Object))
		{
			Targets[i]->Callback(Object, Targets[i]->UserData);
			break;
		}
	}

	Object = nullptr;
}

bool DragAndDropManager::ObjectCanBeDroped() const
{
	for (size_t i = 0; i < Targets.size(); i++)
	{
		if (Targets[i]->bActive && Targets[i]->Accept(Object))
		{
			return true;
		}
	}

	return false;
}

void DragAndDropManager::MouseMove()
{
	bDrawDragAndDropHasAction = false;
	if (Object)
	{
		for (size_t i = 0; i < Targets.size(); i++)
		{
			if (Targets[i]->GetActive() && Targets[i]->Accept(Object))
			{
				bDrawDragAndDropHasAction = true;
				return;
			}
		}
	}
}

void DragAndDropManager::SetObject(FEObject* Obj, FETexture* Texture, ImVec2 UV0, ImVec2 UV1)
{
	Object = Obj;
	PreviewTexture = Texture;
	this->UV0 = UV0;
	this->UV1 = UV1;
}

FETexture* DragAndDropManager::GetToolTipTexture() const
{
	return PreviewTexture;
}

bool DragAndDropManager::ObjectIsDraged() const
{
	return Object != nullptr;
}

DragAndDropTarget::DragAndDropTarget()
{
	this->Callback = nullptr;
}

DragAndDropTarget::DragAndDropTarget(const FE_OBJECT_TYPE AcceptedType, bool (*Callback)(FEObject*, void**), void** UserData, const std::string ToolTipText)
{
	AcceptedTypes.push_back(AcceptedType);
	this->Callback = Callback;
	this->UserData = UserData;
	ToolTipTexts.push_back(ToolTipText);
}

DragAndDropTarget::DragAndDropTarget(std::vector<FE_OBJECT_TYPE>& AcceptedTypes, bool (*Callback)(FEObject*, void**), void** UserData, std::vector<std::string>& ToolTipTexts)
{
	this->AcceptedTypes = AcceptedTypes;
	this->Callback = Callback;
	this->UserData = UserData;
	this->ToolTipTexts = ToolTipTexts;
}

DragAndDropTarget::~DragAndDropTarget()
{
	for (size_t i = 0; i < DRAG_AND_DROP_MANAGER.Targets.size(); i++)
	{
		if (DRAG_AND_DROP_MANAGER.Targets[i] == this)
		{
			DRAG_AND_DROP_MANAGER.Targets.erase(DRAG_AND_DROP_MANAGER.Targets.begin() + i);
			return;
		}
	}
}

void DragAndDropTarget::SetActive(const bool Active)
{
	this->bActive = Active;
}

bool DragAndDropTarget::GetActive() const
{
	return this->bActive;
}

bool DragAndDropTarget::Accept(const FEObject* Object) const
{
	for (size_t i = 0; i < AcceptedTypes.size(); i++)
	{
		if (AcceptedTypes[i] == Object->GetType())
			return true;
	}

	return false;
}

void** DragAndDropTarget::GetUserData() const
{
	return UserData;
}

void DragAndDropTarget::SetNewUserData(void** NewUserData)
{
	UserData = NewUserData;
}

void DragAndDropTarget::StickToItem()
{
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		SetActive(true);
}

void DragAndDropTarget::StickToCurrentWindow()
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		SetActive(true);
}