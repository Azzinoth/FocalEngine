#pragma once

#include "prefabEditorWindow.h"

class debugTextureViewWindow : public FEImGuiWindow
{
	ImGuiButton* CloseButton = nullptr;
	std::function<FETexture* ()> TextureToView;
public:
	debugTextureViewWindow(std::function<FETexture* ()> Func);
	~debugTextureViewWindow();

	void Render() override;
};