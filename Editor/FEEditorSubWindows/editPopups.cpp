#include "editPopups.h"

editGameModelPopup* editGameModelPopup::_instance = nullptr;

editGameModelPopup::editGameModelPopup()
{
	tempModel = new FEGameModel(nullptr, nullptr, "tempGameModel");
	objToWorkWith = nullptr;
	flags = ImGuiWindowFlags_NoResize;

	cancelButton = new ImGuiButton("Cancel");
	cancelButton->setDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
	cancelButton->setHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
	cancelButton->setActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));

	applyButton = new ImGuiButton("Apply");
	changeMaterialButton = new ImGuiButton("Change Material");
	changeBillboardMaterialButton = new ImGuiButton("Change Billboard Material");
	deleteLODMeshButton = new ImGuiButton("X");
	addBillboard = new ImGuiButton("Add Billboard");

	LODGroups = new FERangeConfigurator();
	LODGroups->setSize(ImVec2(870.0f, 40.0f));
	LODGroups->setPosition(ImVec2(920.0f / 2.0f - 870.0f / 2.0f, 650.0f));

	LODColors.push_back(ImColor(0, 255, 0, 255));
	LODColors.push_back(ImColor(0, 0, 255, 255));
	LODColors.push_back(ImColor(0, 255, 255, 255));
	LODColors.push_back(ImColor(0, 255, 125, 255));
}

editGameModelPopup::~editGameModelPopup()
{
	if (cancelButton != nullptr)
		delete cancelButton;

	if (applyButton != nullptr)
		delete applyButton;

	if (changeMaterialButton != nullptr)
		delete changeMaterialButton;

	if (changeBillboardMaterialButton != nullptr)
		delete changeBillboardMaterialButton;

	for (size_t i = 0; i < changeLODMeshButton.size(); i++)
	{
		delete changeLODMeshButton[i];
	}

	if (deleteLODMeshButton != nullptr)
		delete deleteLODMeshButton;

	if (addBillboard != nullptr)
		delete addBillboard;

	if (LODGroups != nullptr)
		delete LODGroups;
}

void editGameModelPopup::show(FEGameModel* GameModel)
{
	if (GameModel != nullptr)
	{
		switchMode(NO_LOD_MODE);
		size = ImVec2(NO_LOD_WINDOW_WIDTH, NO_LOD_WINDOW_HEIGHT);
		objToWorkWith = GameModel;

		flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

		tempModel->setMaterial(objToWorkWith->getMaterial());
		tempModel->setScaleFactor(objToWorkWith->getScaleFactor());
		updatedMaterial = objToWorkWith->getMaterial();
		updatedBillboardMaterial = objToWorkWith->getBillboardMaterial();
		tempModel->setUsingLODlevels(objToWorkWith->useLODlevels());
		tempModel->setBillboardZeroRotaion(objToWorkWith->getBillboardZeroRotaion());

		float zeroRotation = tempModel->getBillboardZeroRotaion();

		changeLODMeshButton.clear();
		if (objToWorkWith->useLODlevels())
		{
			changeLODMeshButton.resize(objToWorkWith->getMaxLODCount());
			for (size_t i = 0; i < objToWorkWith->getMaxLODCount(); i++)
			{
				std::string buttonName = "Change LOD" + std::to_string(i) + " Mesh";
				changeLODMeshButton[i] = new ImGuiButton(buttonName);
				changeLODMeshButton[i]->setSize(ImVec2(200, 35));
			}
		}
		else
		{
			changeLODMeshButton.resize(1);
			changeLODMeshButton[0] = new ImGuiButton("Change Mesh");
			changeLODMeshButton[0]->setSize(ImVec2(200, 35));
			changeLODMeshButton[0]->setPosition(ImVec2(size.x / 2.0f - size.x / 4.0f - changeLODMeshButton[0]->getSize().x / 2, 35 + 340.0f));
		}

		updatedLODMeshs.clear();
		for (size_t i = 0; i < objToWorkWith->getMaxLODCount(); i++)
		{
			tempModel->setLODMesh(i, objToWorkWith->getLODMesh(i));
			tempModel->setLODMaxDrawDistance(i, objToWorkWith->getLODMaxDrawDistance(i));
			tempModel->setIsLODBillboard(i, objToWorkWith->isLODBillboard(i));
			updatedLODMeshs.push_back(objToWorkWith->getLODMesh(i));
		}
		tempModel->setCullDistance(objToWorkWith->getCullDistance());

		std::string tempCaption = "Edit game model:";
		tempCaption += " " + objToWorkWith->getName();
		strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());

		position = ImVec2(FEngine::getInstance().getWindowWidth() / 2 - size.x / 2, FEngine::getInstance().getWindowHeight() / 2 - size.y / 2);
		FEImGuiWindow::show();

		PREVIEW_MANAGER.createGameModelPreview(tempModel, &tempPreview);

		changeMaterialButton->setSize(ImVec2(200, 35));
		changeMaterialButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - changeMaterialButton->getSize().x / 2, 35 + 340.0f));
		changeBillboardMaterialButton->setSize(ImVec2(270, 35));
		changeBillboardMaterialButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - changeBillboardMaterialButton->getSize().x / 2, 35 + 340.0f));

		deleteLODMeshButton->setSize(ImVec2(24, 25));
		deleteLODMeshButton->setDefaultColor(ImVec4(0.9f, 0.5f, 0.5f, 1.0f));

		addBillboard->setSize(ImVec2(200, 35));

		applyButton->setPosition(ImVec2(size.x / 4 - applyButton->getSize().x / 2, size.y - 40));
		cancelButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - cancelButton->getSize().x / 2, size.y - 40));

		// ************** LOD Groups **************
		LODGroups->clear();
		for (size_t i = 0; i < objToWorkWith->getMaxLODCount(); i++)
		{
			if (objToWorkWith->getLODMesh(i) != nullptr)
			{
				LODGroups->addRange((objToWorkWith->getLODMaxDrawDistance(i)) / objToWorkWith->getCullDistance(), std::string("LOD") + std::to_string(i), "", LODColors[i]);
			}
		}
		// ************** LOD Groups END **************

		// ************** Drag&Drop **************
		LODsMeshCallbackInfo.resize(objToWorkWith->getMaxLODCount());
		LODsMeshTarget.resize(objToWorkWith->getMaxLODCount());
		for (size_t i = 0; i < objToWorkWith->getMaxLODCount(); i++)
		{
			LODsMeshCallbackInfo[i].LODLevel = i;
			LODsMeshCallbackInfo[i].window = this;
			LODsMeshTarget[i] = DRAG_AND_DROP_MANAGER.addTarget(FE_MESH, dragAndDropLODMeshCallback, reinterpret_cast<void**>(&LODsMeshCallbackInfo[i]), "Drop to assing LOD" + std::to_string(i) + " mesh");
		}

		materialCallbackInfo;
		materialCallbackInfo.billboardMaterial = false;
		materialCallbackInfo.window = this;

		billboardMaterialCallbackInfo;
		billboardMaterialCallbackInfo.billboardMaterial = true;
		billboardMaterialCallbackInfo.window = this;

		materialTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_MATERIAL, dragAndDropMaterialCallback, reinterpret_cast<void**>(&materialCallbackInfo), "Drop to assing material");
		billboardMaterialTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_MATERIAL, dragAndDropMaterialCallback, reinterpret_cast<void**>(&billboardMaterialCallbackInfo), "Drop to assing billboard material");
		// ************** Drag&Drop END **************
	}
}

void editGameModelPopup::switchMode(int toMode)
{
	switch (toMode)
	{
		case NO_LOD_MODE:
		{
			if (currentMode == NO_LOD_MODE)
				return;
			size.x = NO_LOD_WINDOW_WIDTH;
			size.y = NO_LOD_WINDOW_HEIGHT;
			currentMode = NO_LOD_MODE;
			changeLODMeshButton[0]->setCaption("Change Mesh");
			changeLODMeshButton[0]->setPosition(ImVec2(size.x / 2.0f - size.x / 4.0f - changeLODMeshButton[0]->getSize().x / 2, 35 + 340.0f));
			changeMaterialButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - changeMaterialButton->getSize().x / 2, 35 + 340.0f));
			applyButton->setPosition(ImVec2(size.x / 4 - applyButton->getSize().x / 2, size.y - 40));
			cancelButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - cancelButton->getSize().x / 2, size.y - 40));

			for (size_t i = 1; i < tempModel->getMaxLODCount(); i++)
			{
				updatedLODMeshs[i] = nullptr;
				tempModel->setLODMesh(i, nullptr);
			}

			updatedBillboardMaterial = nullptr;
			tempModel->setBillboardMaterial(nullptr);

			LODGroups->clear();
			LODGroups->addRange((objToWorkWith->getLODMaxDrawDistance(0)) / objToWorkWith->getCullDistance(), "LOD0", "", ImColor(0, 255, 0, 255));
			break;
		}

		case HAS_LOD_MODE:
		{
			changeLODMeshButton.resize(objToWorkWith->getMaxLODCount());
			for (size_t i = 0; i < objToWorkWith->getMaxLODCount(); i++)
			{
				std::string buttonName = "Change LOD" + std::to_string(i) + " Mesh";
				changeLODMeshButton[i] = new ImGuiButton(buttonName);
				changeLODMeshButton[i]->setSize(ImVec2(200, 35));
			}

			if (currentMode == HAS_LOD_MODE)
				return;
			size.x = 920.0f;
			size.y = 830.0f;
			currentMode = HAS_LOD_MODE;
			changeMaterialButton->setPosition(ImVec2(size.x / 2.0f - size.x / 4.0f - changeMaterialButton->getSize().x / 2, 35.0f + 340.0f + 200.0f));
			changeBillboardMaterialButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - changeBillboardMaterialButton->getSize().x / 2, 35 + 340.0f + 200.0f));
			applyButton->setPosition(ImVec2(size.x / 4 - applyButton->getSize().x / 2, size.y - 40));
			cancelButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - cancelButton->getSize().x / 2, size.y - 40));
			break;
		}
		default:
			break;
	}
}

void editGameModelPopup::displayLODGroups()
{
	ImVec2 textSize = ImGui::CalcTextSize("LOD Groups: ");
	ImGui::SetCursorPosX(size.x / 2 - textSize.x / 2);
	ImGui::SetCursorPosY(changeBillboardMaterialButton->getPosition().y + changeBillboardMaterialButton->getSize().y + 10.0f);
	ImGui::Text("LOD Groups: ");

	LODGroups->render();

	float totalRangeSpan = 0.0f;
	for (size_t i = 0; i < tempModel->getMaxLODCount(); i++)
	{
		FERangeRecord* record = LODGroups->getRangesRecord(i);
		if (record == nullptr)
			break;

		float rangeSpan = record->getRangeSpan();
		totalRangeSpan += rangeSpan;

		if (tempModel->getLODMesh(i) == nullptr)
			break;

		if (tempModel->isLODBillboard(i))
		{
			record->setCaption(std::string("Billboard"));
		}
		else
		{
			record->setCaption(std::string("LOD") + std::to_string(i));
		}

		tempModel->setLODMaxDrawDistance(i, tempModel->getCullDistance() * totalRangeSpan);
		std::string newToolTip;
		if (i == 0)
		{
			newToolTip = record->getCaption() + "(0 - " + std::to_string(int(tempModel->getLODMaxDrawDistance(i))) + "m) " + std::to_string(rangeSpan * 100.0f) + "%";
		}
		else
		{
			int endDistance = int(tempModel->getLODMaxDrawDistance(i));
			if (i == tempModel->getMaxLODCount() - 1 || tempModel->getLODMesh(i + 1) == nullptr)
				endDistance = int(tempModel->getCullDistance());
			newToolTip = record->getCaption() + "(" + std::to_string(int(tempModel->getLODMaxDrawDistance(i - 1))) + " - " + std::to_string(endDistance) + "m) " + std::to_string(rangeSpan * 100.0f) + "%";
		}

		record->setToolTipText(newToolTip);
	}

	ImGui::SetCursorPosY(LODGroups->getPosition().y + LODGroups->getSize().y + 10.0f);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
	ImGui::Text("cullDistance:");
	float currentCullRange = tempModel->getCullDistance();
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
	ImGui::DragFloat("##cullDistance", &currentCullRange, 1.0f, 0.1f, 5000.0f);
	tempModel->setCullDistance(currentCullRange);
}

void editGameModelPopup::render()
{
	ImGui::SetNextWindowSize(size);
	FEImGuiWindow::render();

	if (!isVisible())
		return;

	// if we change something we will update preview.
	if (updatedMaterial != tempModel->getMaterial())
	{
		tempModel->setMaterial(updatedMaterial);
		PREVIEW_MANAGER.createGameModelPreview(tempModel, &tempPreview);
	}

	if (updatedBillboardMaterial != tempModel->getBillboardMaterial())
	{
		tempModel->setBillboardMaterial(updatedBillboardMaterial);
	}

	for (size_t i = 0; i < tempModel->getMaxLODCount(); i++)
	{
		if (updatedLODMeshs[i] != tempModel->getLODMesh(i))
		{
			if (tempModel->getLODMesh(i) == nullptr)
			{
				LODGroups->addRange((tempModel->getLODMaxDrawDistance(i)) / tempModel->getCullDistance(), std::string("LOD") + std::to_string(i), "", LODColors[i]);
			}

			tempModel->setLODMesh(i, updatedLODMeshs[i]);
			PREVIEW_MANAGER.createGameModelPreview(tempModel, &tempPreview);
		}
	}

	if (objToWorkWith == nullptr)
	{
		FEImGuiWindow::close();
		return;
	}

	float baseY = 35.0f;
	float currentY = baseY;

	bool isLODsActive = tempModel->useLODlevels();
	ImGui::SetCursorPosY(currentY);
	ImGui::Checkbox("have LOD levels", &isLODsActive);
	tempModel->setUsingLODlevels(isLODsActive);
	if (tempModel->useLODlevels())
	{
		switchMode(HAS_LOD_MODE);
	}
	else
	{
		switchMode(NO_LOD_MODE);
	}

	ImVec2 textSize = ImGui::CalcTextSize("Preview of game model:");
	ImGui::SetCursorPosX(size.x / 2 - textSize.x / 2);
	ImGui::SetCursorPosY(currentY + 30);
	ImGui::Text("Preview of game model:");
	ImGui::SetCursorPosX(size.x / 2 - 128 / 2);
	ImGui::SetCursorPosY(currentY + 50);
	ImGui::Image((void*)(intptr_t)tempPreview->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	if (currentMode == NO_LOD_MODE)
	{
		ImGui::Separator();
		textSize = ImGui::CalcTextSize("Mesh component:");
		ImGui::SetCursorPosX(size.x / 4 - textSize.x / 2);
		ImGui::Text("Mesh component:");
		ImGui::SetCursorPosX(size.x / 4 - 128 / 2);
		ImGui::SetCursorPosY(currentY + 210.0f);
		ImGui::Image((void*)(intptr_t)PREVIEW_MANAGER.getMeshPreview(tempModel->mesh->getObjectID())->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		LODsMeshTarget[0]->stickToItem();

		changeLODMeshButton[0]->render();
		if (changeLODMeshButton[0]->getWasClicked())
		{
			updatedLODMeshs[0] = tempModel->getLODMesh(0);
			selectMeshPopUp::getInstance().show(&updatedLODMeshs[0]);
		}

		textSize = ImGui::CalcTextSize("Material component:");
		ImGui::SetCursorPosX(size.x / 2 + size.x / 4 - textSize.x / 2);
		ImGui::SetCursorPosY(currentY + 187.0f);
		ImGui::Text("Material component:");
		ImGui::SetCursorPosX(size.x / 2 + size.x / 4 - 128 / 2);
		ImGui::SetCursorPosY(currentY + 210.0f);
		ImGui::Image((void*)(intptr_t)PREVIEW_MANAGER.getMaterialPreview(tempModel->material->getObjectID())->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
		materialTarget->stickToItem();
		changeMaterialButton->render();
		if (changeMaterialButton->getWasClicked())
		{
			updatedMaterial = tempModel->getMaterial();
			selectMaterialPopUp::getInstance().show(&updatedMaterial);
		}
	}
	else if (currentMode == HAS_LOD_MODE)
	{
		ImGui::Separator();

		float baseXPosition = size.x / 2.0f - size.x / 4.0f;
		for (size_t i = 0; i < tempModel->getMaxLODCount(); i++)
		{
			float currentXPosition = baseXPosition + (size.x / 4.0f) * i - size.x / 8.0f;
			if (tempModel->getLODMesh(i) == nullptr)
			{
				if (tempModel->isLODBillboard(i - 1))
					break;

				changeLODMeshButton[i]->setCaption(std::string("Add LOD") + std::to_string(i));
				changeLODMeshButton[i]->setPosition(ImVec2(currentXPosition - changeLODMeshButton[i]->getSize().x / 2, currentY + 210.0f + 128.0f / 2.0f - 10.0f - changeLODMeshButton[i]->getSize().y/*- changeLODMeshButton[i]->getSize().y / 2.0f*/));

				changeLODMeshButton[i]->render();
				if (changeLODMeshButton[i]->getWasClicked())
				{
					updatedLODMeshs[i] = tempModel->getLODMesh(i);
					selectMeshPopUp::getInstance().show(&updatedLODMeshs[i]);
				}

				addBillboard->setPosition(ImVec2(currentXPosition - addBillboard->getSize().x / 2, currentY + 210.0f + 128.0f / 2.0f + 10.0f / 2.0f));
				addBillboard->render();
				if (addBillboard->getWasClicked())
				{
					updatedLODMeshs[i] = RESOURCE_MANAGER.getMesh("1Y251E6E6T78013635793156"/*"plane"*/);
					updatedBillboardMaterial = RESOURCE_MANAGER.getMaterial("61649B9E0F08013Q3939316C"/*"FEPBRBaseMaterial"*/);
					tempModel->setIsLODBillboard(i, true);
				}

				break;
			}
			else
			{
				std::string caption = (std::string("LOD") + std::to_string(i)) + ":";
				if (tempModel->isLODBillboard(i))
					caption = "Billboard:";

				textSize = ImGui::CalcTextSize(caption.c_str());
				ImGui::SetCursorPosX(currentXPosition - textSize.x / 2.0f);
				ImGui::SetCursorPosY(currentY + 187.0f);
				ImGui::Text(caption.c_str());
				ImGui::SetCursorPosX(currentXPosition - 128 / 2);
				ImGui::SetCursorPosY(currentY + 210.0f);

				ImGui::Image((void*)(intptr_t)(tempModel->getLODMesh(i) == nullptr ? RESOURCE_MANAGER.noTexture->getTextureID() : PREVIEW_MANAGER.getMeshPreview(tempModel->getLODMesh(i)->getObjectID())->getTextureID()),
					ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
				LODsMeshTarget[i]->stickToItem();

				changeLODMeshButton[i]->setCaption(std::string("Change LOD") + std::to_string(i) + " Mesh");
				if (tempModel->isLODBillboard(i))
					changeLODMeshButton[i]->setCaption(std::string("Change Billboard"));
				changeLODMeshButton[i]->setPosition(ImVec2(currentXPosition - changeLODMeshButton[i]->getSize().x / 2, 35 + 340.0f));

				if (isLastSetupLOD(i) && i > 0)
				{
					deleteLODMeshButton->setPosition(ImVec2(currentXPosition + 80.0f, 215.0f));
					deleteLODMeshButton->render();
					if (deleteLODMeshButton->getWasClicked())
					{
						updatedLODMeshs[i] = nullptr;
						tempModel->setLODMesh(i, nullptr);
						tempModel->setIsLODBillboard(i, false);
						LODGroups->deleteRange(i);
					}
				}
			}

			changeLODMeshButton[i]->render();
			if (changeLODMeshButton[i]->getWasClicked())
			{
				updatedLODMeshs[i] = tempModel->getLODMesh(i);
				selectMeshPopUp::getInstance().show(&updatedLODMeshs[i]);
			}
		}

		for (size_t i = 0; i < tempModel->getMaxLODCount(); i++)
		{
			if (isLastSetupLOD(i))
			{
				if (tempModel->isLODBillboard(i))
				{
					textSize = ImGui::CalcTextSize("Material component:");
					ImGui::SetCursorPosX(size.x / 2 - size.x / 4 - textSize.x / 2);
					ImGui::SetCursorPosY(currentY + 200 + 187.0f);
					ImGui::Text("Material component:");
					ImGui::SetCursorPosX(size.x / 2 - size.x / 4 - 128 / 2);
					ImGui::SetCursorPosY(currentY + 200 + 210.0f);
					ImGui::Image((void*)(intptr_t)PREVIEW_MANAGER.getMaterialPreview(tempModel->getMaterial()->getObjectID())->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					materialTarget->stickToItem();

					changeMaterialButton->setPosition(ImVec2(size.x / 2 - size.x / 4 - changeMaterialButton->getSize().x / 2, baseY + 340.0f + 200.0f));
					changeMaterialButton->render();
					if (changeMaterialButton->getWasClicked())
					{
						updatedMaterial = tempModel->getMaterial();
						selectMaterialPopUp::getInstance().show(&updatedMaterial);
					}

					textSize = ImGui::CalcTextSize("Billboard Material component:");
					ImGui::SetCursorPosX(size.x / 2 + size.x / 4 - textSize.x / 2);
					ImGui::SetCursorPosY(currentY + 200 + 187.0f);
					ImGui::Text("Billboard Material component:");
					ImGui::SetCursorPosX(size.x / 2 + size.x / 4 - 128 / 2);
					ImGui::SetCursorPosY(currentY + 200 + 210.0f);
					ImGui::Image((void*)(intptr_t)(tempModel->getBillboardMaterial() == nullptr ? RESOURCE_MANAGER.noTexture->getTextureID() : PREVIEW_MANAGER.getMaterialPreview(tempModel->getBillboardMaterial()->getObjectID())->getTextureID()), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
					billboardMaterialTarget->stickToItem();

					changeBillboardMaterialButton->render();
					if (changeBillboardMaterialButton->getWasClicked())
					{
						updatedBillboardMaterial = tempModel->getBillboardMaterial();
						selectMaterialPopUp::getInstance().setAllowedShader(RESOURCE_MANAGER.getShader("0800253C242B05321A332D09"/*"FEPBRShader"*/));
						selectMaterialPopUp::getInstance().show(&updatedBillboardMaterial);
					}
				}
				else
				{
					textSize = ImGui::CalcTextSize("Material component:");
					ImGui::SetCursorPosX(size.x / 2 - textSize.x / 2);
					ImGui::SetCursorPosY(currentY + 200 + 187.0f);
					ImGui::Text("Material component:");
					ImGui::SetCursorPosX(size.x / 2 - 128 / 2);
					ImGui::SetCursorPosY(currentY + 200 + 210.0f);
					ImGui::Image((void*)(intptr_t)PREVIEW_MANAGER.getMaterialPreview(tempModel->getMaterial()->getObjectID())->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

					changeMaterialButton->setPosition(ImVec2(size.x / 2 - changeMaterialButton->getSize().x / 2, baseY + 340.0f + 200.0f));
					changeMaterialButton->render();
					if (changeMaterialButton->getWasClicked())
					{
						updatedMaterial = tempModel->getMaterial();
						selectMaterialPopUp::getInstance().show(&updatedMaterial);
					}
				}

				break;
			}
		}

		// ************** LOD Groups **************
		ImGui::Separator();
		displayLODGroups();

		bool hasBillboard = false;
		for (size_t i = 0; i < tempModel->getMaxLODCount(); i++)
		{
			if (tempModel->isLODBillboard(i))
				hasBillboard = true;
		}

		if (hasBillboard)
		{
			size.y = 830.0f + 40.0f;
			applyButton->setPosition(ImVec2(size.x / 4 - applyButton->getSize().x / 2, size.y - 40));
			cancelButton->setPosition(ImVec2(size.x / 2 + size.x / 4 - cancelButton->getSize().x / 2, size.y - 40));

			/*ImGui::Text("Billboard Scale:");
			float billboardScale = tempModel->getBillboardScale();
			ImGui::SameLine();
			ImGui::DragFloat("##Billboard Scale", &billboardScale, 0.1f);
			tempModel->setBillboardScale(billboardScale);*/

			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			ImGui::Text("Billboard Zero Rotation:");
			float zeroRotation = tempModel->getBillboardZeroRotaion();
			ImGui::SameLine();
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
			ImGui::DragFloat("##Billboard Zero Rotation", &zeroRotation, 0.1f, 0.0f, 360.0f);
			tempModel->setBillboardZeroRotaion(zeroRotation);
		}
	}

	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
	ImGui::Text("Scale Factor:");
	float scaleFactor = tempModel->getScaleFactor();
	ImGui::SameLine();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
	ImGui::SetNextItemWidth(200);
	ImGui::DragFloat("##Scale Factor", &scaleFactor, 0.1f);
	tempModel->setScaleFactor(scaleFactor);

	ImGui::SetCursorPosY(size.y - 50.0f);
	ImGui::Separator();
	ImGui::SetItemDefaultFocus();
	applyButton->render();
	if (applyButton->getWasClicked())
	{
		objToWorkWith->dirtyFlag = true;
		objToWorkWith->mesh = tempModel->mesh;
		objToWorkWith->setUsingLODlevels(tempModel->useLODlevels());

		for (size_t i = 0; i < tempModel->getMaxLODCount(); i++)
		{
			objToWorkWith->setLODMesh(i, tempModel->getLODMesh(i));
			objToWorkWith->setLODMaxDrawDistance(i, tempModel->getLODMaxDrawDistance(i));
			objToWorkWith->setIsLODBillboard(i, tempModel->isLODBillboard(i));
		}

		objToWorkWith->setCullDistance(tempModel->getCullDistance());
		objToWorkWith->setMaterial(tempModel->getMaterial());
		objToWorkWith->setBillboardMaterial(tempModel->getBillboardMaterial());
		objToWorkWith->setScaleFactor(tempModel->getScaleFactor());
		objToWorkWith->setBillboardZeroRotaion(tempModel->getBillboardZeroRotaion());
		PREVIEW_MANAGER.createGameModelPreview(objToWorkWith->getObjectID());

		FEImGuiWindow::close();
		return;
	}

	ImGui::SameLine();
	cancelButton->render();
	if (cancelButton->getWasClicked())
	{
		FEImGuiWindow::close();
		return;
	}

	FEImGuiWindow::onRenderEnd();
}

void editGameModelPopup::close()
{
	FEImGuiWindow::close();
}

// ************** Drag&Drop **************
bool editGameModelPopup::dragAndDropLODMeshCallback(FEObject* object, void** callbackInfo)
{
	meshTargetCallbackInfo* info = reinterpret_cast<meshTargetCallbackInfo*>(callbackInfo);
	info->window->updatedLODMeshs[info->LODLevel] = RESOURCE_MANAGER.getMesh(object->getObjectID());
	info->window->tempModel->setLODMesh(info->LODLevel, RESOURCE_MANAGER.getMesh(object->getObjectID()));
	PREVIEW_MANAGER.createGameModelPreview(info->window->tempModel, &info->window->tempPreview);
	return true;
}

bool editGameModelPopup::dragAndDropMaterialCallback(FEObject* object, void** callbackInfo)
{
	materialTargetCallbackInfo* info = reinterpret_cast<materialTargetCallbackInfo*>(callbackInfo);

	if (info->billboardMaterial)
	{
		info->window->updatedBillboardMaterial = RESOURCE_MANAGER.getMaterial(object->getObjectID());
		info->window->tempModel->setBillboardMaterial(RESOURCE_MANAGER.getMaterial(object->getObjectID()));
	}
	else
	{
		info->window->updatedMaterial = RESOURCE_MANAGER.getMaterial(object->getObjectID());
		info->window->tempModel->setMaterial(RESOURCE_MANAGER.getMaterial(object->getObjectID()));
		PREVIEW_MANAGER.createGameModelPreview(info->window->tempModel, &info->window->tempPreview);
	}

	return true;
}
// ************** Drag&Drop END **************

bool editGameModelPopup::isLastSetupLOD(size_t LODindex)
{
	if (LODindex >= tempModel->getMaxLODCount())
		return false;

	if (LODindex == tempModel->getMaxLODCount() - 1)
		return true;

	if (tempModel->isLODBillboard(LODindex))
		return true;

	for (size_t i = LODindex + 1; i < tempModel->getMaxLODCount(); i++)
	{
		if (tempModel->getLODMesh(i) != nullptr)
			return false;
	}

	return true;
}

editMaterialPopup* editMaterialPopup::_instance = nullptr;

editMaterialPopup::editMaterialPopup()
{
	objToWorkWith = nullptr;
	flags = ImGuiWindowFlags_NoResize;

	cancelButton = new ImGuiButton("Cancel");
	cancelButton->setDefaultColor(ImVec4(0.7f, 0.5f, 0.5f, 1.0f));
	cancelButton->setHoveredColor(ImVec4(0.95f, 0.5f, 0.0f, 1.0f));
	cancelButton->setActiveColor(ImVec4(0.1f, 1.0f, 0.1f, 1.0f));
#ifdef USE_NODES
	nodeAreaTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_TEXTURE, dragAndDropnodeAreaTargetCallback, reinterpret_cast<void**>(&dragAndDropCallbackInfo), "Drop to add texture");
#endif // USE_NODES
}

editMaterialPopup::~editMaterialPopup()
{
	if (cancelButton != nullptr)
		delete cancelButton;

	if (iconButton != nullptr)
		delete iconButton;
}

#ifdef USE_NODES
FEMaterial* editMaterialPopup::objToWorkWith = nullptr;
FEEditorNodeArea* editMaterialPopup::materialNodeArea = nullptr;
ImVec2 editMaterialPopup::nodeGridRelativePosition = ImVec2(5, 30);
ImVec2 editMaterialPopup::windowPosition = ImVec2(0, 0);
ImVec2 editMaterialPopup::mousePositionWhenContextMenuWasOpened = ImVec2(0, 0);
FETexture* editMaterialPopup::textureForNewNode = nullptr;
#endif // USE_NODES
void editMaterialPopup::show(FEMaterial* material)
{
	if (material != nullptr)
	{
		tempContainer = RESOURCE_MANAGER.noTexture;
		objToWorkWith = material;

		std::string tempCaption = "Edit material:";
		tempCaption += " " + objToWorkWith->getName();
		strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());
#ifdef USE_NODES
		size = ImVec2(1500.0f, 1000.0f);
#else
		size = ImVec2(1500.0f, 700.0f);
#endif // USE_NODES
		position = ImVec2(FEngine::getInstance().getWindowWidth() / 2 - size.x / 2, FEngine::getInstance().getWindowHeight() / 2 - size.y / 2);
		FEImGuiWindow::show();

		iconButton = new ImGuiImageButton(nullptr);
		iconButton->setSize(ImVec2(128, 128));
		iconButton->setUV0(ImVec2(0.0f, 0.0f));
		iconButton->setUV1(ImVec2(1.0f, 1.0f));
		iconButton->setFramePadding(8);

#ifdef USE_NODES
		materialNodeArea = NODE_SYSTEM.createNodeArea();
		materialNodeArea->setMainContextMenuFunc(nodeSystemMainContextMenu);
		materialNodeArea->setNodeEventCallback(textureNodeCallback);

		FEEditorMaterialNode* newNode = new FEEditorMaterialNode(material);

		ImVec2 positionOnCanvas;
		positionOnCanvas.x = nodeGridRelativePosition.x + size.x - size.x / 6 - newNode->getSize().x / 2.0f;
		positionOnCanvas.y = nodeGridRelativePosition.y + size.y / 2 - newNode->getSize().y / 2.0f;
		newNode->setPosition(positionOnCanvas);

		materialNodeArea->addNode(newNode);

		// Add all textures of material as a texture nodes
		// Place them in shifted grid.
		int visualIndex = 0;
		for (size_t i = 0; i < material->textures.size(); i++)
		{
			if (material->textures[i] == nullptr)
				continue;

			FEEditorTextureSourceNode* newTextureNode = new FEEditorTextureSourceNode(material->textures[i]);

			positionOnCanvas.x = nodeGridRelativePosition.x + size.x / 2 - newTextureNode->getSize().x / 2.0f - (visualIndex % 2 == 0 ? (newTextureNode->getSize().x + 24.0f) : 0.0f);
			positionOnCanvas.y = nodeGridRelativePosition.y + visualIndex / 2.0f * (newTextureNode->getSize().y + 24.0f);
			newTextureNode->setPosition(positionOnCanvas);

			materialNodeArea->addNode(newTextureNode);
			visualIndex++;

			// We should recreate proper connections.
			if (material->getAlbedoMap() == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, 5, newNode, 0);
			}
			else if (material->getAlbedoMap(1) == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, 5, newNode, 6);
			}

			if (material->getNormalMap() == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, 5, newNode, 1);
			}
			else if (material->getNormalMap(1) == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, 5, newNode, 7);
			}

			if (material->getAOMap() == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, objToWorkWith->getAOMapChannel(), newNode, 2);
			}
			else if (material->getAOMap(1) == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, objToWorkWith->getAOMapChannel(), newNode, 8);
			}

			if (material->getRoughtnessMap() == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, objToWorkWith->getRoughtnessMapChannel(), newNode, 3);
			}
			else if (material->getRoughtnessMap(1) == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, objToWorkWith->getRoughtnessMapChannel(), newNode, 9);
			}

			if (material->getMetalnessMap() == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, objToWorkWith->getMetalnessMapChannel(), newNode, 4);
			}
			else if (material->getMetalnessMap(1) == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, objToWorkWith->getMetalnessMapChannel(), newNode, 10);
			}

			if (material->getDisplacementMap() == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, objToWorkWith->getDisplacementMapChannel(), newNode, 5);
			}
			else if (material->getDisplacementMap(1) == material->textures[i])
			{
				materialNodeArea->tryToConnect(newTextureNode, objToWorkWith->getDisplacementMapChannel(), newNode, 11);
			}
		}
#else
		// ************** Drag&Drop **************
		texturesListTarget = DRAG_AND_DROP_MANAGER.addTarget(FE_TEXTURE, dragAndDropTexturesListCallback, reinterpret_cast<void**>(&objToWorkWith), "Drop to add texture");
		if (materialBindingtargets.size() == 0)
		{
			materialBindingInfo.resize(12);
			for (size_t i = 0; i < 12; i++)
			{
				materialBindingInfo[i].material = reinterpret_cast<void**>(&objToWorkWith);
				materialBindingInfo[i].textureBinding = i;

				materialBindingCallbackInfo* test = &materialBindingInfo[i];
				materialBindingtargets.push_back(DRAG_AND_DROP_MANAGER.addTarget(FE_TEXTURE, dragAndDropMaterialBindingsCallback, reinterpret_cast<void**>(&materialBindingInfo[i]), "Drop to assign texture"));
			}
		}
		// ************** Drag&Drop END **************
#endif // USE_NODES
	}
}

void editMaterialPopup::render()
{
	FEImGuiWindow::render();

	if (!isVisible())
		return;

	if (objToWorkWith == nullptr)
	{
		FEImGuiWindow::close();
		return;
	}

#ifdef USE_NODES
	materialNodeArea->setAreaPosition(ImVec2(0, 0));
	materialNodeArea->setAreaSize(ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 50));
	materialNodeArea->update();
	nodeAreaTarget->stickToItem();

	windowPosition = ImGui::GetWindowPos();

	if (ImGui::GetIO().MouseReleased[1])
		mousePositionWhenContextMenuWasOpened = ImGui::GetMousePos();
#else
	// lame callback
	if (tempContainer != RESOURCE_MANAGER.noTexture)
	{
		if (textureDestination == -1)
		{
			objToWorkWith->addTexture(tempContainer);
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
				objToWorkWith->setAlbedoMap(tempContainer, subMaterial);
				break;
			}
			case 1:
			{
				objToWorkWith->setNormalMap(tempContainer, subMaterial);
				break;
			}
			case 2:
			{
				objToWorkWith->setAOMap(tempContainer, 0, subMaterial);
				break;
			}
			case 3:
			{
				objToWorkWith->setRoughtnessMap(tempContainer, 0, subMaterial);
				break;
			}
			case 4:
			{
				objToWorkWith->setMetalnessMap(tempContainer, 0, subMaterial);
				break;
			}
			case 5:
			{
				objToWorkWith->setDisplacementMap(tempContainer, 0, subMaterial);
				break;
			}
			}
		}

		tempContainer = RESOURCE_MANAGER.noTexture;
		PREVIEW_MANAGER.createMaterialPreview(objToWorkWith->getObjectID());
		PROJECT_MANAGER.getCurrent()->modified = true;
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

		texturesListTarget->stickToCurrentWindow();

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
					objToWorkWith->removeTexture(textureFromListUnderMouse);
					PROJECT_MANAGER.getCurrent()->modified = true;
				}
			}

			ImGui::EndPopup();
		}

		ImGui::Columns(2, "TextureColumns", false);
		textureCount = 0;
		for (size_t i = 0; i < objToWorkWith->textures.size(); i++)
		{
			if (objToWorkWith->textures[i] == nullptr)
				continue;

			textureCount++;
			ImGui::PushID(objToWorkWith->textures[i]->getName().c_str());

			objToWorkWith->textures[i]->getName() == "noTexture" ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(RESOURCE_MANAGER.getTexture(objToWorkWith->textures[i]->getObjectID()));
			iconButton->render();

			if (iconButton->isHovered())
			{
				textureFromListUnderMouse = i;
			}

			if (iconButton->isHovered() && ImGui::IsMouseDragging(0) && !DRAG_AND_DROP_MANAGER.objectIsDraged())
				DRAG_AND_DROP_MANAGER.setObject(objToWorkWith->textures[i], objToWorkWith->textures[i], ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

			ImGui::Text(objToWorkWith->textures[i]->getName().c_str());
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
		objToWorkWith->getAlbedoMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getAlbedoMap());
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 0;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[0]->stickToItem();

		if (objToWorkWith->getAlbedoMap() != nullptr)
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
		objToWorkWith->getNormalMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getNormalMap());
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 1;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[1]->stickToItem();

		if (objToWorkWith->getNormalMap() != nullptr)
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
		objToWorkWith->getAOMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getAOMap());
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 2;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[2]->stickToItem();

		if (objToWorkWith->getAOMap() != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##ao", channels[objToWorkWith->getAOMapChannel()].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && objToWorkWith->getAOMap()->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[objToWorkWith->getAOMapChannel()] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						objToWorkWith->setAOMap(objToWorkWith->getAOMap(), i);

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
		objToWorkWith->getRoughtnessMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getRoughtnessMap());
		ImGui::SetCursorPosX(10);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 3;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[3]->stickToItem();

		if (objToWorkWith->getRoughtnessMap() != nullptr)
		{
			ImGui::SetCursorPosX(10);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##roughtness", channels[objToWorkWith->getRoughtnessMapChannel()].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && objToWorkWith->getRoughtnessMap()->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[objToWorkWith->getRoughtnessMapChannel()] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						objToWorkWith->setRoughtnessMap(objToWorkWith->getRoughtnessMap(), i);

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
		objToWorkWith->getMetalnessMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getMetalnessMap());
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 4;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[4]->stickToItem();

		if (objToWorkWith->getMetalnessMap() != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##metalness", channels[objToWorkWith->getMetalnessMapChannel()].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && objToWorkWith->getMetalnessMap()->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[objToWorkWith->getMetalnessMapChannel()] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						objToWorkWith->setMetalnessMap(objToWorkWith->getMetalnessMap(), i);

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
		objToWorkWith->getDisplacementMap() == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getDisplacementMap());
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 5;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[5]->stickToItem();

		if (objToWorkWith->getDisplacementMap() != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##displacement", channels[objToWorkWith->getDisplacementMapChannel()].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && objToWorkWith->getDisplacementMap()->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[objToWorkWith->getDisplacementMapChannel()] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						objToWorkWith->setDisplacementMap(objToWorkWith->getDisplacementMap(), i);

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
		objToWorkWith->getAlbedoMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getAlbedoMap(1));
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 6;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[6]->stickToItem();

		if (objToWorkWith->getAlbedoMap(1) != nullptr)
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
		objToWorkWith->getNormalMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getNormalMap(1));
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 7;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[7]->stickToItem();

		if (objToWorkWith->getNormalMap(1) != nullptr)
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
		objToWorkWith->getAOMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getAOMap(1));
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 8;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[8]->stickToItem();

		if (objToWorkWith->getAOMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##aoSubmaterial", channels[objToWorkWith->getAOMapChannel(1)].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && objToWorkWith->getAOMap(1)->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[objToWorkWith->getAOMapChannel(1)] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						objToWorkWith->setAOMap(objToWorkWith->getAOMap(1), i, 1);

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
		objToWorkWith->getRoughtnessMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getRoughtnessMap(1));
		ImGui::SetCursorPosX(10);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 9;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[9]->stickToItem();

		if (objToWorkWith->getRoughtnessMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##roughtnessSubmaterial", channels[objToWorkWith->getRoughtnessMapChannel(1)].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && objToWorkWith->getRoughtnessMap(1)->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[objToWorkWith->getRoughtnessMapChannel(1)] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						objToWorkWith->setRoughtnessMap(objToWorkWith->getRoughtnessMap(1), i, 1);

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
		objToWorkWith->getMetalnessMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getMetalnessMap(1));
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 10;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[10]->stickToItem();

		if (objToWorkWith->getMetalnessMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##metalnessSubmaterial", channels[objToWorkWith->getMetalnessMapChannel(1)].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && objToWorkWith->getMetalnessMap(1)->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[objToWorkWith->getMetalnessMapChannel(1)] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						objToWorkWith->setMetalnessMap(objToWorkWith->getMetalnessMap(1), i, 1);

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
		objToWorkWith->getDisplacementMap(1) == nullptr ? iconButton->setTexture(RESOURCE_MANAGER.noTexture) : iconButton->setTexture(objToWorkWith->getDisplacementMap(1));
		ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
		iconButton->render();
		if (iconButton->getWasClicked())
		{
			textureDestination = 11;
			selectTexturePopUp::getInstance().showWithCustomList(&tempContainer, objToWorkWith->textures);
		}

		if (materialBindingtargets.size() > 0)
			materialBindingtargets[11]->stickToItem();

		if (objToWorkWith->getDisplacementMap(1) != nullptr)
		{
			ImGui::SetCursorPosX(10 + ImGui::GetWindowContentRegionWidth() / 3.0f * 2.0f);
			ImGui::SetNextItemWidth(85);
			if (ImGui::BeginCombo("Channel##displacementSubmaterial", channels[objToWorkWith->getDisplacementMapChannel(1)].c_str(), ImGuiWindowFlags_None))
			{
				for (size_t i = 0; i < channels.size(); i++)
				{
					// if texture was compresed with out alpha channel
					if (i == 3 && objToWorkWith->getDisplacementMap(1)->getInternalFormat() == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
						continue;

					bool is_selected = (channels[objToWorkWith->getDisplacementMapChannel(1)] == channels[i]);
					if (ImGui::Selectable(channels[i].c_str(), is_selected))
						objToWorkWith->setDisplacementMap(objToWorkWith->getDisplacementMap(1), i, 1);

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

		FEShaderParam* debugFlag = objToWorkWith->getParameter("debugFlag");
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
		float normalMapIntensity = objToWorkWith->getNormalMapIntensity();
		ImGui::DragFloat("##Normal map intensity", &normalMapIntensity, 0.01f, 0.0f, 1.0f);
		objToWorkWith->setNormalMapIntensity(normalMapIntensity);

		// ************* AO *************
		if (objToWorkWith->getAOMap() == nullptr)
		{
			ImGui::Text("Ambient occlusion intensity:");
			ImGui::SetNextItemWidth(fieldWidth);
			float ambientOcclusionIntensity = objToWorkWith->getAmbientOcclusionIntensity();
			ImGui::DragFloat("##Ambient occlusion intensity", &ambientOcclusionIntensity, 0.01f, 0.0f, 10.0f);
			objToWorkWith->setAmbientOcclusionIntensity(ambientOcclusionIntensity);
		}
		else
		{
			ImGui::Text("Ambient occlusion map intensity:");
			ImGui::SetNextItemWidth(fieldWidth);
			float AOMapIntensity = objToWorkWith->getAmbientOcclusionMapIntensity();
			ImGui::DragFloat("##Ambient occlusion map intensity", &AOMapIntensity, 0.01f, 0.0f, 10.0f);
			objToWorkWith->setAmbientOcclusionMapIntensity(AOMapIntensity);
		}

		// ************* Roughtness *************
		if (objToWorkWith->getRoughtnessMap() == nullptr)
		{
			ImGui::Text("Roughtness:");
			ImGui::SetNextItemWidth(fieldWidth);
			float roughtness = objToWorkWith->getRoughtness();
			ImGui::DragFloat("##Roughtness", &roughtness, 0.01f, 0.0f, 1.0f);
			objToWorkWith->setRoughtness(roughtness);
		}
		else
		{
			ImGui::Text("Roughtness map intensity:");
			ImGui::SetNextItemWidth(fieldWidth);
			float roughtness = objToWorkWith->getRoughtnessMapIntensity();
			ImGui::DragFloat("##Roughtness map intensity", &roughtness, 0.01f, 0.0f, 10.0f);
			objToWorkWith->setRoughtnessMapIntensity(roughtness);
		}

		// ************* Metalness *************
		if (objToWorkWith->getMetalnessMap() == nullptr)
		{
			ImGui::Text("Metalness:");
			ImGui::SetNextItemWidth(fieldWidth);
			float metalness = objToWorkWith->getMetalness();
			ImGui::DragFloat("##Metalness", &metalness, 0.01f, 0.0f, 1.0f);
			objToWorkWith->setMetalness(metalness);
		}
		else
		{
			ImGui::Text("Metalness map intensity:");
			ImGui::SetNextItemWidth(fieldWidth);
			float metalness = objToWorkWith->getMetalnessMapIntensity();
			ImGui::DragFloat("##Metalness map intensity", &metalness, 0.01f, 0.0f, 10.0f);
			objToWorkWith->setMetalnessMapIntensity(metalness);
		}

		ImGui::PopStyleColor();
		ImGui::EndChild();
	}
#endif // USE_NODES

	cancelButton->render();
	if (cancelButton->getWasClicked())
	{
		FEImGuiWindow::close();
		return;
	}

	FEImGuiWindow::onRenderEnd();
	selectTexturePopUp::getInstance().render();
}

void editMaterialPopup::close()
{
	FEImGuiWindow::close();
}

#ifdef USE_NODES
bool editMaterialPopup::dragAndDropnodeAreaTargetCallback(FEObject* object, void** callbackInfo)
{
	if (objToWorkWith->isTextureInList(RESOURCE_MANAGER.getTexture(object->getObjectID())))
		return false;

	if (objToWorkWith->getUsedTexturesCount() == FE_MAX_TEXTURES_PER_MATERIAL)
		return false;

	FEEditorTextureSourceNode* newNode = new FEEditorTextureSourceNode(RESOURCE_MANAGER.getTexture(object->getObjectID()));

	ImVec2 positionOnCanvas;
	positionOnCanvas.x = ImGui::GetMousePos().x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
	positionOnCanvas.y = ImGui::GetMousePos().y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;

	newNode->setPosition(positionOnCanvas);
	materialNodeArea->addNode(newNode);
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
void editMaterialPopup::nodeSystemMainContextMenu()
{
	if (ImGui::BeginMenu("Add"))
	{
		if (ImGui::MenuItem("Texture node"))
		{
			textureForNewNode = RESOURCE_MANAGER.noTexture;
			selectTexturePopUp::getInstance().show(&textureForNewNode, textureNodeCreationCallback, reinterpret_cast<void*>(&textureForNewNode));
		}

		if (ImGui::MenuItem("Float node"))
		{
			FEEditorFloatSourceNode* newNode = new FEEditorFloatSourceNode();

			ImVec2 positionOnCanvas;
			positionOnCanvas.x = mousePositionWhenContextMenuWasOpened.x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
			positionOnCanvas.y = mousePositionWhenContextMenuWasOpened.y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;

			newNode->setPosition(positionOnCanvas);
			materialNodeArea->addNode(newNode);
		}

		ImGui::EndMenu();
	}
}

void editMaterialPopup::textureNodeCallback(FEEditorNode* node, FE_EDITOR_NODE_EVENT eventWithNode)
{
	if (node == nullptr)
		return;

	if (node->getType() != "FEEditorTextureSourceNode")
		return;

	FEEditorTextureSourceNode* currentNode = reinterpret_cast<FEEditorTextureSourceNode*>(node);
	if (objToWorkWith->isTextureInList(currentNode->getTexture()))
	{
		objToWorkWith->removeTexture(currentNode->getTexture());
	}
}

void editMaterialPopup::textureNodeCreationCallback(void* texture)
{
	if (texture != nullptr && texture != RESOURCE_MANAGER.noTexture)
	{
		if (!objToWorkWith->isTextureInList(*reinterpret_cast<FETexture**>(texture)))
		{
			if (objToWorkWith->addTexture(*reinterpret_cast<FETexture**>(texture)))
			{
				FEEditorTextureSourceNode* newNode = new FEEditorTextureSourceNode(*reinterpret_cast<FETexture**>(texture));

				ImVec2 positionOnCanvas;
				positionOnCanvas.x = mousePositionWhenContextMenuWasOpened.x - (windowPosition.x + nodeGridRelativePosition.x) - newNode->getSize().x / 2.0f;
				positionOnCanvas.y = mousePositionWhenContextMenuWasOpened.y - (windowPosition.y + nodeGridRelativePosition.y) - newNode->getSize().y / 2.0f;

				newNode->setPosition(positionOnCanvas);
				materialNodeArea->addNode(newNode);
			}
		}
	}
}
#endif // USE_NODES
