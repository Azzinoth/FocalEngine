#include "combineChannelsToTexturePopUp.h"

CombineChannelsToTexturePopUp* CombineChannelsToTexturePopUp::_instance = nullptr;
ImVec2 CombineChannelsToTexturePopUp::nodeGridRelativePosition = ImVec2(5, 30);
ImVec2 CombineChannelsToTexturePopUp::windowPosition = ImVec2(0, 0);
ImVec2 CombineChannelsToTexturePopUp::mousePositionWhenContextMenuWasOpened = ImVec2(0, 0);
FETexture* CombineChannelsToTexturePopUp::textureForNewNode = nullptr;
FEEditorNodeArea* CombineChannelsToTexturePopUp::currentNodeArea = nullptr;

CombineChannelsToTexturePopUp::CombineChannelsToTexturePopUp()
{
	std::string tempCaption = "Choose what channels/textures to combine";
	strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());

	nodeAreaTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_TEXTURE, dragAndDropnodeAreaTargetCallback, reinterpret_cast<void**>(&dragAndDropCallbackInfo), "Drop to add texture");
}

CombineChannelsToTexturePopUp::~CombineChannelsToTexturePopUp()
{
}

void CombineChannelsToTexturePopUp::show(std::string FilePath)
{
	size = ImVec2(800, 800);
	position = ImVec2(FEngine::getInstance().getWindowWidth() / 2 - size.x / 2, FEngine::getInstance().getWindowHeight() / 2 - size.y / 2);

	flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

	textureNode = new FEEditorTextureCreatingNode();
	textureNode->setName("New Texture");

	ImVec2 positionOnCanvas;
	positionOnCanvas.x = nodeGridRelativePosition.x + size.x / 2 - textureNode->getSize().x / 2.0f;
	positionOnCanvas.y = nodeGridRelativePosition.y + size.y / 2 - textureNode->getSize().y / 2.0f;

	textureNode->setPosition(positionOnCanvas);
	currentNodeArea = NODE_SYSTEM.createNodeArea();
	currentNodeArea->addNode(textureNode);
	currentNodeArea->setMainContextMenuFunc(nodeSystemMainContextMenu);
	
	FEImGuiWindow::show();
}

void CombineChannelsToTexturePopUp::render()
{
	FEImGuiWindow::render();

	if (!isVisible())
		return;

	windowPosition = ImGui::GetWindowPos();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	
	currentNodeArea->setAreaPosition(nodeGridRelativePosition);
	currentNodeArea->setAreaSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 67));
	currentNodeArea->update();

	if (ImGui::GetIO().MouseReleased[1])
		mousePositionWhenContextMenuWasOpened = ImVec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	
	nodeAreaTarget->stickToItem();

	float yPosition = ImGui::GetCursorPosY();
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 8.0f - 120.0f / 2.0f);
	if (ImGui::Button("Combine", ImVec2(120, 0)))
	{
		if (textureNode->getTexture() != RESOURCE_MANAGER.noTexture)
		{
			textureNode->getTexture()->setDirtyFlag(true);
			PROJECT_MANAGER.getCurrent()->addUnSavedObject(textureNode->getTexture());
			VIRTUAL_FILE_SYSTEM.createFile(textureNode->getTexture(), VIRTUAL_FILE_SYSTEM.getCurrentPath());
			NODE_SYSTEM.deleteNodeArea(currentNodeArea);
		}

		FEImGuiWindow::close();
	}

	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::ImColor(0.6f, 0.24f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::ImColor(0.7f, 0.21f, 0.21f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::ImColor(0.8f, 0.16f, 0.16f));

	ImGui::SetCursorPosY(yPosition);
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 8.0f - 120.0f / 2.0f);
	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopStyleVar();

		FEImGuiWindow::close();
		return;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PopStyleVar();
	FEImGuiWindow::onRenderEnd();
}

bool CombineChannelsToTexturePopUp::dragAndDropnodeAreaTargetCallback(FEObject* object, void** callbackInfo)
{
	FEEditorTextureSourceNode* newNode = new FEEditorTextureSourceNode(RESOURCE_MANAGER.getTexture(object->getObjectID()));
	
	ImVec2 positionOnCanvas;
	positionOnCanvas.x = ImGui::GetMousePos().x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
	positionOnCanvas.y = ImGui::GetMousePos().y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;

	newNode->setPosition(positionOnCanvas);
	currentNodeArea->addNode(newNode);
	return true;
}

void CombineChannelsToTexturePopUp::nodeSystemMainContextMenu()
{
	if (ImGui::BeginMenu("Add"))
	{
		if (ImGui::MenuItem("Texture node"))
		{
			textureForNewNode = RESOURCE_MANAGER.noTexture;
			selectFEObjectPopUp::getInstance().show(FE_TEXTURE, textureNodeCreationCallback);
		}

		if (ImGui::MenuItem("Float node"))
		{
			FEEditorFloatSourceNode* newNode = new FEEditorFloatSourceNode();

			ImVec2 positionOnCanvas;
			positionOnCanvas.x = mousePositionWhenContextMenuWasOpened.x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
			positionOnCanvas.y = mousePositionWhenContextMenuWasOpened.y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;

			newNode->setPosition(positionOnCanvas);
			currentNodeArea->addNode(newNode);
		}

		ImGui::EndMenu();
	}
}

void CombineChannelsToTexturePopUp::textureNodeCreationCallback(std::vector<FEObject*> selectionsResult)
{
	if (selectionsResult.size() != 1 && selectionsResult[0]->getType() != FE_TEXTURE)
		return;

	if (selectionsResult[0] != nullptr && selectionsResult[0] != RESOURCE_MANAGER.noTexture)
	{
		FEEditorTextureSourceNode* newNode = new FEEditorTextureSourceNode(reinterpret_cast<FETexture*>(selectionsResult[0]));

		ImVec2 positionOnCanvas;
		positionOnCanvas.x = mousePositionWhenContextMenuWasOpened.x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
		positionOnCanvas.y = mousePositionWhenContextMenuWasOpened.y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;

		newNode->setPosition(positionOnCanvas);
		currentNodeArea->addNode(newNode);
	}
}