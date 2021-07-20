#include "shaderEditorWindow.h"

shaderDebugWindow* shaderDebugWindow::_instance = nullptr;

shaderDebugWindow::shaderDebugWindow()
{
	flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
	editor.SetShowWhitespaces(false);
	editor.SetReadOnly(true);
	size = ImVec2(800, 600);
	closeButton = new ImGuiButton("Close");
	updateButton = new ImGuiButton("Update");
	editor.SetPalette(TextEditor::GetLightPalette());
	editor.SetColorizerEnable(false);
}

shaderDebugWindow::~shaderDebugWindow()
{
	if (closeButton != nullptr)
		delete closeButton;

	if (updateButton != nullptr)
		delete updateButton;

	shaderToWorkWith = nullptr;
}

void shaderDebugWindow::show(FEShader* shader, std::string caption)
{
	updateNeeded = true;
	shaderToWorkWith = shader;

	selectedDebugData = "";
	occurrenceList.clear();
	data = nullptr;
	dataDump.clear();

	if (caption.size() == 0)
		caption = "Text view";

	strcpy_s(this->caption, caption.size() + 1, caption.c_str());
	FEImGuiWindow::show();
}

void shaderDebugWindow::render()
{
	FEImGuiWindow::render();

	if (!isVisible())
		return;

	if (updateNeeded)
	{
		data = shaderToWorkWith->getDebugData();
		if (data->size() > 0)
		{
			dataDump.clear();
			occurrenceList.clear();
			updateNeeded = false;
			for (size_t i = 0; i < data->size(); i++)
			{
				std::string debugItemsCount = std::to_string(data->operator[](i)[0]);
				for (size_t j = 0; j < debugItemsCount.size(); j++)
				{
					if (debugItemsCount[j] == '.')
					{
						debugItemsCount.erase(debugItemsCount.begin() + j, debugItemsCount.end());
						break;
					}
				}

				occurrenceList.push_back("occurrence with " + debugItemsCount + " debug items");
				dataDump.push_back(data->operator[](i));
			}

			selectedDebugData = occurrenceList[0];

			selectedDebugData = occurrenceList[0].c_str();
			std::string text = "";
			std::vector<std::string> debugVariables = shaderToWorkWith->getDebugVariables();
			for (size_t i = 1; i < dataDump[0].size(); i++)
			{
				float t = dataDump[0][i];
				text += debugVariables[(i - 1) % debugVariables.size()];
				text += " : ";
				text += std::to_string(t);
				if (i < dataDump[0].size() - 1)
					text += "\n";
			}
			editor.SetText(text);
		}
	}

	if (ImGui::BeginCombo("Shader occurrence", selectedDebugData.c_str(), ImGuiWindowFlags_None))
	{
		for (size_t n = 0; n < occurrenceList.size(); n++)
		{
			ImGui::PushID(n);
			bool is_selected = (selectedDebugData == occurrenceList[n]);
			if (ImGui::Selectable(occurrenceList[n].c_str(), is_selected))
			{
				selectedDebugData = occurrenceList[n].c_str();
				std::string text = "";
				std::vector<std::string> debugVariables = shaderToWorkWith->getDebugVariables();
				for (size_t i = 1; i < dataDump[n].size(); i++)
				{
					float t = dataDump[n][i];
					text += debugVariables[(i - 1) % debugVariables.size()];
					text += " : ";
					text += std::to_string(t);
					if (i < dataDump[n].size() - 1)
						text += "\n";
				}
				editor.SetText(text);
			}

			if (is_selected)
				ImGui::SetItemDefaultFocus();

			ImGui::PopID();
		}
		ImGui::EndCombo();
	}

	closeButton->render();
	if (closeButton->getWasClicked())
	{
		FEImGuiWindow::close();
	}

	updateButton->render();
	if (updateButton->getWasClicked())
	{
		updateNeeded = true;
	}

	editor.Render("TextEditor");
	FEImGuiWindow::onRenderEnd();
}

shaderEditorWindow* shaderEditorWindow::_instance = nullptr;

shaderEditorWindow::shaderEditorWindow()
{
	flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
	currentEditor = &vertexShaderEditor;
	vertexShaderEditor.SetShowWhitespaces(false);
	vertexShaderEditor.SetPalette(TextEditor::GetLightPalette());
	tessControlShaderEditor.SetShowWhitespaces(false);
	tessControlShaderEditor.SetPalette(TextEditor::GetLightPalette());
	tessEvalShaderEditor.SetShowWhitespaces(false);
	tessEvalShaderEditor.SetPalette(TextEditor::GetLightPalette());
	geometryShaderEditor.SetShowWhitespaces(false);
	geometryShaderEditor.SetPalette(TextEditor::GetLightPalette());
	fragmentShaderEditor.SetShowWhitespaces(false);
	fragmentShaderEditor.SetPalette(TextEditor::GetLightPalette());
	computeShaderEditor.SetShowWhitespaces(false);
	computeShaderEditor.SetPalette(TextEditor::GetLightPalette());

	size = ImVec2(800, 600);
	compileButton = new ImGuiButton("Compile");
}

shaderEditorWindow::~shaderEditorWindow()
{
	if (compileButton != nullptr)
		delete compileButton;
}

void shaderEditorWindow::show(FEShader* shader)
{
	shaderToEdit = shader;

	vertexShaderEditor.SetText(shaderToEdit->getVertexShaderText());
	fragmentShaderEditor.SetText(shaderToEdit->getFragmentShaderText());
	currentEditor = &vertexShaderEditor;

	if (shaderToEdit->getTessControlShaderText() != nullptr)
	{
		tessControlShaderEditor.SetText(shaderToEdit->getTessControlShaderText());
		tessControlShaderUsed = true;
	}
	else
	{
		tessControlShaderEditor.SetText("");
		tessControlShaderUsed = false;
	}

	if (shaderToEdit->getTessEvalShaderText() != nullptr)
	{
		tessEvalShaderEditor.SetText(shaderToEdit->getTessEvalShaderText());
		tessEvalShaderUsed = true;
	}
	else
	{
		tessEvalShaderEditor.SetText("");
		tessEvalShaderUsed = false;
	}

	if (shaderToEdit->getGeometryShaderText() != nullptr)
	{
		geometryShaderEditor.SetText(shaderToEdit->getGeometryShaderText());
		geometryShaderUsed = true;
	}
	else
	{
		geometryShaderEditor.SetText("");
		geometryShaderUsed = false;
	}

	std::string tempCaption = "Edit shader: ";
	tempCaption += shaderToEdit->getName();
	strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());
	FEImGuiWindow::show();
}

void shaderEditorWindow::render()
{
	FEImGuiWindow::render();

	if (!isVisible())
		return;

	compileButton->render();
	if (compileButton->getWasClicked())
	{
		if (dummyShader != nullptr)
			delete dummyShader;

		dummyShader = new FEShader("dummyShader", vertexShaderEditor.GetText().c_str(), fragmentShaderEditor.GetText().c_str(),
			tessControlShaderUsed ? tessControlShaderEditor.GetText().c_str() : nullptr,
			tessEvalShaderUsed ? tessEvalShaderEditor.GetText().c_str() : nullptr,
			geometryShaderUsed ? geometryShaderEditor.GetText().c_str() : nullptr,
			computeShaderUsed ? computeShaderEditor.GetText().c_str() : nullptr,
			true);

		std::vector<std::string> oldParameters = shaderToEdit->getParameterList();
		for (size_t i = 0; i < oldParameters.size(); i++)
		{
			dummyShader->addParameter(*shaderToEdit->getParameter(oldParameters[i]));
		}

		std::string errors = dummyShader->getCompilationErrors();
		if (errors.size() != 0)
		{
			justTextWindowObj.show(errors, "Shader compilation error!");
		}
		else
		{
			FEShader* reCompiledShader = new FEShader(shaderToEdit->getName(), vertexShaderEditor.GetText().c_str(), fragmentShaderEditor.GetText().c_str(),
				tessControlShaderUsed ? tessControlShaderEditor.GetText().c_str() : nullptr,
				tessEvalShaderUsed ? tessEvalShaderEditor.GetText().c_str() : nullptr,
				geometryShaderUsed ? geometryShaderEditor.GetText().c_str() : nullptr,
				computeShaderUsed ? computeShaderEditor.GetText().c_str() : nullptr);

			std::vector<std::string> oldParameters = shaderToEdit->getParameterList();
			for (size_t i = 0; i < oldParameters.size(); i++)
			{
				reCompiledShader->addParameter(*shaderToEdit->getParameter(oldParameters[i]));
			}

			RESOURCE_MANAGER.replaceShader(shaderToEdit->getObjectID(), reCompiledShader);

			if (shaderToEdit->isDebugRequest())
			{
				//shaderToEdit->updateDebugData();
				shaderDebugWindow::getInstance().show(shaderToEdit, "Shader debug info");
			}
		}
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Edit"))
		{
			bool ro = currentEditor->IsReadOnly();
			if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
				currentEditor->SetReadOnly(ro);
			ImGui::Separator();

			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && currentEditor->CanUndo()))
				currentEditor->Undo();
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && currentEditor->CanRedo()))
				currentEditor->Redo();

			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, currentEditor->HasSelection()))
				currentEditor->Copy();
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && currentEditor->HasSelection()))
				currentEditor->Cut();
			if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && currentEditor->HasSelection()))
				currentEditor->Delete();
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
				currentEditor->Paste();

			ImGui::Separator();

			if (ImGui::MenuItem("Select all", nullptr, nullptr))
				currentEditor->SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(currentEditor->GetTotalLines(), 0));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
				currentEditor->SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem("Light palette"))
				currentEditor->SetPalette(TextEditor::GetLightPalette());
			if (ImGui::MenuItem("Retro blue palette"))
				currentEditor->SetPalette(TextEditor::GetRetroBluePalette());
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	auto cpos = currentEditor->GetCursorPosition();
	ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, currentEditor->GetTotalLines(),
		currentEditor->IsOverwrite() ? "Ovr" : "Ins",
		currentEditor->CanUndo() ? "*" : " ",
		currentEditor->GetLanguageDefinition().mName.c_str(), "none");

	ImGui::PushStyleColor(ImGuiCol_TabActive, (ImVec4)ImColor::ImColor(0.4f, 0.9f, 0.4f, 1.0f));
	if (ImGui::BeginTabBar("##Shaders Editors", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Vertex Shader"))
		{
			activeTab = 0;
			currentEditor = &vertexShaderEditor;
			ImGui::EndTabItem();
		}

		if (tessControlShaderUsed)
		{
			if (ImGui::BeginTabItem("Tessalation Control Shader"))
			{
				activeTab = 1;
				currentEditor = &tessControlShaderEditor;
				ImGui::EndTabItem();
			}
		}

		if (tessEvalShaderUsed)
		{
			if (ImGui::BeginTabItem("Tessalation Evaluation Shader"))
			{
				activeTab = 2;
				currentEditor = &tessEvalShaderEditor;
				ImGui::EndTabItem();
			}
		}

		if (geometryShaderUsed)
		{
			if (ImGui::BeginTabItem("Geometry Shader"))
			{
				activeTab = 3;
				currentEditor = &geometryShaderEditor;
				ImGui::EndTabItem();
			}
		}

		if (ImGui::BeginTabItem("Fragment Shader"))
		{
			activeTab = 4;
			currentEditor = &fragmentShaderEditor;
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	ImGui::PopStyleColor();

	currentEditor->Render("Editor");

	justTextWindowObj.render();
	shaderDebugWindow::getInstance().render();
	FEImGuiWindow::onRenderEnd();
}

void shaderEditorWindow::replaceShader(FEShader* oldShader, FEShader* newShader)
{
	std::vector<std::string> materialList = RESOURCE_MANAGER.getMaterialList();
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FEMaterial* tempMaterial = RESOURCE_MANAGER.getMaterial(materialList[i]);
		if (tempMaterial->shader->getNameHash() == oldShader->getNameHash())
		{
			tempMaterial->shader = newShader;
		}
	}

	materialList = RESOURCE_MANAGER.getStandardMaterialList();
	for (size_t i = 0; i < materialList.size(); i++)
	{
		FEMaterial* tempMaterial = RESOURCE_MANAGER.getMaterial(materialList[i]);
		if (tempMaterial->shader->getNameHash() == oldShader->getNameHash())
		{
			tempMaterial->shader = newShader;
		}
	}

	std::vector<std::string> terrainList = SCENE.getTerrainList();
	for (size_t i = 0; i < terrainList.size(); i++)
	{
		FETerrain* tempTerrain = SCENE.getTerrain(terrainList[i]);
		if (tempTerrain->shader->getNameHash() == oldShader->getNameHash())
		{
			tempTerrain->shader = newShader;
		}
	}
}