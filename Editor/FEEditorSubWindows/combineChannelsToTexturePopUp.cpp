#include "combineChannelsToTexturePopUp.h"

CombineChannelsToTexturePopUp* CombineChannelsToTexturePopUp::Instance = nullptr;
ImVec2 CombineChannelsToTexturePopUp::NodeGridRelativePosition = ImVec2(5, 30);
ImVec2 CombineChannelsToTexturePopUp::WindowPosition = ImVec2(0, 0);
ImVec2 CombineChannelsToTexturePopUp::MousePositionWhenContextMenuWasOpened = ImVec2(0, 0);
FETexture* CombineChannelsToTexturePopUp::TextureForNewNode = nullptr;
FEVisualNodeArea* CombineChannelsToTexturePopUp::CurrentNodeArea = nullptr;

CombineChannelsToTexturePopUp::CombineChannelsToTexturePopUp()
{
	const std::string TempCaption = "Choose what channels/textures to combine";
	strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());

	NodeAreaTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_TEXTURE, DragAndDropnodeAreaTargetCallback, reinterpret_cast<void**>(&DragAndDropCallbackInfo), "Drop to add texture");
}

CombineChannelsToTexturePopUp::~CombineChannelsToTexturePopUp()
{
}

void CombineChannelsToTexturePopUp::Show()
{
	Size = ImVec2(800, 800);
	Position = ImVec2(FEngine::getInstance().GetWindowWidth() / 2 - Size.x / 2, FEngine::getInstance().GetWindowHeight() / 2 - Size.y / 2);

	Flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

	TextureNode = new FEEditorTextureCreatingNode();
	TextureNode->SetName("New Texture");

	ImVec2 PositionOnCanvas;
	PositionOnCanvas.x = NodeGridRelativePosition.x + Size.x / 2 - TextureNode->GetSize().x / 2.0f;
	PositionOnCanvas.y = NodeGridRelativePosition.y + Size.y / 2 - TextureNode->GetSize().y / 2.0f;

	TextureNode->SetPosition(PositionOnCanvas);
	CurrentNodeArea = NODE_SYSTEM.CreateNodeArea();
	CurrentNodeArea->AddNode(TextureNode);
	CurrentNodeArea->SetMainContextMenuFunc(NodeSystemMainContextMenu);
	
	FEImGuiWindow::Show();
}

void CombineChannelsToTexturePopUp::Render()
{
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	WindowPosition = ImGui::GetWindowPos();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	
	CurrentNodeArea->SetAreaPosition(NodeGridRelativePosition);
	CurrentNodeArea->SetAreaSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 67));
	CurrentNodeArea->Update();

	if (ImGui::GetIO().MouseReleased[1])
		MousePositionWhenContextMenuWasOpened = ImVec2(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	
	NodeAreaTarget->StickToItem();

	const float YPosition = ImGui::GetCursorPosY();
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 8.0f - 120.0f / 2.0f);
	if (ImGui::Button("Combine", ImVec2(120, 0)))
	{
		if (TextureNode->GetTexture() != RESOURCE_MANAGER.NoTexture)
		{
			TextureNode->GetTexture()->SetDirtyFlag(true);
			PROJECT_MANAGER.GetCurrent()->AddUnSavedObject(TextureNode->GetTexture());
			VIRTUAL_FILE_SYSTEM.CreateFile(TextureNode->GetTexture(), VIRTUAL_FILE_SYSTEM.GetCurrentPath());
			NODE_SYSTEM.DeleteNodeArea(CurrentNodeArea);
		}

		FEImGuiWindow::Close();
	}

	ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::ImColor(0.6f, 0.24f, 0.24f)));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::ImColor(0.7f, 0.21f, 0.21f)));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::ImColor(0.8f, 0.16f, 0.16f)));

	ImGui::SetCursorPosY(YPosition);
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f + ImGui::GetWindowWidth() / 8.0f - 120.0f / 2.0f);
	if (ImGui::Button("Cancel", ImVec2(120, 0)))
	{
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

		ImGui::PopStyleVar();

		FEImGuiWindow::Close();
		return;
	}

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	ImGui::PopStyleVar();
	FEImGuiWindow::OnRenderEnd();
}

bool CombineChannelsToTexturePopUp::DragAndDropnodeAreaTargetCallback(FEObject* Object, void** CallbackInfo)
{
	FEEditorTextureSourceNode* NewNode = new FEEditorTextureSourceNode(RESOURCE_MANAGER.GetTexture(Object->GetObjectID()));
	
	ImVec2 PositionOnCanvas;
	PositionOnCanvas.x = ImGui::GetMousePos().x - (WindowPosition.x + NodeGridRelativePosition.x) - NewNode->GetSize().x / 2.0f;
	PositionOnCanvas.y = ImGui::GetMousePos().y - (WindowPosition.y + NodeGridRelativePosition.y) - NewNode->GetSize().y / 2.0f;

	NewNode->SetPosition(PositionOnCanvas);
	CurrentNodeArea->AddNode(NewNode);
	return true;
}

void CombineChannelsToTexturePopUp::NodeSystemMainContextMenu()
{
	if (ImGui::BeginMenu("Add"))
	{
		if (ImGui::MenuItem("Texture node"))
		{
			TextureForNewNode = RESOURCE_MANAGER.NoTexture;
			SelectFeObjectPopUp::getInstance().Show(FE_TEXTURE, TextureNodeCreationCallback);
		}

		if (ImGui::MenuItem("Float node"))
		{
			FEEditorFloatSourceNode* NewNode = new FEEditorFloatSourceNode();

			ImVec2 PositionOnCanvas;
			PositionOnCanvas.x = MousePositionWhenContextMenuWasOpened.x - (WindowPosition.x + NodeGridRelativePosition.x) - NewNode->GetSize().x / 2.0f;
			PositionOnCanvas.y = MousePositionWhenContextMenuWasOpened.y - (WindowPosition.y + NodeGridRelativePosition.y) - NewNode->GetSize().y / 2.0f;

			NewNode->SetPosition(PositionOnCanvas);
			CurrentNodeArea->AddNode(NewNode);
		}

		ImGui::EndMenu();
	}
}

void CombineChannelsToTexturePopUp::TextureNodeCreationCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() != 1 && SelectionsResult[0]->GetType() != FE_TEXTURE)
		return;

	if (SelectionsResult[0] != nullptr && SelectionsResult[0] != RESOURCE_MANAGER.NoTexture)
	{
		FEEditorTextureSourceNode* NewNode = new FEEditorTextureSourceNode(reinterpret_cast<FETexture*>(SelectionsResult[0]));

		ImVec2 PositionOnCanvas;
		PositionOnCanvas.x = MousePositionWhenContextMenuWasOpened.x - (WindowPosition.x + NodeGridRelativePosition.x) - NewNode->GetSize().x / 2.0f;
		PositionOnCanvas.y = MousePositionWhenContextMenuWasOpened.y - (WindowPosition.y + NodeGridRelativePosition.y) - NewNode->GetSize().y / 2.0f;

		NewNode->SetPosition(PositionOnCanvas);
		CurrentNodeArea->AddNode(NewNode);
	}
}