#pragma once

#include "prefabEditorWindow.h"

class debugTextureViewWindow : public FEImGuiWindow
{
	ImGuiButton* closeButton = nullptr;
	std::function<FETexture* ()> textureToView;
public:
	debugTextureViewWindow(std::function<FETexture* ()> func);
	~debugTextureViewWindow();

	void render() override;
};