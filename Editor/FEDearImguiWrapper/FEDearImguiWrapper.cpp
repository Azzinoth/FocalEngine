#include "FEDearImguiWrapper.h"

WindowsManager* WindowsManager::Instance = nullptr;

ImGuiModalPopup::ImGuiModalPopup()
{
	PopupCaption = "";
	bShouldOpen = false;
	bOpened = false;
	WindowsManager::getInstance().RegisterPopup(this);
}

void ImGuiModalPopup::Show()
{
	bShouldOpen = true;
	bOpened = true;
}

void ImGuiModalPopup::Render()
{
	if (bShouldOpen)
	{
		ImGui::OpenPopup(PopupCaption.c_str());
		bShouldOpen = false;
	}
}

bool ImGuiModalPopup::IsOpened() const
{
	return bOpened;
}

void ImGuiModalPopup::Close()
{
	bOpened = false;
	ImGui::CloseCurrentPopup();
}

ImGuiButton::ImGuiButton(std::string Caption)
{
	if (Caption.size() + 1 > BUTTON_CAPTION_SIZE)
		Caption.erase(Caption.begin() + BUTTON_CAPTION_SIZE - 1, Caption.end() - (BUTTON_CAPTION_SIZE + 1));
	
	strcpy_s(this->Caption, Caption.size() + 1, Caption.c_str());
}

void ImGuiButton::SetCaption(std::string NewCaption)
{
	if (NewCaption.size() + 1 > BUTTON_CAPTION_SIZE)
		NewCaption.erase(NewCaption.begin() + BUTTON_CAPTION_SIZE - 1, NewCaption.end() - (BUTTON_CAPTION_SIZE + 1));

	strcpy_s(this->Caption, NewCaption.size() + 1, NewCaption.c_str());
}

ImVec2 ImGuiButton::GetPosition() const
{
	return Position;
}

void ImGuiButton::SetPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;
}

ImVec2 ImGuiButton::GetSize() const
{
	return Size;
}

void ImGuiButton::SetSize(const ImVec2 NewSize)
{
	Size = NewSize;
}

ImVec4 ImGuiButton::GetDefaultColor() const
{
	return DefaultColor;
}

void ImGuiButton::SetDefaultColor(const ImVec4 NewDefaultColor)
{
	DefaultColor = NewDefaultColor;
}

ImVec4 ImGuiButton::GetHoveredColor() const
{
	return HoveredColor;
}

void ImGuiButton::SetHoveredColor(const ImVec4 NewHoveredColor)
{
	HoveredColor = NewHoveredColor;
}

ImVec4 ImGuiButton::GetActiveColor() const
{
	return ActiveColor;
}

void ImGuiButton::SetActiveColor(ImVec4 NewActiveColor)
{
	ActiveColor = NewActiveColor;
}

void ImGuiButton::RenderBegin()
{
	bWasClicked = false;

	ImGui::PushStyleColor(ImGuiCol_Button, DefaultColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HoveredColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ActiveColor);

	if (Position.x != -1.0f)
		ImGui::SetCursorPosX(Position.x);
	
	if (Position.y != -1.0f)
		ImGui::SetCursorPosY(Position.y);
	
	bHovered = false;
	
	ImGui::Button(Caption, Size);

	// flag important for drag and drop functionality
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		bHovered = true;

	if (ImGui::IsMouseClicked(0) && bHovered)
		bWasClicked = true;
}

bool ImGuiButton::IsClicked() const
{
	return bWasClicked;
}

void ImGuiButton::RenderEnd()
{
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void ImGuiButton::Render()
{
	RenderBegin();
	RenderEnd();
}

ImGuiImageButton::ImGuiImageButton(FETexture* Texture)
{
	this->Texture = Texture;
}

ImVec2 ImGuiImageButton::GetPosition() const
{
	return Position;
}

void ImGuiImageButton::SetPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;
}

ImVec2 ImGuiImageButton::GetSize() const
{
	return Size;
}

void ImGuiImageButton::SetSize(const ImVec2 NewSize)
{
	Size = NewSize;
}

ImVec4 ImGuiImageButton::GetDefaultColor() const
{
	return DefaultColor;
}

void ImGuiImageButton::SetDefaultColor(const ImVec4 NewDefaultColor)
{
	DefaultColor = NewDefaultColor;
}

ImVec4 ImGuiImageButton::GetHoveredColor() const
{
	return HoveredColor;
}

void ImGuiImageButton::SetHoveredColor(const ImVec4 NewHoveredColor)
{
	HoveredColor = NewHoveredColor;
}

ImVec4 ImGuiImageButton::GetActiveColor() const
{
	return ActiveColor;
}

void ImGuiImageButton::SetActiveColor(const ImVec4 NewActiveColor)
{
	ActiveColor = NewActiveColor;
}

ImVec2 ImGuiImageButton::GetUV0() const
{
	return UV0;
}

void ImGuiImageButton::SetUV0(const ImVec2 NewValue)
{
	UV0 = NewValue;
}

ImVec2 ImGuiImageButton::GetUV1() const
{
	return UV1;
}

void ImGuiImageButton::SetUV1(const ImVec2 NewValue)
{
	UV1 = NewValue;
}

int ImGuiImageButton::GetFramePadding() const
{
	return FramePadding;
}

void ImGuiImageButton::SetFramePadding(int NewFramePadding)
{
	if (NewFramePadding < 0)
		NewFramePadding = 0;
	FramePadding = NewFramePadding;
}

ImVec4 ImGuiImageButton::GetBackgroundColor() const
{
	return BackgroundColor;
}

void ImGuiImageButton::SetBackgroundColor(const ImVec4 NewBackgroundColor)
{
	BackgroundColor = NewBackgroundColor;
}

ImVec4 ImGuiImageButton::GetTintColor() const
{
	return TintColor;
}

void ImGuiImageButton::SetTintColor(const ImVec4 NewTintColor)
{
	TintColor = NewTintColor;
}

FETexture* ImGuiImageButton::GetTexture() const
{
	return Texture;
}

void ImGuiImageButton::SetTexture(FETexture* NewTexture)
{
	Texture = NewTexture;
}

bool ImGuiImageButton::IsHovered() const
{
	return bHovered;
}

void ImGuiImageButton::RenderBegin()
{
	bWasClicked = false;

	ImGui::PushStyleColor(ImGuiCol_Button, DefaultColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, HoveredColor);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ActiveColor);

	if (Position.x != -1.0f)
		ImGui::SetCursorPosX(Position.x);

	if (Position.y != -1.0f)
		ImGui::SetCursorPosY(Position.y);

	bHovered = false;

	ImGui::ImageButton((void*)static_cast<intptr_t>(Texture->GetTextureID()), Size, UV0, UV1, FramePadding, BackgroundColor, TintColor);

	// flag important for drag and drop functionality
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
		bHovered = true;

	if (ImGui::IsMouseClicked(0) && bHovered)
		bWasClicked = true;
}

bool ImGuiImageButton::IsClicked() const
{
	return bWasClicked;
}

void ImGuiImageButton::RenderEnd()
{
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
}

void ImGuiImageButton::Render()
{
	RenderBegin();
	RenderEnd();
}

FEImGuiWindow::FEImGuiWindow()
{
	Position = ImVec2(0.0f, 0.0f);
	Size = ImVec2(100.0f, 100.0f);
	bVisible = false;
	WindowsManager::getInstance().RegisterWindow(this);
}

FEImGuiWindow::~FEImGuiWindow()
{
}

void FEImGuiWindow::Show()
{
	bVisible = true;
	bWasClosedLastFrame = true;
}

bool FEImGuiWindow::IsMouseHovered() const
{
	if (Window == nullptr)
		return false;

	if (ImGui::GetCurrentContext()->HoveredWindow != nullptr &&
		ImGui::GetCurrentContext()->HoveredWindow == Window)
		return true;

	return false;
}

void FEImGuiWindow::Render()
{
	if (bVisible)
	{
		if (bWasClosedLastFrame)
		{
			ImGui::SetNextWindowPos(Position);
		}
		ImGui::SetNextWindowSize(Size);

		bWasClosedLastFrame = false;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
		ImGui::Begin(Caption, nullptr, Flags);
		Window = ImGui::GetCurrentWindow();
	}
}

void FEImGuiWindow::OnRenderEnd()
{
	if (bVisible)
	{
		if (!Window->Collapsed)
			Size = ImGui::GetWindowSize();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::End();
	}
}

bool FEImGuiWindow::IsVisible() const
{
	return bVisible;
}

void FEImGuiWindow::Close()
{
	if (bVisible)
	{
		bVisible = false;
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::End();
	}
}

void FEImGuiWindow::SetCaption(const std::string NewCaption)
{
	strcpy_s(Caption, NewCaption.size() + 1, NewCaption.c_str());
}

void FEImGuiWindow::SetVisible(bool NewValue)
{
	bVisible = NewValue;
}

WindowsManager::WindowsManager()
{
}

void WindowsManager::RegisterWindow(FEImGuiWindow* Window)
{
	Windows.push_back(Window);
}

void WindowsManager::RegisterPopup(ImGuiModalPopup* Popup)
{
	PopUps.push_back(Popup);
}

void WindowsManager::CloseAllPopups() const
{
	for (size_t i = 0; i < PopUps.size(); i++)
	{
		PopUps[i]->Close();
	}
}

void WindowsManager::CloseAllWindows() const
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		Windows[i]->bVisible = false;
	}
}

void WindowsManager::RenderAllWindows() const
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		Windows[i]->Render();
	}
}

bool WindowsManager::IsRegisteredWindowWithCaption(const std::string Caption) const
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		if (Windows[i]->Caption == Caption)
			return true;
	}

	return false;
}

FEImGuiWindow* WindowsManager::GetWindowByCaption(const std::string Caption) const
{
	for (size_t i = 0; i < Windows.size(); i++)
	{
		if (Windows[i]->Caption == Caption)
			return Windows[i];
	}

	return nullptr;
}

void ShowToolTip(const char* Text)
{
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(Text);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void SetSelectedStyle(ImGuiImageButton* Button)
{
	Button->SetDefaultColor(SelectedStyle);
	Button->SetHoveredColor(SelectedStyle);
	Button->SetActiveColor(SelectedStyle);
}

void SetDefaultStyle(ImGuiImageButton* Button)
{
	Button->SetDefaultColor(DefaultColor);
	Button->SetHoveredColor(HoveredColor);
	Button->SetActiveColor(ActiveColor);
}

float FERangeRecord::GetRangeSpan() const
{
	return RangeSpan;
}

std::string FERangeRecord::GetCaption()
{
	return Caption;
}

void FERangeRecord::SetCaption(const std::string NewValue)
{
	Caption = NewValue;
}

std::string FERangeRecord::GetToolTipText()
{
	return ToolTipText;
}

void FERangeRecord::SetToolTipText(const std::string NewValue)
{
	ToolTipText = NewValue;
}

ImColor FERangeRecord::GetColor() const
{
	return Color;
}

void FERangeRecord::SetColor(const ImColor NewValue)
{
	Color = NewValue;
}

FERangeConfigurator::FERangeConfigurator()
{
}

ImVec2 FERangeConfigurator::GetPosition() const
{
	return Position;
}

void FERangeConfigurator::SetPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;
}

ImVec2 FERangeConfigurator::GetSize() const
{
	return Size;
}

void FERangeConfigurator::SetSize(ImVec2 NewSize)
{
	if (NewSize.x < 100)
		NewSize.x = 100;
	Size = NewSize;
}

bool FERangeConfigurator::IsVisible() const
{
	return bVisible;
}

void FERangeConfigurator::RecalculateRangeInfo()
{
	for (size_t i = 0; i < Ranges.size(); i++)
	{
		if (i == 0)
		{
			Ranges[i].Rect.left = static_cast<LONG>(Position.x);
		}
		else
		{
			Ranges[i].Rect.left = static_cast<LONG>(Ranges[i - 1].Rect.right);
		}

		Ranges[i].Rect.right = static_cast<LONG>(Ranges[i].Rect.left + Size.x * Ranges[i].RangeSpan);
		Ranges[i].Rect.top = static_cast<LONG>(Position.y);
		Ranges[i].Rect.bottom = static_cast<LONG>(Ranges[i].Rect.top + Size.y);
	}
}

bool FERangeConfigurator::AddRange(float RangeSpan, std::string Caption, std::string ToolTipText, ImColor Color)
{
	if (Ranges.size() >= 100)
		return false;

	FERangeRecord NewRange;
	NewRange.RangeSpan = RangeSpan;

	NewRange.Caption = Caption;
	NewRange.ToolTipText = ToolTipText;
	NewRange.Color = Color;

	Ranges.push_back(NewRange);	
	RecalculateRangeInfo();

	for (size_t i = 0; i < Ranges.size(); i++)
	{
		Ranges[i].Scroller.SetPosition(ImVec2(static_cast<float>(Ranges[i].Rect.right), static_cast<float>(Ranges[i].Rect.top)));
	}

	return true;
}

void FERangeConfigurator::NormalizeRanges()
{
	float Coverage = 0.0f;
	for (size_t i = 0; i < Ranges.size(); i++)
	{
		Coverage += Ranges[i].RangeSpan;
	}

	const float Scale = 1.0f / Coverage;
	if (abs(1.0f - Coverage) > 0.01f)
	{
		for (size_t i = 0; i < Ranges.size(); i++)
		{
			Ranges[i].RangeSpan *= Scale;
		}

		RecalculateRangeInfo();

		for (size_t i = 0; i < Ranges.size(); i++)
		{
			Ranges[i].Scroller.SetPosition(ImVec2(static_cast<float>(Ranges[i].Rect.right), static_cast<float>(Ranges[i].Rect.top)));
		}
	}
}

void FERangeConfigurator::InputCalculations()
{
	const float MouseXWindows = ImGui::GetIO().MousePos.x - ImGui::GetCurrentWindow()->Pos.x;
	const float MouseYWindows = ImGui::GetIO().MousePos.y - ImGui::GetCurrentWindow()->Pos.y;

	for (size_t i = 0; i < Ranges.size(); i++)
	{
		if (MouseXWindows >= Ranges[i].Rect.left && MouseXWindows < Ranges[i].Rect.right &&
			MouseYWindows >= Ranges[i].Rect.top && MouseYWindows < Ranges[i].Rect.bottom)
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(Ranges[i].ToolTipText.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		if (i == Ranges.size() - 1)
			break;
	}

	ImGui::GetCurrentWindow()->Flags = ImGuiWindowFlags_None;
	for (size_t i = 0; i < Ranges.size(); i++)
	{
		const float NeedToAdd = Ranges[i].Scroller.GetLastFrameDelta() / Size.x;
		if (NeedToAdd != 0.0f)
		{
			ImGui::GetCurrentWindow()->Flags = ImGuiWindowFlags_NoMove;
			if (Ranges[i + 1].RangeSpan - NeedToAdd < 0.001f || Ranges[i].RangeSpan + NeedToAdd < 0.001f)
				break;
			
			Ranges[i + 1].RangeSpan -= NeedToAdd;
			Ranges[i].RangeSpan += NeedToAdd;
			RecalculateRangeInfo();
			
			break;
		}
	}
}

void FERangeConfigurator::Render()
{
	NormalizeRanges();
	InputCalculations();

	ScreenX = ImGui::GetCurrentWindow()->Pos.x + Position.x;
	ScreenY = ImGui::GetCurrentWindow()->Pos.y + Position.y;

	float BeginX = ScreenX;
	for (size_t i = 0; i < Ranges.size(); i++)
	{
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ScreenX + (Ranges[i].Rect.left - Position.x), ScreenY), ImVec2(BeginX + (Ranges[i].Rect.right - Ranges[i].Rect.left), ScreenY + Size.y), Ranges[i].Color);
		const ImVec2 TextSize = ImGui::CalcTextSize(Ranges[i].Caption.c_str());
		ImGui::SetCursorPosX(Ranges[i].Rect.left + (Ranges[i].Rect.right - Ranges[i].Rect.left) / 2.0f - TextSize.x / 2.0f);
		ImGui::SetCursorPosY(Position.y + Size.y / 2.0f - TextSize.y / 2.0f);
		ImGui::Text(Ranges[i].Caption.c_str());

		if (i == Ranges.size() - 1)
			break;

		Ranges[i].Scroller.Render();
		if (Ranges[i].Scroller.IsSelected())
		{
			ImGui::GetWindowDrawList()->AddRect(ImVec2(ScreenX + (Ranges[i].Rect.left - Position.x), ScreenY), ImVec2(BeginX + (Ranges[i].Rect.right - Ranges[i].Rect.left), ScreenY + Size.y), ImColor(10, 10, 10, 255), 0.0f, 0, 3.0f);

			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(Ranges[i].ToolTipText.c_str());
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}

		BeginX += Ranges[i].Rect.right - Ranges[i].Rect.left;
	}

	// Only one scroller should be selected at a time.
	int SelectedCount = 0;
	for (size_t i = 0; i < Ranges.size(); i++)
	{
		if (Ranges[i].Scroller.IsSelected())
			SelectedCount++;
		
		if (SelectedCount > 1)
			Ranges[i].Scroller.SetSelected(false);
	}

	if (Ranges.empty())
	{
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(ScreenX, ScreenY), ImVec2(ScreenX + Size.x, ScreenY + Size.y), ImColor(125, 125, 125, 255));
		const ImVec2 TextSize = ImGui::CalcTextSize("No ranges");
		ImGui::SetCursorPosX(Position.x + Size.x / 2.0f - TextSize.x / 2.0f);
		ImGui::SetCursorPosY(Position.y + Size.y / 2.0f - TextSize.y / 2.0f);
		ImGui::Text("No ranges");
	}
}

void FERangeConfigurator::DeleteRange(const size_t Index)
{
	if (Index >= Ranges.size())
		return;

	Ranges.erase(Ranges.begin() + Index);
}

std::vector<FERangeRecord> FERangeConfigurator::GetRangesRecordsCopy()
{
	return Ranges;
}

FERangeRecord* FERangeConfigurator::GetRangesRecord(const size_t Index)
{
	if (Index >= Ranges.size())
		return nullptr;

	return &Ranges[Index];
}

int FERangeConfigurator::GetRangesCount() const
{
	return static_cast<int>(Ranges.size());
}

void FERangeConfigurator::Clear()
{
	Ranges.clear();
}

MessagePopUp* MessagePopUp::Instance = nullptr;

MessagePopUp::MessagePopUp() {};

void MessagePopUp::Show(const std::string NewWindowCaption, const std::string MessageToShow)
{
	bShouldOpen = true;
	Message = MessageToShow;
	PopupCaption = NewWindowCaption;
}

void MessagePopUp::Render()
{
	ImGuiModalPopup::Render();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopupModal(PopupCaption.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::SetWindowPos(ImVec2(ENGINE.GetWindowWidth() / 2.0f - ImGui::GetWindowWidth() / 2.0f, ENGINE.GetWindowHeight() / 2.0f - ImGui::GetWindowHeight() / 2.0f));
		ImGui::Text(Message.c_str());
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - 120.0f / 2.0f);
		if (ImGui::Button("Ok", ImVec2(120, 0)))
		{
			ImGuiModalPopup::Close();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	else
	{
		ImGui::PopStyleVar();
	}
}

FEArrowScroller::FEArrowScroller(const bool Horizontal)
{
	bHorizontal = Horizontal;

	bSelected = false;
	bMouseHover = false;

	bWindowFlagWasAdded = false;
	OriginalWindowFlags = 0;

	LastFrameDelta = 0;
	Size = 20.0f;

	Color = ImColor(10, 10, 40, 255);
	SelectedColor = ImColor(115, 115, 255, 255);

	AvailableRange = ImVec2(-FLT_MAX, FLT_MAX);
}

ImVec2 FEArrowScroller::GetPosition() const
{
	return Position;
}

void FEArrowScroller::SetPosition(const ImVec2 NewPosition)
{
	Position = NewPosition;

	if (bHorizontal)
	{
		Area.left = static_cast<LONG>(Position.x - Size / 2.0f);
		Area.right = static_cast<LONG>(Position.x + Size / 2.0f);
		Area.top = static_cast<LONG>(Position.y - Size);
		Area.bottom = static_cast<LONG>(Position.y);
	}
	else
	{
		Area.left = static_cast<LONG>(Position.x - Size);
		Area.right = static_cast<LONG>(Position.x);
		Area.top = static_cast<LONG>(Position.y - Size / 2.0f);
		Area.bottom = static_cast<LONG>(Position.y + Size / 2.0f);
	}
}

bool FEArrowScroller::IsSelected() const
{
	return bSelected;
}

void FEArrowScroller::SetSelected(const bool NewValue)
{
	bSelected = NewValue;
}

void FEArrowScroller::Render()
{
	const float MouseXWindows = ImGui::GetIO().MousePos.x - ImGui::GetCurrentWindow()->Pos.x;
	const float MouseYWindows = ImGui::GetIO().MousePos.y - ImGui::GetCurrentWindow()->Pos.y;

	bMouseHover = false;
	if (MouseXWindows >= Area.left && MouseXWindows < Area.right &&
		MouseYWindows >= Area.top && MouseYWindows < Area.bottom)
	{
		bMouseHover = true;
	}

	if (!bMouseHover && bWindowFlagWasAdded)
	{
		bWindowFlagWasAdded = false;
		ImGui::GetCurrentWindow()->Flags = OriginalWindowFlags;
	}

	if (!(ImGui::GetCurrentWindow()->Flags & ImGuiWindowFlags_NoMove) && bMouseHover)
	{
		bWindowFlagWasAdded = true;
		OriginalWindowFlags = ImGui::GetCurrentWindow()->Flags;
		ImGui::GetCurrentWindow()->Flags |= ImGuiWindowFlags_NoMove;
	}

	if (ImGui::GetIO().MouseClicked[0])
	{
		bMouseHover ? SetSelected(true) : SetSelected(false);
	}

	if (ImGui::GetIO().MouseReleased[0])
		SetSelected(false);

	LastFrameDelta = 0;
	if (IsSelected())
	{
		LastFrameDelta = bHorizontal ? MouseXWindows - LastFrameMouseX : MouseYWindows - LastFrameMouseY;

		if (bHorizontal)
		{
			if (GetPosition().x + LastFrameDelta <= AvailableRange.y && GetPosition().x + LastFrameDelta >= AvailableRange.x)
			{
				SetPosition(ImVec2(GetPosition().x + LastFrameDelta, GetPosition().y));
			}
		}
		else
		{
			if (GetPosition().y + LastFrameDelta <= AvailableRange.y && GetPosition().y + LastFrameDelta >= AvailableRange.x)
			{
				SetPosition(ImVec2(GetPosition().x, GetPosition().y + LastFrameDelta));
			}
		}
	}

	LastFrameMouseX = MouseXWindows;
	LastFrameMouseY = MouseYWindows;

	ImVec2 P1;
	ImVec2 P2;
	ImVec2 P3;

	if (bHorizontal)
	{
		P1 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.left,
			ImGui::GetCurrentWindow()->Pos.y + Area.top);
		P2 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.right,
			ImGui::GetCurrentWindow()->Pos.y + Area.top);
		P3 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.left + (Area.right - Area.left) / 2.0f,
			ImGui::GetCurrentWindow()->Pos.y + Area.bottom);
	}
	else
	{
		P1 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.left,
			ImGui::GetCurrentWindow()->Pos.y + Area.top);
		P2 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.left,
			ImGui::GetCurrentWindow()->Pos.y + Area.bottom);
		P3 = ImVec2(ImGui::GetCurrentWindow()->Pos.x + Area.right,
			ImGui::GetCurrentWindow()->Pos.y + Area.top + (Area.right - Area.left) / 2.0f);
	}

	if (IsSelected())
	{
		ImGui::GetWindowDrawList()->AddTriangleFilled(P1, P2, P3, SelectedColor);
	}
	else
	{
		ImGui::GetWindowDrawList()->AddTriangleFilled(P1, P2, P3, Color);
	}
}

float FEArrowScroller::GetLastFrameDelta() const
{
	return LastFrameDelta;
}

float FEArrowScroller::GetSize() const
{
	return Size;
}

void FEArrowScroller::SetSize(const float NewValue)
{
	if (NewValue > 1.0f)
		Size = NewValue;
}

ImColor FEArrowScroller::GetColor() const
{
	return Color;
}

void FEArrowScroller::SetColor(const ImColor NewValue)
{
	Color = NewValue;
}

ImColor FEArrowScroller::GetSelectedColor() const
{
	return SelectedColor;
}

void FEArrowScroller::SetSelectedColor(const ImColor NewValue)
{
	SelectedColor = NewValue;
}

void FEArrowScroller::SetAvailableRange(const ImVec2 NewValue)
{
	AvailableRange = NewValue;
}

void FEArrowScroller::LiftRangeRestrictions()
{
	AvailableRange = ImVec2(-FLT_MAX, FLT_MAX);
}