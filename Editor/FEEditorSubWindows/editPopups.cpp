#include "editPopups.h"

EditGameModelPopup* EditGameModelPopup::Instance = nullptr;
FEMesh** EditGameModelPopup::MeshToModify = nullptr;

FEMaterial** EditGameModelPopup::MaterialToModify = nullptr;
FEMaterial** EditGameModelPopup::BillboardMaterialToModify = nullptr;

void EditGameModelPopup::ChangeMeshCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (MeshToModify == nullptr)
		return;

	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MESH)
	{
		FEMesh* SelectedMesh = RESOURCE_MANAGER.GetMesh(SelectionsResult[0]->GetObjectID());
		if (SelectedMesh == nullptr)
			return;

		*MeshToModify = SelectedMesh;
	}
}

void EditGameModelPopup::ChangeMaterialCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetObjectID());
		if (SelectedMaterial == nullptr)
			return;

		*MaterialToModify = SelectedMaterial;
	}
}

void EditGameModelPopup::ChangeBillboardMaterialCallBack(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() == 1 && SelectionsResult[0]->GetType() == FE_MATERIAL)
	{
		FEMaterial* SelectedMaterial = RESOURCE_MANAGER.GetMaterial(SelectionsResult[0]->GetObjectID());
		if (SelectedMaterial == nullptr)
			return;

		*BillboardMaterialToModify = SelectedMaterial;
	}
}

EditGameModelPopup::EditGameModelPopup()
{
	TempModel = new FEGameModel(nullptr, nullptr, "tempGameModel");
	ObjToWorkWith = nullptr;
	Flags = ImGuiWindowFlags_NoResize;

	CancelButton = new ImGuiButton("Cancel");
	CancelButton->SetDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
	CancelButton->SetHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
	CancelButton->SetActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));

	ApplyButton = new ImGuiButton("Apply");
	ChangeMaterialButton = new ImGuiButton("Change Material");
	ChangeBillboardMaterialButton = new ImGuiButton("Change Billboard Material");
	DeleteLODMeshButton = new ImGuiButton("X");
	AddBillboard = new ImGuiButton("Add Billboard");

	LODGroups = new FERangeConfigurator();
	LODGroups->SetSize(ImVec2(870.0f, 40.0f));
	LODGroups->SetPosition(ImVec2(920.0f / 2.0f - 870.0f / 2.0f, 650.0f));

	LODColors.push_back(ImColor(0, 255, 0, 255));
	LODColors.push_back(ImColor(0, 0, 255, 255));
	LODColors.push_back(ImColor(0, 255, 255, 255));
	LODColors.push_back(ImColor(0, 255, 125, 255));
}

EditGameModelPopup::~EditGameModelPopup()
{
	delete CancelButton;
	delete ApplyButton;
	delete ChangeMaterialButton;
	delete ChangeBillboardMaterialButton;

	for (size_t i = 0; i < ChangeLODMeshButton.size(); i++)
	{
		delete ChangeLODMeshButton[i];
	}

	delete DeleteLODMeshButton;
	delete AddBillboard;
	delete LODGroups;
}

void EditGameModelPopup::Show(FEGameModel* GameModel)
{
	if (GameModel != nullptr)
	{
		ObjToWorkWith = GameModel;
		Flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		TempModel->SetMaterial(ObjToWorkWith->GetMaterial());
		TempModel->SetScaleFactor(ObjToWorkWith->GetScaleFactor());
		UpdatedMaterial = ObjToWorkWith->GetMaterial();
		UpdatedBillboardMaterial = ObjToWorkWith->GetBillboardMaterial();
		TempModel->SetUsingLOD(ObjToWorkWith->IsUsingLOD());
		TempModel->SetBillboardZeroRotaion(ObjToWorkWith->GetBillboardZeroRotaion());

		ChangeLODMeshButton.clear();
		if (ObjToWorkWith->IsUsingLOD())
		{
			ChangeLODMeshButton.resize(ObjToWorkWith->GetMaxLODCount());
			for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
			{
				const std::string ButtonName = "Change LOD" + std::to_string(i) + " Mesh";
				ChangeLODMeshButton[i] = new ImGuiButton(ButtonName);
				ChangeLODMeshButton[i]->SetSize(ImVec2(200, 35));
			}
		}
		else
		{
			ChangeLODMeshButton.resize(1);
			ChangeLODMeshButton[0] = new ImGuiButton("Change Mesh");
			ChangeLODMeshButton[0]->SetSize(ImVec2(200, 35));
			ChangeLODMeshButton[0]->SetPosition(ImVec2(Size.x / 2.0f - Size.x / 4.0f - ChangeLODMeshButton[0]->GetSize().x / 2, 35 + 340.0f));
		}

		UpdatedLODMeshs.clear();
		for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
		{
			TempModel->SetLODMesh(i, ObjToWorkWith->GetLODMesh(i));
			TempModel->SetLODMaxDrawDistance(i, ObjToWorkWith->GetLODMaxDrawDistance(i));
			TempModel->SetIsLODBillboard(i, ObjToWorkWith->IsLODBillboard(i));
			UpdatedLODMeshs.push_back(ObjToWorkWith->GetLODMesh(i));
		}
		TempModel->SetCullDistance(ObjToWorkWith->GetCullDistance());

		if (ObjToWorkWith->IsUsingLOD())
		{
			SwitchMode(HAS_LOD_MODE);
		}
		else
		{
			SwitchMode(NO_LOD_MODE);
		}

		std::string TempCaption = "Edit game model:";
		TempCaption += " " + ObjToWorkWith->GetName();
		strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());

		Position = ImVec2(FEngine::getInstance().GetWindowWidth() / 2 - Size.x / 2, FEngine::getInstance().GetWindowHeight() / 2 - Size.y / 2);
		FEImGuiWindow::Show();

		PREVIEW_MANAGER.CreateGameModelPreview(TempModel, &TempPreview);

		ChangeMaterialButton->SetSize(ImVec2(200, 35));
		ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - ChangeMaterialButton->GetSize().x / 2, 35 + 340.0f));
		ChangeBillboardMaterialButton->SetSize(ImVec2(270, 35));
		ChangeBillboardMaterialButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - ChangeBillboardMaterialButton->GetSize().x / 2, 35 + 340.0f));

		DeleteLODMeshButton->SetSize(ImVec2(24, 25));
		DeleteLODMeshButton->SetDefaultColor(ImVec4(0.9f, 0.5f, 0.5f, 1.0f));

		AddBillboard->SetSize(ImVec2(200, 35));

		ApplyButton->SetPosition(ImVec2(Size.x / 4 - ApplyButton->GetSize().x / 2, Size.y - 40));
		CancelButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - CancelButton->GetSize().x / 2, Size.y - 40));

		// ************** LOD Groups **************
		LODGroups->Clear();
		float Previous = 0.0f;
		for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
		{
			if (ObjToWorkWith->GetLODMesh(i) != nullptr)
			{
				LODGroups->AddRange(((ObjToWorkWith->GetLODMaxDrawDistance(i)) - Previous) / ObjToWorkWith->GetCullDistance(), std::string("LOD") + std::to_string(i), "", LODColors[i]);
				Previous = ObjToWorkWith->GetLODMaxDrawDistance(i);
			}
		}
		// ************** LOD Groups END **************

		// ************** Drag&Drop **************
		LODMeshCallbackInfo.resize(ObjToWorkWith->GetMaxLODCount());
		LODMeshTarget.resize(ObjToWorkWith->GetMaxLODCount());
		for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
		{
			LODMeshCallbackInfo[i].LODLevel = static_cast<int>(i);
			LODMeshCallbackInfo[i].Window = this;
			LODMeshTarget[i] = DRAG_AND_DROP_MANAGER.AddTarget(FE_MESH, DragAndDropLODMeshCallback, reinterpret_cast<void**>(&LODMeshCallbackInfo[i]), "Drop to assing LOD" + std::to_string(i) + " mesh");
		}

		MaterialCallbackInfo.bBillboardMaterial = false;
		MaterialCallbackInfo.Window = this;

		BillboardMaterialCallbackInfo.bBillboardMaterial = true;
		BillboardMaterialCallbackInfo.Window = this;

		MaterialTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_MATERIAL, DragAndDropMaterialCallback, reinterpret_cast<void**>(&MaterialCallbackInfo), "Drop to assing material");
		BillboardMaterialTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_MATERIAL, DragAndDropMaterialCallback, reinterpret_cast<void**>(&BillboardMaterialCallbackInfo), "Drop to assing billboard material");
		// ************** Drag&Drop END **************
	}
}

void EditGameModelPopup::SwitchMode(const int ToMode)
{
	switch (ToMode)
	{
		case NO_LOD_MODE:
		{
			Size.x = NO_LOD_WINDOW_WIDTH;
			Size.y = NO_LOD_WINDOW_HEIGHT;
			CurrentMode = NO_LOD_MODE;
			ChangeLODMeshButton[0]->SetCaption("Change Mesh");
			ChangeLODMeshButton[0]->SetPosition(ImVec2(Size.x / 2.0f - Size.x / 4.0f - ChangeLODMeshButton[0]->GetSize().x / 2, 35 + 340.0f));
			ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - ChangeMaterialButton->GetSize().x / 2, 35 + 340.0f));
			ApplyButton->SetPosition(ImVec2(Size.x / 4 - ApplyButton->GetSize().x / 2, Size.y - 40));
			CancelButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - CancelButton->GetSize().x / 2, Size.y - 40));

			for (size_t i = 1; i < TempModel->GetMaxLODCount(); i++)
			{
				UpdatedLODMeshs[i] = nullptr;
				TempModel->SetLODMesh(i, nullptr);
			}

			UpdatedBillboardMaterial = nullptr;
			TempModel->SetBillboardMaterial(nullptr);

			LODGroups->Clear();
			LODGroups->AddRange((ObjToWorkWith->GetLODMaxDrawDistance(0)) / ObjToWorkWith->GetCullDistance(), "LOD0", "", ImColor(0, 255, 0, 255));
			break;
		}

		case HAS_LOD_MODE:
		{
			ChangeLODMeshButton.resize(ObjToWorkWith->GetMaxLODCount());
			for (size_t i = 0; i < ObjToWorkWith->GetMaxLODCount(); i++)
			{
				const std::string ButtonName = "Change LOD" + std::to_string(i) + " Mesh";
				ChangeLODMeshButton[i] = new ImGuiButton(ButtonName);
				ChangeLODMeshButton[i]->SetSize(ImVec2(200, 35));
			}

			Size.x = 920.0f;
			Size.y = 880.0f;
			CurrentMode = HAS_LOD_MODE;
			ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2.0f - Size.x / 4.0f - ChangeMaterialButton->GetSize().x / 2, 35.0f + 340.0f + 200.0f));
			ChangeBillboardMaterialButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - ChangeBillboardMaterialButton->GetSize().x / 2, 35 + 340.0f + 200.0f));
			ApplyButton->SetPosition(ImVec2(Size.x / 4 - ApplyButton->GetSize().x / 2, Size.y - 40));
			CancelButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - CancelButton->GetSize().x / 2, Size.y - 40));
			break;
		}
		default:
			break;
	}
}

void EditGameModelPopup::DisplayLODGroups()
{
	const ImVec2 TextSize = ImGui::CalcTextSize("LOD Groups: ");
	ImGui::SetCursorPosX(Size.x / 2 - TextSize.x / 2);
	ImGui::SetCursorPosY(ChangeBillboardMaterialButton->GetPosition().y + ChangeBillboardMaterialButton->GetSize().y + 10.0f);
	ImGui::Text("LOD Groups: ");

	LODGroups->Render();

	float TotalRangeSpan = 0.0f;
	for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
	{
		FERangeRecord* Record = LODGroups->GetRangesRecord(i);
		if (Record == nullptr)
			break;

		const float RangeSpan = Record->GetRangeSpan();
		TotalRangeSpan += RangeSpan;

		if (TempModel->GetLODMesh(i) == nullptr)
			break;

		if (TempModel->IsLODBillboard(i))
		{
			Record->SetCaption(std::string("Billboard"));
		}
		else
		{
			Record->SetCaption(std::string("LOD") + std::to_string(i));
		}

		TempModel->SetLODMaxDrawDistance(i, TempModel->GetCullDistance() * TotalRangeSpan);
		std::string NewToolTip;
		if (i == 0)
		{
			NewToolTip = Record->GetCaption() + "(0 - " + std::to_string(static_cast<int>(TempModel->GetLODMaxDrawDistance(i))) + "m) " + std::to_string(RangeSpan * 100.0f) + "%";
		}
		else
		{
			int EndDistance = static_cast<int>(TempModel->GetLODMaxDrawDistance(i));
			if (i == TempModel->GetMaxLODCount() - 1 || TempModel->GetLODMesh(i + 1) == nullptr)
				EndDistance = static_cast<int>(TempModel->GetCullDistance());
			NewToolTip = Record->GetCaption() + "(" + std::to_string(static_cast<int>(TempModel->GetLODMaxDrawDistance(i - 1))) + " - " + std::to_string(EndDistance) + "m) " + std::to_string(RangeSpan * 100.0f) + "%";
		}

		Record->SetToolTipText(NewToolTip);
	}

	ImGui::SetCursorPosY(LODGroups->GetPosition().y + LODGroups->GetSize().y + 10.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
	ImGui::Text("cullDistance:");
	float CurrentCullRange = TempModel->GetCullDistance();
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
	ImGui::DragFloat("##cullDistance", &CurrentCullRange, 1.0f, 0.1f, 5000.0f);
	TempModel->SetCullDistance(CurrentCullRange);
}

void EditGameModelPopup::Render()
{
	if (!IsVisible())
		return;

	if (TempModel->IsUsingLOD())
	{
		SwitchMode(HAS_LOD_MODE);
	}
	else
	{
		SwitchMode(NO_LOD_MODE);
	}

	ImGui::SetNextWindowSize(Size);
	FEImGuiWindow::Render();

	// if we change something we will update preview.
	if (UpdatedMaterial != TempModel->GetMaterial())
	{
		TempModel->SetMaterial(UpdatedMaterial);
		PREVIEW_MANAGER.CreateGameModelPreview(TempModel, &TempPreview);
	}

	if (UpdatedBillboardMaterial != TempModel->GetBillboardMaterial())
	{
		TempModel->SetBillboardMaterial(UpdatedBillboardMaterial);
	}

	for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
	{
		if (UpdatedLODMeshs[i] != TempModel->GetLODMesh(i))
		{
			if (TempModel->GetLODMesh(i) == nullptr)
			{
				LODGroups->AddRange((TempModel->GetLODMaxDrawDistance(i)) / TempModel->GetCullDistance(), std::string("LOD") + std::to_string(i), "", LODColors[i]);
			}

			TempModel->SetLODMesh(i, UpdatedLODMeshs[i]);
			PREVIEW_MANAGER.CreateGameModelPreview(TempModel, &TempPreview);
		}
	}

	if (ObjToWorkWith == nullptr)
	{
		FEImGuiWindow::Close();
		return;
	}

	const float BaseY = 35.0f;
	const float CurrentY = BaseY;

	bool bLODActive = TempModel->IsUsingLOD();
	ImGui::SetCursorPosY(CurrentY);
	ImGui::Checkbox("have LOD levels", &bLODActive);
	TempModel->SetUsingLOD(bLODActive);
	/*if (tempModel->IsUsingLOD())
	{
		switchMode(HAS_LOD_MODE);
	}
	else
	{
		switchMode(NO_LOD_MODE);
	}*/

	ImVec2 TextSize = ImGui::CalcTextSize("Preview of game model:");
	ImGui::SetCursorPosX(Size.x / 2 - TextSize.x / 2);
	ImGui::SetCursorPosY(CurrentY + 30);
	ImGui::Text("Preview of game model:");
	ImGui::SetCursorPosX(Size.x / 2 - 128 / 2);
	ImGui::SetCursorPosY(CurrentY + 50);
	ImGui::Image((void*)static_cast<intptr_t>(TempPreview->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	if (CurrentMode == NO_LOD_MODE)
	{
		ImGui::Separator();
		TextSize = ImGui::CalcTextSize("Mesh component:");
		ImGui::SetCursorPosX(Size.x / 4 - TextSize.x / 2);
		ImGui::Text("Mesh component:");
		ImGui::SetCursorPosX(Size.x / 4 - 128 / 2);
		ImGui::SetCursorPosY(CurrentY + 210.0f);
		ImGui::Image((void*)static_cast<intptr_t>(PREVIEW_MANAGER.GetMeshPreview(TempModel->Mesh->GetObjectID())->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		LODMeshTarget[0]->StickToItem();

		ChangeLODMeshButton[0]->Render();
		if (ChangeLODMeshButton[0]->IsClicked())
		{
			UpdatedLODMeshs[0] = TempModel->GetLODMesh(0);

			MeshToModify = &UpdatedLODMeshs[0];
			SelectFeObjectPopUp::getInstance().Show(FE_MESH, ChangeMeshCallBack, UpdatedLODMeshs[0]);
		}

		TextSize = ImGui::CalcTextSize("Material component:");
		ImGui::SetCursorPosX(Size.x / 2 + Size.x / 4 - TextSize.x / 2);
		ImGui::SetCursorPosY(CurrentY + 187.0f);
		ImGui::Text("Material component:");
		ImGui::SetCursorPosX(Size.x / 2 + Size.x / 4 - 128 / 2);
		ImGui::SetCursorPosY(CurrentY + 210.0f);
		ImGui::Image((void*)static_cast<intptr_t>(PREVIEW_MANAGER.GetMaterialPreview(TempModel->Material->GetObjectID())->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		MaterialTarget->StickToItem();
		ChangeMaterialButton->Render();
		if (ChangeMaterialButton->IsClicked())
		{
			UpdatedMaterial = TempModel->GetMaterial();

			MaterialToModify = &UpdatedMaterial;
			SelectFeObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeMaterialCallBack, UpdatedMaterial);
		}
	}
	else if (CurrentMode == HAS_LOD_MODE)
	{
		ImGui::Separator();

		const float BaseXPosition = Size.x / 2.0f - Size.x / 4.0f;
		for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
		{
			const float CurrentXPosition = BaseXPosition + (Size.x / 4.0f) * i - Size.x / 8.0f;
			if (TempModel->GetLODMesh(i) == nullptr)
			{
				if (TempModel->IsLODBillboard(i - 1))
					break;

				ChangeLODMeshButton[i]->SetCaption(std::string("Add LOD") + std::to_string(i));
				ChangeLODMeshButton[i]->SetPosition(ImVec2(CurrentXPosition - ChangeLODMeshButton[i]->GetSize().x / 2, CurrentY + 210.0f + 128.0f / 2.0f - 10.0f - ChangeLODMeshButton[i]->GetSize().y/*- changeLODMeshButton[i]->getSize().y / 2.0f*/));

				ChangeLODMeshButton[i]->Render();
				if (ChangeLODMeshButton[i]->IsClicked())
				{
					UpdatedLODMeshs[i] = TempModel->GetLODMesh(i);

					MeshToModify = &UpdatedLODMeshs[i];
					SelectFeObjectPopUp::getInstance().Show(FE_MESH, ChangeMeshCallBack, UpdatedLODMeshs[i]);
				}

				AddBillboard->SetPosition(ImVec2(CurrentXPosition - AddBillboard->GetSize().x / 2, CurrentY + 210.0f + 128.0f / 2.0f + 10.0f / 2.0f));
				AddBillboard->Render();
				if (AddBillboard->IsClicked())
				{
					UpdatedLODMeshs[i] = RESOURCE_MANAGER.GetMesh("1Y251E6E6T78013635793156"/*"plane"*/);
					UpdatedBillboardMaterial = RESOURCE_MANAGER.GetMaterial("61649B9E0F08013Q3939316C"/*"FEPBRBaseMaterial"*/);
					TempModel->SetIsLODBillboard(i, true);
				}

				break;
			}
			else
			{
				std::string Caption = (std::string("LOD") + std::to_string(i)) + ":";
				if (TempModel->IsLODBillboard(i))
					Caption = "Billboard:";

				TextSize = ImGui::CalcTextSize(Caption.c_str());
				ImGui::SetCursorPosX(CurrentXPosition - TextSize.x / 2.0f);
				ImGui::SetCursorPosY(CurrentY + 187.0f);
				ImGui::Text(Caption.c_str());
				ImGui::SetCursorPosX(CurrentXPosition - 128 / 2);
				ImGui::SetCursorPosY(CurrentY + 210.0f);

				ImGui::Image((void*)static_cast<intptr_t>(TempModel->GetLODMesh(i) == nullptr
					                                          ? RESOURCE_MANAGER.NoTexture->GetTextureID()
					                                          : PREVIEW_MANAGER.GetMeshPreview(TempModel->GetLODMesh(i)->GetObjectID())->GetTextureID()),
														  ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				LODMeshTarget[i]->StickToItem();

				ChangeLODMeshButton[i]->SetCaption(std::string("Change LOD") + std::to_string(i) + " Mesh");
				if (TempModel->IsLODBillboard(i))
					ChangeLODMeshButton[i]->SetCaption(std::string("Change Billboard"));
				ChangeLODMeshButton[i]->SetPosition(ImVec2(CurrentXPosition - ChangeLODMeshButton[i]->GetSize().x / 2, 35 + 340.0f));

				if (IsLastSetupLOD(i) && i > 0)
				{
					DeleteLODMeshButton->SetPosition(ImVec2(CurrentXPosition + 80.0f, 215.0f));
					DeleteLODMeshButton->Render();
					if (DeleteLODMeshButton->IsClicked())
					{
						UpdatedLODMeshs[i] = nullptr;
						TempModel->SetLODMesh(i, nullptr);
						TempModel->SetIsLODBillboard(i, false);
						LODGroups->DeleteRange(i);
					}
				}
			}

			ChangeLODMeshButton[i]->Render();
			if (ChangeLODMeshButton[i]->IsClicked())
			{
				UpdatedLODMeshs[i] = TempModel->GetLODMesh(i);

				MeshToModify = &UpdatedLODMeshs[i];
				SelectFeObjectPopUp::getInstance().Show(FE_MESH, ChangeMeshCallBack, UpdatedLODMeshs[i]);
			}
		}

		for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
		{
			if (IsLastSetupLOD(i))
			{
				if (TempModel->IsLODBillboard(i))
				{
					TextSize = ImGui::CalcTextSize("Material component:");
					ImGui::SetCursorPosX(Size.x / 2 - Size.x / 4 - TextSize.x / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 187.0f);
					ImGui::Text("Material component:");
					ImGui::SetCursorPosX(Size.x / 2 - Size.x / 4 - 128 / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 210.0f);
					ImGui::Image((void*)static_cast<intptr_t>(PREVIEW_MANAGER.GetMaterialPreview(TempModel->GetMaterial()->GetObjectID())->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					MaterialTarget->StickToItem();

					ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2 - Size.x / 4 - ChangeMaterialButton->GetSize().x / 2, BaseY + 340.0f + 200.0f));
					ChangeMaterialButton->Render();
					if (ChangeMaterialButton->IsClicked())
					{
						MaterialToModify = &UpdatedMaterial;
						SelectFeObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeMaterialCallBack, UpdatedMaterial);
					}

					TextSize = ImGui::CalcTextSize("Billboard Material component:");
					ImGui::SetCursorPosX(Size.x / 2 + Size.x / 4 - TextSize.x / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 187.0f);
					ImGui::Text("Billboard Material component:");
					ImGui::SetCursorPosX(Size.x / 2 + Size.x / 4 - 128 / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 210.0f);
					ImGui::Image((void*)static_cast<intptr_t>(TempModel->GetBillboardMaterial() == nullptr
						                                          ? RESOURCE_MANAGER.NoTexture->GetTextureID()
						                                          : PREVIEW_MANAGER.GetMaterialPreview(TempModel->GetBillboardMaterial()->GetObjectID())->
						                                          GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					BillboardMaterialTarget->StickToItem();

					ChangeBillboardMaterialButton->Render();
					if (ChangeBillboardMaterialButton->IsClicked())
					{
						UpdatedBillboardMaterial = TempModel->GetBillboardMaterial();

						const std::vector<std::string> TempMaterialList = RESOURCE_MANAGER.GetMaterialList();
						std::vector<FEObject*> FinalMaterialList;
						for (size_t j = 0; j < TempMaterialList.size(); j++)
						{
							if (RESOURCE_MANAGER.GetMaterial(TempMaterialList[j])->Shader->GetObjectID() == "0800253C242B05321A332D09"/*"FEPBRShader"*/)
							{
								FinalMaterialList.push_back(RESOURCE_MANAGER.GetMaterial(TempMaterialList[j]));
							}
						}

						BillboardMaterialToModify = &UpdatedBillboardMaterial;
						SelectFeObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeBillboardMaterialCallBack, UpdatedBillboardMaterial, FinalMaterialList);
					}
				}
				else
				{
					TextSize = ImGui::CalcTextSize("Material component:");
					ImGui::SetCursorPosX(Size.x / 2 - TextSize.x / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 187.0f);
					ImGui::Text("Material component:");
					ImGui::SetCursorPosX(Size.x / 2 - 128 / 2);
					ImGui::SetCursorPosY(CurrentY + 200 + 210.0f);
					ImGui::Image((void*)static_cast<intptr_t>(PREVIEW_MANAGER.GetMaterialPreview(TempModel->GetMaterial()->GetObjectID())->GetTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

					ChangeMaterialButton->SetPosition(ImVec2(Size.x / 2 - ChangeMaterialButton->GetSize().x / 2, BaseY + 340.0f + 200.0f));
					ChangeMaterialButton->Render();
					if (ChangeMaterialButton->IsClicked())
					{
						MaterialToModify = &UpdatedMaterial;
						SelectFeObjectPopUp::getInstance().Show(FE_MATERIAL, ChangeMaterialCallBack, UpdatedMaterial);
					}
				}

				break;
			}
		}

		// ************** LOD Groups **************
		ImGui::Separator();
		DisplayLODGroups();

		bool bBillboard = false;
		for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
		{
			if (TempModel->IsLODBillboard(i))
				bBillboard = true;
		}

		if (bBillboard)
		{
			Size.y = 830.0f + 40.0f;
			ApplyButton->SetPosition(ImVec2(Size.x / 4 - ApplyButton->GetSize().x / 2, Size.y - 40));
			CancelButton->SetPosition(ImVec2(Size.x / 2 + Size.x / 4 - CancelButton->GetSize().x / 2, Size.y - 40));

			/*ImGui::Text("Billboard Scale:");
			float billboardScale = tempModel->getBillboardScale();
			ImGui::SameLine();
			ImGui::DragFloat("##Billboard Scale", &billboardScale, 0.1f);
			tempModel->setBillboardScale(billboardScale);*/

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			ImGui::Text("Billboard Zero Rotation:");
			float ZeroRotation = TempModel->GetBillboardZeroRotaion();
			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
			ImGui::DragFloat("##Billboard Zero Rotation", &ZeroRotation, 0.1f, 0.0f, 360.0f);
			TempModel->SetBillboardZeroRotaion(ZeroRotation);
		}
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
	ImGui::Text("Scale Factor:");
	float ScaleFactor = TempModel->GetScaleFactor();
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
	ImGui::SetNextItemWidth(200);
	ImGui::DragFloat("##Scale Factor", &ScaleFactor, 0.1f);
	TempModel->SetScaleFactor(ScaleFactor);

	ImGui::SetCursorPosY(Size.y - 50.0f);
	ImGui::Separator();
	ImGui::SetItemDefaultFocus();
	ApplyButton->Render();
	if (ApplyButton->IsClicked())
	{
		ObjToWorkWith->SetDirtyFlag(true);
		ObjToWorkWith->Mesh = TempModel->Mesh;
		ObjToWorkWith->SetUsingLOD(TempModel->IsUsingLOD());

		for (size_t i = 0; i < TempModel->GetMaxLODCount(); i++)
		{
			ObjToWorkWith->SetLODMesh(i, TempModel->GetLODMesh(i));
			ObjToWorkWith->SetLODMaxDrawDistance(i, TempModel->GetLODMaxDrawDistance(i));
			ObjToWorkWith->SetIsLODBillboard(i, TempModel->IsLODBillboard(i));
		}

		ObjToWorkWith->SetCullDistance(TempModel->GetCullDistance());
		ObjToWorkWith->SetMaterial(TempModel->GetMaterial());
		ObjToWorkWith->SetBillboardMaterial(TempModel->GetBillboardMaterial());
		ObjToWorkWith->SetScaleFactor(TempModel->GetScaleFactor());
		ObjToWorkWith->SetBillboardZeroRotaion(TempModel->GetBillboardZeroRotaion());
		PREVIEW_MANAGER.CreateGameModelPreview(ObjToWorkWith->GetObjectID());

		FEImGuiWindow::Close();
		return;
	}

	ImGui::SameLine();
	CancelButton->Render();
	if (CancelButton->IsClicked())
	{
		FEImGuiWindow::Close();
		return;
	}

	FEImGuiWindow::OnRenderEnd();
}

void EditGameModelPopup::Close()
{
	FEImGuiWindow::Close();
}

// ************** Drag&Drop **************
bool EditGameModelPopup::DragAndDropLODMeshCallback(FEObject* Object, void** CallbackInfo)
{
	const MeshTargetCallbackInfo* Info = reinterpret_cast<MeshTargetCallbackInfo*>(CallbackInfo);
	Info->Window->UpdatedLODMeshs[Info->LODLevel] = RESOURCE_MANAGER.GetMesh(Object->GetObjectID());
	Info->Window->TempModel->SetLODMesh(Info->LODLevel, RESOURCE_MANAGER.GetMesh(Object->GetObjectID()));
	PREVIEW_MANAGER.CreateGameModelPreview(Info->Window->TempModel, &Info->Window->TempPreview);
	return true;
}

bool EditGameModelPopup::DragAndDropMaterialCallback(FEObject* Object, void** CallbackInfo)
{
	const MaterialTargetCallbackInfo* Info = reinterpret_cast<MaterialTargetCallbackInfo*>(CallbackInfo);

	if (Info->bBillboardMaterial)
	{
		Info->Window->UpdatedBillboardMaterial = RESOURCE_MANAGER.GetMaterial(Object->GetObjectID());
		Info->Window->TempModel->SetBillboardMaterial(RESOURCE_MANAGER.GetMaterial(Object->GetObjectID()));
	}
	else
	{
		Info->Window->UpdatedMaterial = RESOURCE_MANAGER.GetMaterial(Object->GetObjectID());
		Info->Window->TempModel->SetMaterial(RESOURCE_MANAGER.GetMaterial(Object->GetObjectID()));
		PREVIEW_MANAGER.CreateGameModelPreview(Info->Window->TempModel, &Info->Window->TempPreview);
	}

	return true;
}
// ************** Drag&Drop END **************

bool EditGameModelPopup::IsLastSetupLOD(const size_t LODIndex)
{
	if (LODIndex >= TempModel->GetMaxLODCount())
		return false;

	if (LODIndex == TempModel->GetMaxLODCount() - 1)
		return true;

	if (TempModel->IsLODBillboard(LODIndex))
		return true;

	for (size_t i = LODIndex + 1; i < TempModel->GetMaxLODCount(); i++)
	{
		if (TempModel->GetLODMesh(i) != nullptr)
			return false;
	}

	return true;
}

EditMaterialPopup* EditMaterialPopup::Instance = nullptr;

EditMaterialPopup::EditMaterialPopup()
{
	ObjToWorkWith = nullptr;
	Flags = ImGuiWindowFlags_NoResize;

	CancelButton = new ImGuiButton("Cancel");
	CancelButton->SetDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
	CancelButton->SetHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
	CancelButton->SetActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));
#ifdef USE_NODES
	NodeAreaTarget = DRAG_AND_DROP_MANAGER.AddTarget(FE_TEXTURE, DragAndDropnodeAreaTargetCallback, reinterpret_cast<void**>(&DragAndDropCallbackInfo), "Drop to add texture");
#endif // USE_NODES
}

EditMaterialPopup::~EditMaterialPopup()
{
	delete CancelButton;
	delete IconButton;
}

#ifdef USE_NODES
FEMaterial* EditMaterialPopup::ObjToWorkWith = nullptr;
FEVisualNodeArea* EditMaterialPopup::MaterialNodeArea = nullptr;
ImVec2 EditMaterialPopup::NodeGridRelativePosition = ImVec2(5, 30);
ImVec2 EditMaterialPopup::WindowPosition = ImVec2(0, 0);
ImVec2 EditMaterialPopup::MousePositionWhenContextMenuWasOpened = ImVec2(0, 0);
FETexture* EditMaterialPopup::TextureForNewNode = nullptr;
#endif // USE_NODES
void EditMaterialPopup::Show(FEMaterial* Material)
{
	if (Material != nullptr)
	{
		TempContainer = RESOURCE_MANAGER.NoTexture;
		ObjToWorkWith = Material;

		std::string TempCaption = "Edit material:";
		TempCaption += " " + ObjToWorkWith->GetName();
		strcpy_s(Caption, TempCaption.size() + 1, TempCaption.c_str());
#ifdef USE_NODES
		Size = ImVec2(1500.0f, 1000.0f);
#else
		size = ImVec2(1500.0f, 700.0f);
#endif // USE_NODES
		Position = ImVec2(FEngine::getInstance().GetWindowWidth() / 2 - Size.x / 2, FEngine::getInstance().GetWindowHeight() / 2 - Size.y / 2);
		FEImGuiWindow::Show();

		IconButton = new ImGuiImageButton(nullptr);
		IconButton->SetSize(ImVec2(128, 128));
		IconButton->SetUV0(ImVec2(0.0f, 0.0f));
		IconButton->SetUV1(ImVec2(1.0f, 1.0f));
		IconButton->SetFramePadding(8);

#ifdef USE_NODES
		MaterialNodeArea = NODE_SYSTEM.CreateNodeArea();
		MaterialNodeArea->SetMainContextMenuFunc(NodeSystemMainContextMenu);
		MaterialNodeArea->SetNodeEventCallback(TextureNodeCallback);

		FEEditorMaterialNode* NewNode = new FEEditorMaterialNode(Material);

		ImVec2 PositionOnCanvas;
		PositionOnCanvas.x = NodeGridRelativePosition.x + Size.x - Size.x / 6 - NewNode->GetSize().x / 2.0f;
		PositionOnCanvas.y = NodeGridRelativePosition.y + Size.y / 2 - NewNode->GetSize().y / 2.0f;
		NewNode->SetPosition(PositionOnCanvas);

		MaterialNodeArea->AddNode(NewNode);

		// Add all textures of material as a texture nodes
		// Place them in shifted grid.
		int VisualIndex = 0;
		for (size_t i = 0; i < Material->Textures.size(); i++)
		{
			if (Material->Textures[i] == nullptr)
				continue;

			FEEditorTextureSourceNode* NewTextureNode = new FEEditorTextureSourceNode(Material->Textures[i]);

			PositionOnCanvas.x = NodeGridRelativePosition.x + Size.x / 2 - NewTextureNode->GetSize().x / 2.0f - (VisualIndex % 2 == 0 ? (NewTextureNode->GetSize().x + 24.0f) : 0.0f);
			PositionOnCanvas.y = NodeGridRelativePosition.y + VisualIndex / 2.0f * (NewTextureNode->GetSize().y + 24.0f);
			NewTextureNode->SetPosition(PositionOnCanvas);

			MaterialNodeArea->AddNode(NewTextureNode);
			VisualIndex++;

			// We should recreate proper connections.
			if (Material->GetAlbedoMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, 5, NewNode, 0);
			}
			else if (Material->GetAlbedoMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, 5, NewNode, 6);
			}

			if (Material->GetNormalMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, 5, NewNode, 1);
			}
			else if (Material->GetNormalMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, 5, NewNode, 7);
			}

			if (Material->GetAOMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetAOMapChannel(), NewNode, 2);
			}
			else if (Material->GetAOMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetAOMapChannel(), NewNode, 8);
			}

			if (Material->GetRoughtnessMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetRoughtnessMapChannel(), NewNode, 3);
			}
			else if (Material->GetRoughtnessMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetRoughtnessMapChannel(), NewNode, 9);
			}

			if (Material->GetMetalnessMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetMetalnessMapChannel(), NewNode, 4);
			}
			else if (Material->GetMetalnessMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetMetalnessMapChannel(), NewNode, 10);
			}

			if (Material->GetDisplacementMap() == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetDisplacementMapChannel(), NewNode, 5);
			}
			else if (Material->GetDisplacementMap(1) == Material->Textures[i])
			{
				MaterialNodeArea->TryToConnect(NewTextureNode, ObjToWorkWith->GetDisplacementMapChannel(), NewNode, 11);
			}
		}
#else
		// ************** Drag&Drop **************
		TexturesListTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_TEXTURE, DragAndDropTexturesListCallback, reinterpret_cast<void**>(&ObjToWorkWith), "Drop to add texture");
		if (MaterialBindingtargets.size() == 0)
		{
			MaterialBindingInfo.resize(12);
			for (size_t i = 0; i < 12; i++)
			{
				MaterialBindingInfo[i].Material = reinterpret_cast<void**>(&ObjToWorkWith);
				MaterialBindingInfo[i].TextureBinding = i;

				MaterialBindingCallbackInfo* test = &MaterialBindingInfo[i];
				MaterialBindingtargets.push_back(DRAG_AND_DROP_MANAGER.addTarget(FE_TEXTURE, DragAndDropMaterialBindingsCallback, reinterpret_cast<void**>(&MaterialBindingInfo[i]), "Drop to assign texture"));
			}
		}
		// ************** Drag&Drop END **************
#endif // USE_NODES
	}
}

void EditMaterialPopup::Render()
{
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	if (ObjToWorkWith == nullptr)
	{
		FEImGuiWindow::Close();
		return;
	}

#ifdef USE_NODES
	MaterialNodeArea->SetAreaPosition(ImVec2(0, 0));
	MaterialNodeArea->SetAreaSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 50));
	MaterialNodeArea->Update();
	NodeAreaTarget->StickToItem();

	WindowPosition = ImGui::GetWindowPos();

	if (ImGui::GetIO().MouseReleased[1])
		MousePositionWhenContextMenuWasOpened = ImGui::GetMousePos();
#else
	// lame callback
	if (tempContainer != RESOURCE_MANAGER.noTexture)
	{
		if (textureDestination == -1)
		{
			ObjToWorkWith->addTexture(tempContainer);
		}
		else
		{
			int subMaterial = textureDestination > 5;
			if (subMaterial)
				textureDestination -= 6;

			switch (textureDestination)
			{
			case 0:
			{
				ObjToWorkWith->setAlbedoMap(tempContainer, subMaterial);
				break;
			}
			case 1:
			{
				ObjToWorkWith->setNormalMap(tempContainer, subMaterial);
				break;
			}
			case 2:
			{
				ObjToWorkWith->setAOMap(tempContainer, 0, subMaterial);
				break;
			}
			case 3:
			{
				ObjToWorkWith->setRoughtnessMap(tempContainer, 0, subMaterial);
				break;
			}
			case 4:
			{
				ObjToWorkWith->setMetalnessMap(tempContainer, 0, subMaterial);
				break;
			}
			case 5:
			{
				ObjToWorkWith->setDisplacementMap(tempContainer, 0, subMaterial);
				break;
			}
			}
		}

		tempContainer = RESOURCE_MANAGER.noTexture;
		PREVIEW_MANAGER.createMaterialPreview(ObjToWorkWith->getObjectID());
		PROJECT_MANAGER.getCurrent()->bModified = true;
	}

	ImGui::Text("Textures (%d out of 16):", textureCount);
	ImGui::SameLine();
	ImGui::SetCursorPosX(size.x * 0.26f);
	ImGui::Text("Bindings:");

	ImGui::SameLine();
	ImGui::SetCursorPosX(size.x * 0.26f + size.x * 0.49f);
	ImGui::Text("Settings:");

	// Textures
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 162, 232, 50));
		ImGui::BeginChild("Textures", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.25f, 600), true, ImGuiWindowFlags_HorizontalScrollbar);

		if (!ImGui::IsPopupOpen("##materialPropertiesContext_menu"))
			textureFromListUnderMouse = -1;

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1))
			ImGui::OpenPopup("##materialPropertiesContext_menu");

		TexturesListTarget->stickToCurrentWindow();

		if (ImGui::BeginPopup("##materialPropertiesContext_menu"))
		{
			if (textureFromListUnderMouse == -1)
			{
				if (ImGui::MenuItem("Add texture..."))
				{
					textureDestination = -1;
					selectTexturePopUp::getInstance().show(&tempContainer);
				}
			}
			else
			{
				if (ImGui::MenuItem("Remove"))
				{
					ObjToWorkWith->removeTexture(textureFromListUnderMouse);
					PROJECT_MANAGER.getCurrent()->bModified = true;
				}
			}

			ImGui::EndPopup();
		}

		ImGui::Columns(2, "TextureColumns", false);
		textureCount = 0;
		for (size_t i = 0; i < ObjToWorkWith->textures.size(); i++)
		{
			if (ObjToWorkWith->textures[i] == nullptr)
				continue;

			textureCount++;
			ImGui::PushID(ObjToWorkWith->textures[i]->getName().c_str());

			ObjToWorkWith->textures[i]->getName() == "noTexture" ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(RESOURCE_MANAGER.getTexture(ObjToWorkWith->textures[i]->getObjectID()));
			iconButton->render();

			if (iconButton->isHovered())
			{
				textureFromListUnderMouse = i;
			}

			if (iconButton->isHovered() && ImGui::IsMouseDragging(0) && !DRAG_AND_DROP_MANAGER.objectIsDraged())
				DRAG_AND_DROP_MANAGER.setObject(ObjToWorkWith->textures[i], ObjToWorkWith->textures[i], ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			ImGui::Text(ObjToWorkWith->textures[i]->getName().c_str());
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1);

		ImGui::PopStyleColor();
		ImGui::EndChild();
	}

	// material options
	{
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 162, 232, 50));
		ImGui::BeginChild("Bindings", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.49f, 600), true, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::Text("First sub material:");

		// ************* Albedo *************
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosY(38);
		ImGui::Text("Albedo:");
		ObjToWorkWith->getAlbedoMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getAlbedoMap());
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 0;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[0]->stickToItem();

		if (ObjToWorkWith->getAlbedoMap() != nullptr)
		{
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##albedo", "rgba", ImGuiWindowFlags_None))
			{
				bool is_selected = true;
				ImGui::Selectable("rgba", is_selected);
				ImGui::SetItemDefaultFocus();
				ImGui::EndCombo();
			}
		}

		// ************* Normal *************
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		ImGui::SetCursorPosY(38);
		ImGui::Text("Normal:");
		ObjToWorkWith->getNormalMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getNormalMap());
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 1;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[1]->stickToItem();

		if (ObjToWorkWith->getNormalMap() != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##normal", "rgb", ImGuiWindowFlags_None))
			{
				bool is_selected = true;
				ImGui::Selectable("rgb", is_selected);
				ImGui::SetItemDefaultFocus();
				ImGui::EndCombo();
			}
		}

		// ************* AO *************
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		ImGui::SetCursorPosY(38);
		ImGui::Text("AO:");
		ObjToWorkWith->getAOMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getAOMap());
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 2;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[2]->stickToItem();

		if (ObjToWorkWith->getAOMap() != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##ao", channels[ObjToWorkWith->getAOMapChannel()].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && ObjToWorkWith->getAOMap()->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[ObjToWorkWith->getAOMapChannel()] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						ObjToWorkWith->setAOMap(ObjToWorkWith->getAOMap(), i);

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// ************* Roughtness *************
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosY(38 + 128 + 80);
		ImGui::Text("Roughtness:");
		ObjToWorkWith->getRoughtnessMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getRoughtnessMap());
		ImGui::SetCursorPosX(10);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 3;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[3]->stickToItem();

		if (ObjToWorkWith->getRoughtnessMap() != nullptr)
		{
			ImGui::SetCursorPosX(10);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##roughtness", channels[ObjToWorkWith->getRoughtnessMapChannel()].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && ObjToWorkWith->getRoughtnessMap()->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[ObjToWorkWith->getRoughtnessMapChannel()] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						ObjToWorkWith->setRoughtnessMap(ObjToWorkWith->getRoughtnessMap(), i);

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// ************* Metalness *************
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		ImGui::SetCursorPosY(38 + 128 + 80);
		ImGui::Text("Metalness:");
		ObjToWorkWith->getMetalnessMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getMetalnessMap());
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 4;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[4]->stickToItem();

		if (ObjToWorkWith->getMetalnessMap() != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##metalness", channels[ObjToWorkWith->getMetalnessMapChannel()].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && ObjToWorkWith->getMetalnessMap()->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[ObjToWorkWith->getMetalnessMapChannel()] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						ObjToWorkWith->setMetalnessMap(ObjToWorkWith->getMetalnessMap(), i);

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// ************* Displacement *************
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		ImGui::SetCursorPosY(38 + 128 + 80);
		ImGui::Text("Displacement:");
		ObjToWorkWith->getDisplacementMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getDisplacementMap());
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 5;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[5]->stickToItem();

		if (ObjToWorkWith->getDisplacementMap() != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##displacement", channels[ObjToWorkWith->getDisplacementMapChannel()].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && ObjToWorkWith->getDisplacementMap()->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[ObjToWorkWith->getDisplacementMapChannel()] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						ObjToWorkWith->setDisplacementMap(ObjToWorkWith->getDisplacementMap(), i);

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		ImGui::SetCursorPosY(38 + 128 + 80 + 128 + 80);
		ImGui::Separator();

		ImGui::Text("Second sub material:");
		// ************* Albedo *************
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosY(64 + 128 + 80 + 128 + 80);
		ImGui::Text("Albedo:");
		ObjToWorkWith->getAlbedoMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getAlbedoMap(1));
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 6;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[6]->stickToItem();

		if (ObjToWorkWith->getAlbedoMap(1) != nullptr)
		{
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##albedoSubmaterial", "rgba", ImGuiWindowFlags_None))
			{
				bool is_selected = true;
				ImGui::Selectable("rgba", is_selected);
				ImGui::SetItemDefaultFocus();
				ImGui::EndCombo();
			}
		}

		// ************* Normal *************
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		ImGui::SetCursorPosY(64 + 128 + 80 + 128 + 80);
		ImGui::Text("Normal:");
		ObjToWorkWith->getNormalMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getNormalMap(1));
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 7;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[7]->stickToItem();

		if (ObjToWorkWith->getNormalMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##normalSubmaterial", "rgb", ImGuiWindowFlags_None))
			{
				bool is_selected = true;
				ImGui::Selectable("rgb", is_selected);
				ImGui::SetItemDefaultFocus();
				ImGui::EndCombo();
			}
		}

		// ************* AO *************
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		ImGui::SetCursorPosY(64 + 128 + 80 + 128 + 80);
		ImGui::Text("AO:");
		ObjToWorkWith->getAOMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getAOMap(1));
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 8;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[8]->stickToItem();

		if (ObjToWorkWith->getAOMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##aoSubmaterial", channels[ObjToWorkWith->getAOMapChannel(1)].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && ObjToWorkWith->getAOMap(1)->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[ObjToWorkWith->getAOMapChannel(1)] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						ObjToWorkWith->setAOMap(ObjToWorkWith->getAOMap(1), i, 1);

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// ************* Roughtness *************
		ImGui::SetCursorPosX(10);
		ImGui::SetCursorPosY(64 + 128 + 80 + 128 + 80 + 128 + 80);
		ImGui::Text("Roughtness:");
		ObjToWorkWith->getRoughtnessMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getRoughtnessMap(1));
		ImGui::SetCursorPosX(10);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 9;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[9]->stickToItem();

		if (ObjToWorkWith->getRoughtnessMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##roughtnessSubmaterial", channels[ObjToWorkWith->getRoughtnessMapChannel(1)].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && ObjToWorkWith->getRoughtnessMap(1)->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[ObjToWorkWith->getRoughtnessMapChannel(1)] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						ObjToWorkWith->setRoughtnessMap(ObjToWorkWith->getRoughtnessMap(1), i, 1);

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// ************* Metalness *************
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		ImGui::SetCursorPosY(64 + 128 + 80 + 128 + 80 + 128 + 80);
		ImGui::Text("Metalness:");
		ObjToWorkWith->getMetalnessMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getMetalnessMap(1));
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 10;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[10]->stickToItem();

		if (ObjToWorkWith->getMetalnessMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##metalnessSubmaterial", channels[ObjToWorkWith->getMetalnessMapChannel(1)].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && ObjToWorkWith->getMetalnessMap(1)->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[ObjToWorkWith->getMetalnessMapChannel(1)] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						ObjToWorkWith->setMetalnessMap(ObjToWorkWith->getMetalnessMap(1), i, 1);

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// ************* Displacement *************
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		ImGui::SetCursorPosY(64 + 128 + 80 + 128 + 80 + 128 + 80);
		ImGui::Text("Displacement:");
		ObjToWorkWith->getDisplacementMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(ObjToWorkWith->getDisplacementMap(1));
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 11;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, ObjToWorkWith->textures);
		}

		if (MaterialBindingtargets.size() > 0)
			MaterialBindingtargets[11]->stickToItem();

		if (ObjToWorkWith->getDisplacementMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##displacementSubmaterial", channels[ObjToWorkWith->getDisplacementMapChannel(1)].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && ObjToWorkWith->getDisplacementMap(1)->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[ObjToWorkWith->getDisplacementMapChannel(1)] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						ObjToWorkWith->setDisplacementMap(ObjToWorkWith->getDisplacementMap(1), i, 1);

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		ImGui::PopStyleColor();
		ImGui::EndChild();
	}

	// Settings
	{
		static float fieldWidth = 350.0f;
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 162, 232, 50));
		ImGui::BeginChild("Settings", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.30f, 600), true, ImGuiWindowFlags_HorizontalScrollbar);

		FEShaderParam* debugFlag = ObjToWorkWith->getParameter("debugFlag");
		if (debugFlag != nullptr)
		{
			ImGui::Text("Debug flag:");
			ImGui::SetNextItemWidth(fieldWidth);
			int iData = *(int*)debugFlag->data;
			ImGui::SliderInt("##Debug flag", &iData, 0, 10);
			debugFlag->updateData(iData);
		}

		// ************* Normal *************
		ImGui::Text("Normal map intensity:");
		ImGui::SetNextItemWidth(fieldWidth);
		float normalMapIntensity = ObjToWorkWith->getNormalMapIntensity();
		ImGui::DragFloat("##Normal map intensity", &normalMapIntensity, 0.01f, 0.0f, 1.0f);
		ObjToWorkWith->setNormalMapIntensity(normalMapIntensity);

		// ************* AO *************
		if (ObjToWorkWith->getAOMap() == nullptr)
		{
			ImGui::Text("Ambient occlusion intensity:");
			ImGui::SetNextItemWidth(fieldWidth);
			float ambientOcclusionIntensity = ObjToWorkWith->getAmbientOcclusionIntensity();
			ImGui::DragFloat("##Ambient occlusion intensity", &ambientOcclusionIntensity, 0.01f, 0.0f, 10.0f);
			ObjToWorkWith->setAmbientOcclusionIntensity(ambientOcclusionIntensity);
		}
		else
		{
			ImGui::Text("Ambient occlusion map intensity:");
			ImGui::SetNextItemWidth(fieldWidth);
			float AOMapIntensity = ObjToWorkWith->getAmbientOcclusionMapIntensity();
			ImGui::DragFloat("##Ambient occlusion map intensity", &AOMapIntensity, 0.01f, 0.0f, 10.0f);
			ObjToWorkWith->setAmbientOcclusionMapIntensity(AOMapIntensity);
		}

		// ************* Roughtness *************
		if (ObjToWorkWith->getRoughtnessMap() == nullptr)
		{
			ImGui::Text("Roughtness:");
			ImGui::SetNextItemWidth(fieldWidth);
			float roughtness = ObjToWorkWith->getRoughtness();
			ImGui::DragFloat("##Roughtness", &roughtness, 0.01f, 0.0f, 1.0f);
			ObjToWorkWith->setRoughtness(roughtness);
		}
		else
		{
			ImGui::Text("Roughtness map intensity:");
			ImGui::SetNextItemWidth(fieldWidth);
			float roughtness = ObjToWorkWith->getRoughtnessMapIntensity();
			ImGui::DragFloat("##Roughtness map intensity", &roughtness, 0.01f, 0.0f, 10.0f);
			ObjToWorkWith->setRoughtnessMapIntensity(roughtness);
		}

		// ************* Metalness *************
		if (ObjToWorkWith->getMetalnessMap() == nullptr)
		{
			ImGui::Text("Metalness:");
			ImGui::SetNextItemWidth(fieldWidth);
			float metalness = ObjToWorkWith->getMetalness();
			ImGui::DragFloat("##Metalness", &metalness, 0.01f, 0.0f, 1.0f);
			ObjToWorkWith->setMetalness(metalness);
		}
		else
		{
			ImGui::Text("Metalness map intensity:");
			ImGui::SetNextItemWidth(fieldWidth);
			float metalness = ObjToWorkWith->getMetalnessMapIntensity();
			ImGui::DragFloat("##Metalness map intensity", &metalness, 0.01f, 0.0f, 10.0f);
			ObjToWorkWith->setMetalnessMapIntensity(metalness);
		}

		ImGui::PopStyleColor();
		ImGui::EndChild();
	}
#endif // USE_NODES

	CancelButton->Render();
	if (CancelButton->IsClicked())
	{
		FEImGuiWindow::Close();
		return;
	}

	FEImGuiWindow::OnRenderEnd();
}

void EditMaterialPopup::Close()
{
	FEImGuiWindow::Close();
}

#ifdef USE_NODES
bool EditMaterialPopup::DragAndDropnodeAreaTargetCallback(FEObject* Object, void** CallbackInfo)
{
	if (ObjToWorkWith->IsTextureInList(RESOURCE_MANAGER.GetTexture(Object->GetObjectID())))
		return false;

	if (ObjToWorkWith->GetUsedTexturesCount() == FE_MAX_TEXTURES_PER_MATERIAL)
		return false;

	FEEditorTextureSourceNode* NewNode = new FEEditorTextureSourceNode(RESOURCE_MANAGER.GetTexture(Object->GetObjectID()));

	ImVec2 PositionOnCanvas;
	PositionOnCanvas.x = ImGui::GetMousePos().x - (WindowPosition.x + NodeGridRelativePosition.x) - NewNode->GetSize().x / 2.0f;
	PositionOnCanvas.y = ImGui::GetMousePos().y - (WindowPosition.y + NodeGridRelativePosition.y) - NewNode->GetSize().y / 2.0f;

	NewNode->SetPosition(PositionOnCanvas);
	MaterialNodeArea->AddNode(NewNode);
	return true;
}
#else
bool editMaterialPopup::dragAndDropCallback(FEObject* object, void** oldTexture)
{
	FETexture* newTexture = RESOURCE_MANAGER.getTexture(object->getObjectID());
	*oldTexture = reinterpret_cast<void*>(newTexture);

	return true;
}

bool editMaterialPopup::dragAndDropTexturesListCallback(FEObject* object, void** material)
{
	reinterpret_cast<FEMaterial*>(*material)->addTexture(RESOURCE_MANAGER.getTexture(object->getObjectID()));
	return true;
}

bool editMaterialPopup::dragAndDropMaterialBindingsCallback(FEObject* object, void** callbackInfoPointer)
{
	materialBindingCallbackInfo* info = reinterpret_cast<materialBindingCallbackInfo*>(callbackInfoPointer);
	FEMaterial* material = reinterpret_cast<FEMaterial*>(*info->material);


	if (!material->isTextureInList(RESOURCE_MANAGER.getTexture(object->getObjectID())))
		material->addTexture(RESOURCE_MANAGER.getTexture(object->getObjectID()));

	int subMaterial = info->textureBinding > 5;
	if (subMaterial)
		info->textureBinding -= 6;

	switch (info->textureBinding)
	{
	case 0:
	{
		material->setAlbedoMap(RESOURCE_MANAGER.getTexture(object->getObjectID()), subMaterial);
		break;
	}

	case 1:
	{
		material->setNormalMap(RESOURCE_MANAGER.getTexture(object->getObjectID()), subMaterial);
		break;
	}

	case 2:
	{
		material->setAOMap(RESOURCE_MANAGER.getTexture(object->getObjectID()), 0, subMaterial);
		break;
	}

	case 3:
	{
		material->setRoughtnessMap(RESOURCE_MANAGER.getTexture(object->getObjectID()), 0, subMaterial);
		break;
	}

	case 4:
	{
		material->setMetalnessMap(RESOURCE_MANAGER.getTexture(object->getObjectID()), 0, subMaterial);
		break;
	}

	case 5:
	{
		material->setDisplacementMap(RESOURCE_MANAGER.getTexture(object->getObjectID()), 0, subMaterial);
		break;
	}

	default:
		break;
	}

	return true;
}
#endif // USE_NODES


#ifdef USE_NODES
void EditMaterialPopup::NodeSystemMainContextMenu()
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
			MaterialNodeArea->AddNode(NewNode);
		}

		ImGui::EndMenu();
	}
}

void EditMaterialPopup::TextureNodeCallback(FEVisualNode* Node, const FE_VISUAL_NODE_EVENT EventWithNode)
{
	if (Node == nullptr)
		return;

	if (Node->GetType() != "FEEditorTextureSourceNode")
		return;

	if (EventWithNode != FE_VISUAL_NODE_DESTROYED && EventWithNode != FE_VISUAL_NODE_REMOVED)
		return;

	const FEEditorTextureSourceNode* CurrentNode = reinterpret_cast<FEEditorTextureSourceNode*>(Node);
	if (ObjToWorkWith->IsTextureInList(CurrentNode->GetTexture()))
	{
		ObjToWorkWith->RemoveTexture(CurrentNode->GetTexture());
	}
}

void EditMaterialPopup::TextureNodeCreationCallback(const std::vector<FEObject*> SelectionsResult)
{
	if (SelectionsResult.size() != 1 && SelectionsResult[0]->GetType() != FE_TEXTURE)
		return;

	if (SelectionsResult[0] != nullptr && SelectionsResult[0] != RESOURCE_MANAGER.NoTexture)
	{
		if (!ObjToWorkWith->IsTextureInList(reinterpret_cast<FETexture*>(SelectionsResult[0])))
		{
			if (ObjToWorkWith->AddTexture(reinterpret_cast<FETexture*>(SelectionsResult[0])))
			{
				FEEditorTextureSourceNode* NewNode = new FEEditorTextureSourceNode(reinterpret_cast<FETexture*>(SelectionsResult[0]));

				ImVec2 PositionOnCanvas;
				PositionOnCanvas.x = MousePositionWhenContextMenuWasOpened.x - (WindowPosition.x + NodeGridRelativePosition.x) - NewNode->GetSize().x / 2.0f;
				PositionOnCanvas.y = MousePositionWhenContextMenuWasOpened.y - (WindowPosition.y + NodeGridRelativePosition.y) - NewNode->GetSize().y / 2.0f;

				NewNode->SetPosition(PositionOnCanvas);
				MaterialNodeArea->AddNode(NewNode);
			}
		}
	}
}
#endif // USE_NODES
