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
		ImGui::OpenPopup(popupCaption.c_str());
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

void ImGuiButton::setCaption(std::string newCaption)
{
	if (newCaption.size() + 1 > BUTTON_CAPTION_SIZE)
		newCaption.erase(newCaption.begin() + BUTTON_CAPTION_SIZE - 1, newCaption.end() - (BUTTON_CAPTION_SIZE + 1));

	strcpy_s(this->caption, newCaption.size() + 1, newCaption.c_str());
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

	// flag important for drag and drop functionality
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
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

	// flag important for drag and drop functionality
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
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

FEImGuiWindow::FEImGuiWindow()
{
	position = ImVec2(0.0f, 0.0f);
	size = ImVec2(100.0f, 100.0f);
	visible = false;
	WindowsManager::getInstance().registerWindow(this);
}

FEImGuiWindow::~FEImGuiWindow()
{
}

void FEImGuiWindow::show()
{
	visible = true;
	wasClosedLastFrame = true;
}

bool FEImGuiWindow::isMouseHovered()
{
	if (window == nullptr)
		return false;

	if (ImGui::GetCurrentContext()->HoveredWindow != nullptr &&
		ImGui::GetCurrentContext()->HoveredWindow == window)
		return true;

	return false;
}

void FEImGuiWindow::render()
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
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		ImGui::Begin(caption, nullptr, flags);
		window = ImGui::GetCurrentWindow();
	}

	/*if (!visible && lastFrameVisible != visible)
	{
		ImGui::End();
	}

	lastFrameVisible = visible;*/
}

void FEImGuiWindow::onRenderEnd()
{
	if (visible)
	{
		ImGui::PopStyleVar();
		ImGui::End();
	}
}

bool FEImGuiWindow::isVisible()
{
	return visible;
}

void FEImGuiWindow::close()
{
	if (visible)
	{
		visible = false;
		ImGui::PopStyleVar();
		ImGui::End();
	}
}

WindowsManager::WindowsManager()
{
}

void WindowsManager::registerWindow(FEImGuiWindow* window)
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

void showToolTip(const char* text)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(text);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void setSelectedStyle(ImGuiImageButton* button)
{
	button->setDefaultColor(selectedStyle);
	button->setHoveredColor(selectedStyle);
	button->setActiveColor(selectedStyle);
}

void setDefaultStyle(ImGuiImageButton* button)
{
	button->setDefaultColor(defaultColor);
	button->setHoveredColor(hoveredColor);
	button->setActiveColor(activeColor);
}

float FERangeRecord::getRangeSpan()
{
	return rangeSpan;
}

std::string FERangeRecord::getCaption()
{
	return caption;
}

void FERangeRecord::setCaption(std::string newValue)
{
	caption = newValue;
}

std::string FERangeRecord::getToolTipText()
{
	return toolTipText;
}

void FERangeRecord::setToolTipText(std::string newValue)
{
	toolTipText = newValue;
}

ImColor FERangeRecord::getColor()
{
	return color;
}

void FERangeRecord::setColor(ImColor newValue)
{
	color = newValue;
}

FERangeConfigurator::FERangeConfigurator()
{
}

ImVec2 FERangeConfigurator::getPosition()
{
	return position;
}

void FERangeConfigurator::setPosition(ImVec2 newPosition)
{
	position = newPosition;
}

ImVec2 FERangeConfigurator::getSize()
{
	return size;
}

void FERangeConfigurator::setSize(ImVec2 newSize)
{
	if (newSize.x < 100)
		newSize.x = 100;
	size = newSize;
}

bool FERangeConfigurator::isVisible()
{
	return visible;
}

void FERangeConfigurator::recalculateRangeInfo()
{
	for (size_t i = 0; i < ranges.size(); i++)
	{
		if (i == 0)
		{
			ranges[i].rect.left = LONG(position.x);
		}
		else
		{
			ranges[i].rect.left = LONG(ranges[i - 1].rect.right);
		}

		ranges[i].rect.right = LONG(ranges[i].rect.left + size.x * ranges[i].rangeSpan);
		ranges[i].rect.top = LONG(position.y);
		ranges[i].rect.bottom = LONG(ranges[i].rect.top + size.y);

		ranges[i].scrollRect.left = LONG(ranges[i].rect.right - SCROLLER_SIZE / 2.0f);
		ranges[i].scrollRect.right = LONG(ranges[i].rect.right + SCROLLER_SIZE / 2.0f);
		ranges[i].scrollRect.top = LONG(ranges[i].rect.top - SCROLLER_SIZE);
		ranges[i].scrollRect.bottom = LONG(ranges[i].rect.top);
	}
}

bool FERangeConfigurator::addRange(float rangeSpan, std::string caption, std::string toolTipText, ImColor color)
{
	if (ranges.size() >= 100)
		return false;

	FERangeRecord newRange;
	newRange.rangeSpan = rangeSpan;

	newRange.caption = caption;
	newRange.toolTipText = toolTipText;
	newRange.color = color;

	ranges.push_back(newRange);
	recalculateRangeInfo();

	return true;
}

void FERangeConfigurator::normalizeRanges()
{
	float coverage = 0.0f;
	for (size_t i = 0; i < ranges.size(); i++)
	{
		coverage += ranges[i].rangeSpan;
	}

	float scale = 1.0f / coverage;
	if (abs(1.0f - coverage) > 0.01f)
	{
		for (size_t i = 0; i < ranges.size(); i++)
		{
			ranges[i].rangeSpan *= scale;
		}

		recalculateRangeInfo();
	}
}

void FERangeConfigurator::inputCalculations()
{
	float mouseXWindows = ImGui::GetIO().MousePos.x - ImGui::GetCurrentWindow()->Pos.x;
	float mouseYWindows = ImGui::GetIO().MousePos.y - ImGui::GetCurrentWindow()->Pos.y;

	for (size_t i = 0; i < ranges.size(); i++)
	{
		if (mouseXWindows >= ranges[i].rect.left && mouseXWindows < ranges[i].rect.right &&
			mouseYWindows >= ranges[i].rect.top && mouseYWindows < ranges[i].rect.bottom)
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(ranges[i].toolTipText.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		if (i == ranges.size() - 1)
			break;

		if (ImGui::GetIO().MouseClicked[0])
		{
			if (mouseXWindows >= ranges[i].scrollRect.left && mouseXWindows < ranges[i].scrollRect.right &&
				mouseYWindows >= ranges[i].scrollRect.top && mouseYWindows < ranges[i].scrollRect.bottom)
			{
				ranges[i].scrollSelected = true;
				break;
			}
			else
			{
				ranges[i].scrollSelected = false;
			}
		}
	}

	if (ImGui::GetIO().MouseReleased[0])
	{
		for (size_t i = 0; i < ranges.size(); i++)
		{
			ranges[i].scrollSelected = false;
		}
	}

	ImGui::GetCurrentWindow()->Flags = ImGuiWindowFlags_None;
	for (size_t i = 0; i < ranges.size(); i++)
	{
		if (ranges[i].scrollSelected)
		{
			ImGui::GetCurrentWindow()->Flags = ImGuiWindowFlags_NoMove;
			float needToAdd = (mouseXWindows - lastMouseX) / size.x;
			if (ranges[i + 1].rangeSpan - needToAdd < 0.001f || ranges[i].rangeSpan + needToAdd < 0.001f)
				break;
			
			ranges[i + 1].rangeSpan -= needToAdd;
			ranges[i].rangeSpan += needToAdd;
			recalculateRangeInfo();
			
			break;
		}
	}

	lastMouseX = mouseXWindows;
}

void FERangeConfigurator::render()
{
	normalizeRanges();
	inputCalculations();

	screenX = ImGui::GetCurrentWindow()->Pos.x + position.x;
	screenY = ImGui::GetCurrentWindow()->Pos.y + position.y;

	float beginX = screenX;
	for (size_t i = 0; i < ranges.size(); i++)
	{
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(screenX + (ranges[i].rect.left - position.x), screenY), ImVec2(beginX + (ranges[i].rect.right - ranges[i].rect.left), screenY + size.y), ranges[i].color);
		ImVec2 textSize = ImGui::CalcTextSize(ranges[i].caption.c_str());
		ImGui::SetCursorPosX(ranges[i].rect.left + (ranges[i].rect.right - ranges[i].rect.left) / 2.0f - textSize.x / 2.0f);
		ImGui::SetCursorPosY(position.y + size.y / 2.0f - textSize.y / 2.0f);
		ImGui::Text(ranges[i].caption.c_str());

		if (i == ranges.size() - 1)
			break;

		ImVec2 P1 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + ranges[i].scrollRect.left,
						   ImGui::GetCurrentWindow()->Pos.y + ranges[i].scrollRect.top);
		ImVec2 P2 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + ranges[i].scrollRect.right,
						   ImGui::GetCurrentWindow()->Pos.y + ranges[i].scrollRect.top);
		ImVec2 P3 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + ranges[i].scrollRect.left + (ranges[i].scrollRect.right - ranges[i].scrollRect.left) / 2.0f,
						   ImGui::GetCurrentWindow()->Pos.y + ranges[i].scrollRect.bottom);

		if (ranges[i].scrollSelected)
		{
			ImGui::GetWindowDrawList()->AddTriangleFilled(P1, P2, P3, ImColor(115, 115, 255, 255));
			ImGui::GetWindowDrawList()->AddRect(ImVec2(screenX + (ranges[i].rect.left - position.x), screenY), ImVec2(beginX + (ranges[i].rect.right - ranges[i].rect.left), screenY + size.y), ImColor(10, 10, 10, 255), 0.0f, 0, 3.0f);
			
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(ranges[i].toolTipText.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		else
		{
			ImGui::GetWindowDrawList()->AddTriangleFilled(P1, P2, P3, ImColor(10, 10, 10, 255));
		}

		beginX += ranges[i].rect.right - ranges[i].rect.left;
	}

	if (ranges.size() == 0)
	{
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(screenX, screenY), ImVec2(screenX + size.x, screenY + size.y), ImColor(125, 125, 125, 255));
		ImVec2 textSize = ImGui::CalcTextSize("No ranges");
		ImGui::SetCursorPosX(position.x + size.x / 2.0f - textSize.x / 2.0f);
		ImGui::SetCursorPosY(position.y + size.y / 2.0f - textSize.y / 2.0f);
		ImGui::Text("No ranges");
	}
}

void FERangeConfigurator::deleteRange(size_t index)
{
	if (index >= ranges.size())
		return;

	ranges.erase(ranges.begin() + index);
}

std::vector<FERangeRecord> FERangeConfigurator::getRangesRecordsCopy()
{
	return ranges;
}

FERangeRecord* FERangeConfigurator::getRangesRecord(size_t index)
{
	if (index >= ranges.size())
		return nullptr;

	return &ranges[index];
}

int FERangeConfigurator::getRangesCount()
{
	return ranges.size();
}

void FERangeConfigurator::clear()
{
	ranges.clear();
}

messagePopUp* messagePopUp::_instance = nullptr;

messagePopUp::messagePopUp() {};

void messagePopUp::show(std::string newWindowCaption, std::string messageToShow)
{
	shouldOpen = true;
	message = messageToShow;
	popupCaption = newWindowCaption;
}

void messagePopUp::render()
{
	ImGuiModalPopup::render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(popupCaption.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowPos(ImVec2(ENGINE.getWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.getWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text(message.c_str());
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - 120.0f / 2.0f);
		if (ImGui::Button("Ok", ImVec2(120, 0)))
		{
			ImGuiModalPopup::close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}