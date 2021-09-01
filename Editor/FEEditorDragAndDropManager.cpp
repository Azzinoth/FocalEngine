#include "FEEditorDragAndDropManager.h"
using namespace FocalEngine;

DragAndDropManager* DragAndDropManager::_instance = nullptr;
DragAndDropManager::DragAndDropManager() {}
DragAndDropManager::~DragAndDropManager() {}

void DragAndDropManager::initializeResources()
{
	handCursor = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/handCursor.png", "handCursor");
	RESOURCE_MANAGER.makeTextureStandard(handCursor);

	handCursorUnavailable = RESOURCE_MANAGER.LoadPNGTexture("Editor/Images/handCursorUnavailable.png", "handCursorUnavailable");
	RESOURCE_MANAGER.makeTextureStandard(handCursorUnavailable);
}

DragAndDropTarget* DragAndDropManager::addTarget(FEObjectType acceptedType, bool (*callback)(FEObject*, void**), void** userData, std::string toolTipText)
{
	targets.push_back(new DragAndDropTarget(acceptedType, callback, userData, toolTipText));
	return targets.back();
}

DragAndDropTarget* DragAndDropManager::addTarget(std::vector<FEObjectType>& acceptedTypes, bool (*callback)(FEObject*, void**), void** userData, std::vector<std::string>& toolTipTexts)
{
	targets.push_back(new DragAndDropTarget(acceptedTypes, callback, userData, toolTipTexts));
	return targets.back();
}

DragAndDropTarget* DragAndDropManager::addTarget(DragAndDropTarget* newTarget)
{
	targets.push_back(newTarget);
	return targets.back();
}

void DragAndDropManager::drawToolTip()
{
	ImGui::Begin("dragAndDrop info", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);

	std::string actionText = getToolTipText();

	ImGui::Text(actionText.c_str());
	ImGui::TextUnformatted(("Name: " + object->getName() + "\nType: " + FEObjectTypeToString(object->getType())).c_str());
	if (previewTexture != nullptr)
		ImGui::Image((void*)(intptr_t)previewTexture->getTextureID(), ImVec2(128, 128), uv0, uv1);

	ImGui::End();
}

std::string DragAndDropManager::getToolTipText()
{
	std::string result = "No action available";
	if (object == nullptr)
		return result;

	for (size_t i = 0; i < targets.size(); i++)
	{
		if (targets[i]->getActive())
		{
			for (size_t j = 0; j < targets[i]->acceptedTypes.size(); j++)
			{
				if (targets[i]->acceptedTypes[j] == object->getType())
				{
					result = targets[i]->toolTipTexts[j];
					break;
				}
			}
		}

		if (result != "No action available")
			break;
	}

	return result;
}

void DragAndDropManager::render()
{
	if (object == nullptr)
		return;

	ImGui::SetMouseCursor(ImGuiMouseCursor_None);

	auto cursor = ImGui::GetCurrentContext()->IO.MousePos;
	const auto offset = ImGui::GetItemRectMin();
	const auto center = ImVec2(0, 0);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::SetNextWindowPos(cursor);
	auto oldImGuiCol_PopupBg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	auto oldImGuiCol_Border = ImGui::GetStyle().Colors[ImGuiCol_Border];
	ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

	ImGui::BeginTooltip();

	if (objectCanBeDroped())
	{
		ImGui::Image((void*)(intptr_t)handCursor->getTextureID(), ImVec2(32, 32), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}
	else
	{
		ImGui::Image((void*)(intptr_t)handCursorUnavailable->getTextureID(), ImVec2(32, 32), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
	}
	ImGui::End();

	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = oldImGuiCol_PopupBg;
	ImGui::GetStyle().Colors[ImGuiCol_Border] = oldImGuiCol_Border;
	ImGui::PopStyleVar();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	ImVec2 newPosition = cursor;
	newPosition.x += 0;
	newPosition.y += 48;
	ImGui::SetNextWindowPos(newPosition);

	drawToolTip();

	for (size_t i = 0; i < targets.size(); i++)
		targets[i]->active = false;

	ImGui::PopStyleVar();
}

void DragAndDropManager::dropAction()
{
	if (object == nullptr)
		return;

	for (size_t i = 0; i < targets.size(); i++)
	{
		if (targets[i]->active && targets[i]->accept(object))
		{
			targets[i]->callback(object, targets[i]->userData);
			break;
		}
	}

	object = nullptr;
}

bool DragAndDropManager::objectCanBeDroped()
{
	for (size_t i = 0; i < targets.size(); i++)
	{
		if (targets[i]->active && targets[i]->accept(object))
		{
			return true;
		}
	}

	return false;
}

void DragAndDropManager::mouseMove()
{
	drawDragAndDropHasAction = false;
	if (object)
	{
		for (size_t i = 0; i < targets.size(); i++)
		{
			if (targets[i]->getActive() && targets[i]->accept(object))
			{
				drawDragAndDropHasAction = true;
				return;
			}
		}
	}
}

void DragAndDropManager::setObject(FEObject* obj, FETexture* texture, ImVec2 uv0, ImVec2 uv1)
{
	object = obj;
	previewTexture = texture;
	this->uv0 = uv0;
	this->uv1 = uv1;
}

FETexture* DragAndDropManager::getToolTipTexture()
{
	return previewTexture;
}

bool DragAndDropManager::objectIsDraged()
{
	return object != nullptr;
}

DragAndDropTarget::DragAndDropTarget()
{
	this->callback = nullptr;
}

DragAndDropTarget::DragAndDropTarget(FEObjectType acceptedType, bool (*callback)(FEObject*, void**), void** userData, std::string toolTipText)
{
	acceptedTypes.push_back(acceptedType);
	this->callback = callback;
	this->userData = userData;
	toolTipTexts.push_back(toolTipText);
}

DragAndDropTarget::DragAndDropTarget(std::vector<FEObjectType>& acceptedTypes, bool (*callback)(FEObject*, void**), void** userData, std::vector<std::string>& toolTipTexts)
{
	this->acceptedTypes = acceptedTypes;
	this->callback = callback;
	this->userData = userData;
	this->toolTipTexts = toolTipTexts;
}

DragAndDropTarget::~DragAndDropTarget()
{
	for (size_t i = 0; i < DRAG_AND_DROP_MANAGER.targets.size(); i++)
	{
		if (DRAG_AND_DROP_MANAGER.targets[i] == this)
		{
			DRAG_AND_DROP_MANAGER.targets.erase(DRAG_AND_DROP_MANAGER.targets.begin() + i);
			return;
		}
	}
}

void DragAndDropTarget::setActive(bool active)
{
	this->active = active;
}

bool DragAndDropTarget::getActive()
{
	return this->active;
}

bool DragAndDropTarget::accept(FEObject* object)
{
	for (size_t i = 0; i < acceptedTypes.size(); i++)
	{
		if (acceptedTypes[i] == object->getType())
			return true;
	}

	return false;
}

void** DragAndDropTarget::getUserData()
{
	return userData;
}

void DragAndDropTarget::setNewUserData(void** newUserData)
{
	userData = newUserData;
}

void DragAndDropTarget::stickToItem()
{
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		setActive(true);
}

void DragAndDropTarget::stickToCurrentWindow()
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		setActive(true);
}