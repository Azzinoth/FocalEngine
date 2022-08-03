#include "debugTextureViewWindow.h"

debugTextureViewWindow::debugTextureViewWindow(std::function<FETexture* ()> Func)
{
	TextureToView = Func;

	std::string tempCaption = TextureToView()->GetName();
	strcpy_s(Caption, tempCaption.size() + 1, tempCaption.c_str());

	Size = ImVec2(800, 800);
	Position = ImVec2(FEngine::getInstance().GetWindowWidth() / 2 - Size.x / 2, FEngine::getInstance().GetWindowHeight() / 2 - Size.y / 2);

	Flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

	CloseButton = new ImGuiButton("Close");
	CloseButton->SetSize(ImVec2(140, 24));
	CloseButton->SetPosition(ImVec2(800.0 / 2.0 - 140.0 / 2.0, 800.0 - 35.0));
}

debugTextureViewWindow::~debugTextureViewWindow()
{
	if (CloseButton != nullptr)
		delete CloseButton;
}

void debugTextureViewWindow::Render()
{
	FEImGuiWindow::Render();

	if (!IsVisible())
		return;

	ImGui::Image((void*)(intptr_t)TextureToView()->GetTextureID(), ImVec2(Size.x - 30.0f, Size.y - 85.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	CloseButton->SetPosition(ImVec2(Size.x / 2.0f - 140.0f / 2.0f, Size.y - 35.0f));
	CloseButton->Render();
	if (CloseButton->IsClicked())
		Close();

	FEImGuiWindow::OnRenderEnd();
}