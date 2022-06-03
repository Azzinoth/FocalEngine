#include "debugTextureViewWindow.h"

debugTextureViewWindow::debugTextureViewWindow(std::function<FETexture* ()> func)
{
	textureToView = func;

	std::string tempCaption = textureToView()->getName();
	strcpy_s(caption, tempCaption.size() + 1, tempCaption.c_str());

	size = ImVec2(800, 800);
	position = ImVec2(FEngine::getInstance().getWindowWidth() / 2 - size.x / 2, FEngine::getInstance().getWindowHeight() / 2 - size.y / 2);

	flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

	closeButton = new ImGuiButton("Close");
	closeButton->setSize(ImVec2(140, 24));
	closeButton->setPosition(ImVec2(800.0 / 2.0 - 140.0 / 2.0, 800.0 - 35.0));
}

debugTextureViewWindow::~debugTextureViewWindow()
{
	if (closeButton != nullptr)
		delete closeButton;
}

void debugTextureViewWindow::render()
{
	FEImGuiWindow::render();

	if (!isVisible())
		return;

	ImGui::Image((void*)(intptr_t)textureToView()->getTextureID(), ImVec2(size.x - 30.0f, size.y - 85.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	closeButton->setPosition(ImVec2(size.x / 2.0f - 140.0f / 2.0f, size.y - 35.0f));
	closeButton->render();
	if (closeButton->getWasClicked())
		close();

	FEImGuiWindow::onRenderEnd();
}