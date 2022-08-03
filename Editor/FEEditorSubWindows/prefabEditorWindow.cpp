#include "prefabEditorWindow.h"

PrefabEditorWindow* PrefabEditorWindow::Instance = nullptr;
FEPrefab* PrefabEditorWindow::ObjToWorkWith = nullptr;

bool PrefabEditorWindow::AddGameModelTargetCallBack(FEObject* Object, void** EntityPointer)
{
	if (Object == nullptr)
		return false;

	FEGameModel* NewGameModel = reinterpret_cast<FEGameModel*>(Object);
	ObjToWorkWith->AddComponent(NewGameModel);

	return true;
}

void PrefabEditorWindow::ShowTransformConfiguration(FETransformComponent* Transform, const int Index)
{
	// ********************* POSITION *********************
	glm::vec3 position = Transform->GetPosition();
	ImGui::Text("Position : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X pos : ") + std::to_string(Index)).c_str(), &position[0], 0.1f);
	ShowToolTip("X position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y pos : ") + std::to_string(Index)).c_str(), &position[1], 0.1f);
	ShowToolTip("Y position");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z pos : ") + std::to_string(Index)).c_str(), &position[2], 0.1f);
	ShowToolTip("Z position");
	Transform->SetPosition(position);

	// ********************* ROTATION *********************
	glm::vec3 rotation = Transform->GetRotation();
	ImGui::Text("Rotation : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X rot : ") + std::to_string(Index)).c_str(), &rotation[0], 0.1f, -360.0f, 360.0f);
	ShowToolTip("X rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y rot : ") + std::to_string(Index)).c_str(), &rotation[1], 0.1f, -360.0f, 360.0f);
	ShowToolTip("Y rotation");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z rot : ") + std::to_string(Index)).c_str(), &rotation[2], 0.1f, -360.0f, 360.0f);
	ShowToolTip("Z rotation");
	Transform->SetRotation(rotation);

	// ********************* SCALE *********************
	bool bUniformScaling = Transform->IsUniformScalingSet();
	ImGui::Checkbox("Uniform scaling", &bUniformScaling);
	Transform->SetUniformScaling(bUniformScaling);

	glm::vec3 scale = Transform->GetScale();
	ImGui::Text("Scale : ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##X scale : ") + std::to_string(Index)).c_str(), &scale[0], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("X scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Y scale : ") + std::to_string(Index)).c_str(), &scale[1], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("Y scale");

	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::DragFloat((std::string("##Z scale : ") + std::to_string(Index)).c_str(), &scale[2], 0.01f, 0.01f, 1000.0f);
	ShowToolTip("Z scale");

	glm::vec3 OldScale = Transform->GetScale();
	Transform->ChangeXScaleBy(scale[0] - OldScale[0]);
	Transform->ChangeYScaleBy(scale[1] - OldScale[1]);
	Transform->ChangeZScaleBy(scale[2] - OldScale[2]);
}

void PrefabEditorWindow::AddNewGameModelCallBack(const std::vector<FEObject*> SelectionsResult)
{
	for (int i = 0; i < SelectionsResult.size(); i++)
	{
		if (SelectionsResult[i]->GetType() == FE_GAMEMODEL)
			ObjToWorkWith->AddComponent(reinterpret_cast<FEGameModel*>(SelectionsResult[i]));
	}	
}

PrefabEditorWindow::PrefabEditorWindow()
{
	const std::string TempCaption = "Edit Prefab";
	strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());

	CloseButton = new ImGuiButton("Close");
	CloseButton->SetSize(ImVec2(140, 24));
	CloseButton->SetPosition(ImVec2(800.0 / 2.0 - 140.0 / 2.0, 800.0 - 35.0));

	//addGameModelTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_GAMEMODEL, addGameModelTargetCallBack, nullptr, "Drop to add Game model");
}

PrefabEditorWindow::~PrefabEditorWindow()
{
	delete CloseButton;
}

void PrefabEditorWindow::Show(FEPrefab* Prefab)
{
	Size = ImVec2(800, 800);
	Position = ImVec2(FEngine::getInstance().GetWindowWidth() / 2 - Size.x / 2, FEngine::getInstance().GetWindowHeight() / 2 - Size.y / 2);

	Flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

	ObjToWorkWith = Prefab;
	
	FEImGuiWindow::Show();
}

void PrefabEditorWindow::Render()
{
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	if (ImGui::BeginTabBar("##prefabSettings", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Game Models"))
		{
			static bool ContextMenuOpened = false;

			ImGui::BeginChildFrame(ImGui::GetID("GameModels ListBox Child"), ImVec2(ImGui::GetWindowContentRegionWidth() - 10.0f, 500.0f), ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			bool bListBoxHovered = false;
			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
			{
				bListBoxHovered = true;
				//addGameModelTarget->setActive(true);
			}

			static bool bOpenContextMenu = false;
			if (ImGui::IsMouseClicked(1))
			{
				if (bListBoxHovered)
				{
					bOpenContextMenu = true;
				}
			}

			if (ImGui::BeginListBox("##GameModels ListBox", ImVec2(ImGui::GetWindowContentRegionWidth() - 10.0f, 500.0f)))
			{
				ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

				for (int i = 0; i < ObjToWorkWith->ComponentsCount(); i++)
				{
					FEGameModel* GameModel = ObjToWorkWith->GetComponent(i)->GameModel;
					if (GameModel == nullptr)
						break;

					ImVec2 PostionBeforeDraw = ImGui::GetCursorPos();

					const ImVec2 TextSize = ImGui::CalcTextSize(GameModel->GetName().c_str());
					ImGui::SetCursorPos(PostionBeforeDraw + ImVec2(ImGui::GetWindowContentRegionWidth() / 2.0f - TextSize.x / 2.0f, 16));
					ImGui::Text(GameModel->GetName().c_str());
					ImGui::SetCursorPos(PostionBeforeDraw);

					ImGui::PushID(i);
					if (ImGui::Selectable("##item", SelectedGameModel == i ? true : false, ImGuiSelectableFlags_None, ImVec2(ImGui::GetWindowContentRegionWidth() - 0, 64)))
					{
						SelectedGameModel = i;
					}
					ImGui::PopID();

					if (ImGui::IsItemHovered())
						HoveredGameModelItem = i;

					ImGui::SetCursorPos(PostionBeforeDraw);
					ImColor ImageTint = ImGui::IsItemHovered() ? ImColor(1.0f, 1.0f, 1.0f, 0.5f) : ImColor(1.0f, 1.0f, 1.0f, 1.0f);
					FETexture* PreviewTexture = PREVIEW_MANAGER.GetPreview(GameModel->GetObjectID());
					ImGui::Image((void*)static_cast<intptr_t>(PreviewTexture->GetTextureID()), ImVec2(64, 64), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f), ImageTint);


					ShowTransformConfiguration(&ObjToWorkWith->GetComponent(i)->Transform, i);
				}

				ImGui::EndListBox();
				ImGui::PopFont();
			}
			

			ImGui::EndChildFrame();
			ImGui::EndTabItem();

			if (bOpenContextMenu)
			{
				bOpenContextMenu = false;
				ImGui::OpenPopup("##layers_listBox_context_menu");
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
			if (ImGui::BeginPopup("##layers_listBox_context_menu"))
			{
				ContextMenuOpened = true;

				if (ImGui::MenuItem("Add game model..."))
				{
					SelectFeObjectPopUp::getInstance().Show(FE_GAMEMODEL, AddNewGameModelCallBack);
				}

				if (HoveredGameModelItem != -1)
				{
					FEGameModel* GameModel = ObjToWorkWith->GetComponent(HoveredGameModelItem)->GameModel;
					if (GameModel != nullptr)
					{
						ImGui::Separator();
						const std::string LayerName = GameModel->GetName();
						ImGui::Text((std::string("Actions with ") + LayerName).c_str());
						ImGui::Separator();

						if (ImGui::MenuItem("Remove"))
						{
							ObjToWorkWith->RemoveComponent(HoveredGameModelItem);
						}
					}
				}

				ImGui::EndPopup();
			}
			ImGui::PopStyleVar();

			if (!ContextMenuOpened)
				HoveredGameModelItem = -1;

			ContextMenuOpened = false;
		}
		ImGui::EndTabBar();
	}

	CloseButton->Render();
	if (CloseButton->IsClicked())
		Close();
	
	FEImGuiWindow::OnRenderEnd();
}