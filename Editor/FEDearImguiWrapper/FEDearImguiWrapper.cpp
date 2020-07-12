#include "FEDearImguiWrapper.h"

WindowsManager* WindowsManager::_instance = nullptr;

ImGuiModalPopup::ImGuiModalPopup()
{
	popupCaption = "";
	shouldOpen = false;
	opened = false;
	WindowsManager::getInstance().registerPopup(this);
}

void ImGuiModalPopup::show()
{
	shouldOpen = true;
	opened = true;
}

void ImGuiModalPopup::render()
{
	if (shouldOpen)
	{
		ImGui::OpenPopup(popupCaption);
		shouldOpen = false;
	}
}

bool ImGuiModalPopup::isOpened()
{
	return opened;
}

void ImGuiModalPopup::close()
{
	opened = false;
	ImGui::CloseCurrentPopup();
}

ImGuiButton::ImGuiButton(std::string caption)
{
	if (caption.size() + 1 > BUTTON_CAPTION_SIZE)
		caption.erase(caption.begin() + BUTTON_CAPTION_SIZE - 1, caption.end() - (BUTTON_CAPTION_SIZE + 1));
	
	strcpy_s(this->caption, caption.size() + 1, caption.c_str());
}

ImVec2 ImGuiButton::getPosition()
{
	return position;
}

void ImGuiButton::setPosition(ImVec2 newPosition)
{
	position = newPosition;
}

ImVec2 ImGuiButton::getSize()
{
	return size;
}

void ImGuiButton::setSize(ImVec2 newSize)
{
	size = newSize;
}

ImVec4 ImGuiButton::getDefaultColor()
{
	return defaultColor;
}

void ImGuiButton::setDefaultColor(ImVec4 newDefaultColor)
{
	defaultColor = newDefaultColor;
}

ImVec4 ImGuiButton::getHoveredColor()
{
	return hoveredColor;
}

void ImGuiButton::setHoveredColor(ImVec4 newHoveredColor)
{
	hoveredColor = newHoveredColor;
}

ImVec4 ImGuiButton::getActiveColor()
{
	return activeColor;
}

void ImGuiButton::setActiveColor(ImVec4 newActiveColor)
{
	activeColor = newActiveColor;
}

void ImGuiButton::renderBegin()
{
	wasClicked = false;

	ImGui::PushStyleColor(ImGuiCol_Button, defaultColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);

	if (position.x != -1.0f)
		ImGui::SetCursorPosX(position.x);
	
	if (position.y != -1.0f)
		ImGui::SetCursorPosY(position.y);
	
	hovered = false;
	
	ImGui::Button(caption, size);

	if (ImGui::IsItemHovered())
		hovered = true;

	if (ImGui::IsMouseClicked(0) && hovered)
		wasClicked = true;
}

bool ImGuiButton::getWasClicked()
{
	return wasClicked;
}

void ImGuiButton::renderEnd()
{
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void ImGuiButton::render()
{
	renderBegin();
	renderEnd();
}

ImGuiImageButton::ImGuiImageButton(FETexture* texture)
{
	this->texture = texture;
}

ImVec2 ImGuiImageButton::getPosition()
{
	return position;
}

void ImGuiImageButton::setPosition(ImVec2 newPosition)
{
	position = newPosition;
}

ImVec2 ImGuiImageButton::getSize()
{
	return size;
}

void ImGuiImageButton::setSize(ImVec2 newSize)
{
	size = newSize;
}

ImVec4 ImGuiImageButton::getDefaultColor()
{
	return defaultColor;
}

void ImGuiImageButton::setDefaultColor(ImVec4 newDefaultColor)
{
	defaultColor = newDefaultColor;
}

ImVec4 ImGuiImageButton::getHoveredColor()
{
	return hoveredColor;
}

void ImGuiImageButton::setHoveredColor(ImVec4 newHoveredColor)
{
	hoveredColor = newHoveredColor;
}

ImVec4 ImGuiImageButton::getActiveColor()
{
	return activeColor;
}

void ImGuiImageButton::setActiveColor(ImVec4 newActiveColor)
{
	activeColor = newActiveColor;
}

ImVec2 ImGuiImageButton::getUV0()
{
	return uv0;
}

void ImGuiImageButton::setUV0(ImVec2 newUV0)
{
	uv0 = newUV0;
}

ImVec2 ImGuiImageButton::getUV1()
{
	return uv1;
}

void ImGuiImageButton::setUV1(ImVec2 newUV1)
{
	uv1 = newUV1;
}

int ImGuiImageButton::getFramePadding()
{
	return framePadding;
}

void ImGuiImageButton::setFramePadding(int newFramePadding)
{
	if (newFramePadding < 0)
		newFramePadding = 0;
	framePadding = newFramePadding;
}

ImVec4 ImGuiImageButton::getBackgroundColor()
{
	return backgroundColor;
}

void ImGuiImageButton::setBackgroundColor(ImVec4 newBackgroundColor)
{
	backgroundColor = newBackgroundColor;
}

ImVec4 ImGuiImageButton::getTintColor()
{
	return tintColor;
}

void ImGuiImageButton::setTintColor(ImVec4 newTintColor)
{
	tintColor = newTintColor;
}

FETexture* ImGuiImageButton::getTexture()
{
	return texture;
}

void ImGuiImageButton::setTexture(FETexture* newTexture)
{
	texture = newTexture;
}

bool ImGuiImageButton::isHovered()
{
	return hovered;
}

void ImGuiImageButton::renderBegin()
{
	wasClicked = false;

	ImGui::PushStyleColor(ImGuiCol_Button, defaultColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoveredColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);

	if (position.x != -1.0f)
		ImGui::SetCursorPosX(position.x);

	if (position.y != -1.0f)
		ImGui::SetCursorPosY(position.y);

	hovered = false;

	ImGui::ImageButton((void*)(intptr_t)texture->getTextureID(), size, uv0, uv1, framePadding, backgroundColor, tintColor);

	if (ImGui::IsItemHovered())
		hovered = true;

	if (ImGui::IsMouseClicked(0) && hovered)
		wasClicked = true;
}

bool ImGuiImageButton::getWasClicked()
{
	return wasClicked;
}

void ImGuiImageButton::renderEnd()
{
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void ImGuiImageButton::render()
{
	renderBegin();
	renderEnd();
}

ImGuiWindow::ImGuiWindow()
{
	position = ImVec2(0.0f, 0.0f);
	size = ImVec2(100.0f, 100.0f);
	visible = false;
	WindowsManager::getInstance().registerWindow(this);
}

ImGuiWindow::~ImGuiWindow()
{
}

void ImGuiWindow::show()
{
	visible = true;
	wasClosedLastFrame = true;
}

void ImGuiWindow::render()
{
	if (visible)
	{
		if (wasClosedLastFrame)
		{
			ImGui::SetNextWindowPos(position);
		}

		if ((flags & ImGuiWindowFlags_NoResize) == ImGuiWindowFlags_NoResize)
		{
			ImGui::SetNextWindowSize(size);
		}
		else if (wasClosedLastFrame)
		{
			ImGui::SetNextWindowSize(size);
		}

		wasClosedLastFrame = false;
		ImGui::Begin(caption, nullptr, flags);
	}
}

void ImGuiWindow::onRenderEnd()
{
	if (visible)
		ImGui::End();
}

bool ImGuiWindow::isVisible()
{
	return visible;
}

void ImGuiWindow::close()
{
	if (visible)
	{
		visible = false;
		ImGui::End();
	}
}

WindowsManager::WindowsManager()
{
}

void WindowsManager::registerWindow(ImGuiWindow* window)
{
	windows.push_back(window);
}

void WindowsManager::registerPopup(ImGuiModalPopup* popup)
{
	popUps.push_back(popup);
}

void WindowsManager::closeAllPopups()
{
	for (size_t i = 0; i < popUps.size(); i++)
	{
		//popUps[i]->visible = false;
		popUps[i]->close();
	}
}

void WindowsManager::closeAllWindows()
{
	for (size_t i = 0; i < windows.size(); i++)
	{
		windows[i]->visible = false;
		//windows[i]->close();
	}
}