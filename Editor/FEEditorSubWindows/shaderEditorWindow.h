#pragma once

#include "editPopups.h"
#pragma warning (disable: 4724)

class shaderDebugWindow : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(shaderDebugWindow)

	TextEditor editor;
	ImGuiButton* closeButton = nullptr;
	ImGuiButton* updateButton = nullptr;
	FEShader* shaderToWorkWith = nullptr;
	bool updateNeeded = true;
	std::string selectedDebugData = "";
	std::vector<std::string> occurrenceList;
	std::vector<std::vector<float>>* data = nullptr;
	std::vector<std::vector<float>> dataDump;
public:
	SINGLETON_PUBLIC_PART(shaderDebugWindow)

	void show(FEShader* shader, std::string caption);
	void render() override;
};

class shaderEditorWindow : public FEImGuiWindow
{
	SINGLETON_PRIVATE_PART(shaderEditorWindow)

	FEShader* shaderToEdit = nullptr;
	FEShader* dummyShader = nullptr;
	TextEditor* currentEditor = nullptr;
	TextEditor vertexShaderEditor;
	TextEditor tessControlShaderEditor;
	TextEditor tessEvalShaderEditor;
	TextEditor geometryShaderEditor;
	TextEditor fragmentShaderEditor;
	TextEditor computeShaderEditor;

	bool vertexShaderUsed = false;
	bool fragmentShaderUsed = false;
	bool tessControlShaderUsed = false;
	bool tessEvalShaderUsed = false;
	bool geometryShaderUsed = false;
	bool computeShaderUsed = false;

	ImGuiButton* compileButton = nullptr;
	ImGuiButton* closeButton = nullptr;
	int activeTab = 0;

	void replaceShader(FEShader* oldShader, FEShader* newShader);
public:
	SINGLETON_PUBLIC_PART(shaderEditorWindow)

	void show(FEShader* shader);

	void render() override;
};