#pragma once

#include "../Editor/FEEditorSubWindows/gyzmosSettingsWindow.h"

class loadTexturePopUp : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(loadTexturePopUp)

	std::string filePath;
public:
	SINGLETON_PUBLIC_PART(loadTexturePopUp)

	void show(std::string FilePath);
	void render() override;
};