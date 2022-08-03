#pragma once

#include "../FEngine.h"
#include "../ThirdParty/textEditor/TextEditor.h"
using namespace FocalEngine;

//silly windows manager
class ImGuiModalPopup;
class FEImGuiWindow;

class WindowsManager
{
public:
	SINGLETON_PUBLIC_PART(WindowsManager)

	void RegisterWindow(FEImGuiWindow* Window);
	void RegisterPopup(ImGuiModalPopup* Popup);

	bool IsRegisteredWindowWithCaption(std::string Caption) const;
	FEImGuiWindow* GetWindowByCaption(std::string Caption) const;

	void CloseAllPopups() const;
	void CloseAllWindows() const;

	void RenderAllWindows() const;
private:
	SINGLETON_PRIVATE_PART(WindowsManager)

	std::vector<ImGuiModalPopup*> PopUps;
	std::vector<FEImGuiWindow*> Windows;
};

#define FE_IMGUI_WINDOW_MANAGER WindowsManager::getInstance()

class ImGuiModalPopup
{
	friend WindowsManager;
protected:
	bool bShouldOpen;
	bool bOpened;
	std::string PopupCaption;
	ImGuiModalPopup();
public:
	virtual void Show();
	virtual void Close();
	virtual void Render();
	bool IsOpened() const;
};

class ImGuiButton
{
	static const int BUTTON_CAPTION_SIZE = 512;
protected:
	char Caption[BUTTON_CAPTION_SIZE];
	ImVec2 Position = ImVec2(-1.0f, -1.0f);
	ImVec2 Size = ImVec2(120.0f, 0.0f);

	ImVec4 DefaultColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	ImVec4 HoveredColor = ImVec4(0.95f, 0.90f, 0.0f, 1.0f);
	ImVec4 ActiveColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);

	bool bHovered = false;
	bool bWasClicked = false;
	void RenderBegin();
	void RenderEnd();
public:
	ImGuiButton(std::string Caption);
	void Render();

	void SetCaption(std::string NewCaption);

	ImVec4 GetDefaultColor() const;
	void SetDefaultColor(ImVec4 NewDefaultColor);

	ImVec4 GetHoveredColor() const;
	void SetHoveredColor(ImVec4 NewHoveredColor);

	ImVec4 GetActiveColor() const;
	void SetActiveColor(ImVec4 NewActiveColor);

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewPosition);

	ImVec2 GetSize() const;
	void SetSize(ImVec2 NewSize);

	bool IsClicked() const;
};

class ImGuiImageButton
{
protected:
	ImVec2 Position = ImVec2(-1.0f, -1.0f);
	ImVec2 Size = ImVec2(32.0f, 32.0f);
	ImVec2 UV0 = ImVec2(0.0f, 0.0f);
	ImVec2 UV1 = ImVec2(1.0f, 1.0f);
	int FramePadding = 4;
	ImVec4 BackgroundColor = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 TintColor = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

	ImVec4 DefaultColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	ImVec4 HoveredColor = ImVec4(0.95f, 0.90f, 0.0f, 1.0f);
	ImVec4 ActiveColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);

	FETexture* Texture = nullptr;

	bool bHovered = false;
	bool bWasClicked = false;
	void RenderBegin();
	void RenderEnd();
public:
	ImGuiImageButton(FETexture* Texture);
	void Render();

	ImVec4 GetDefaultColor() const;
	void SetDefaultColor(ImVec4 NewDefaultColor);

	ImVec4 GetHoveredColor() const;
	void SetHoveredColor(ImVec4 NewHoveredColor);

	ImVec4 GetActiveColor() const;
	void SetActiveColor(ImVec4 NewActiveColor);

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewPosition);

	ImVec2 GetSize() const;
	void SetSize(ImVec2 NewSize);

	ImVec2 GetUV0() const;
	void SetUV0(ImVec2 NewValue);

	ImVec2 GetUV1() const;
	void SetUV1(ImVec2 NewValue);
	
	int GetFramePadding() const;
	void SetFramePadding(int NewFramePadding);

	ImVec4 GetBackgroundColor() const;
	void SetBackgroundColor(ImVec4 NewBackgroundColor);

	ImVec4 GetTintColor() const;
	void SetTintColor(ImVec4 NewTintColor);

	FETexture* GetTexture() const;
	void SetTexture(FETexture* NewTexture);

	bool IsHovered() const;

	bool IsClicked() const;
};

class FEImGuiWindow
{
	friend WindowsManager;
protected:
	bool bVisible;
	char Caption[512];
	ImVec2 Position;
	ImVec2 Size;
	int Flags = ImGuiWindowFlags_None;
	bool bWasClosedLastFrame = false;
	FEImGuiWindow();
	ImGuiWindow* Window = nullptr;
public:
	virtual ~FEImGuiWindow();
	virtual void Show();
	virtual void Close();
	virtual void Render();
	virtual void OnRenderEnd();

	bool IsVisible() const;
	void SetVisible(bool NewValue);

	bool IsMouseHovered() const;

	virtual void SetCaption(std::string NewCaption);
};

class FEArrowScroller
{
	bool bHorizontal;

	ImVec2 Position;
	bool bSelected;
	bool bMouseHover;

	bool bWindowFlagWasAdded;
	int OriginalWindowFlags;

	RECT Area;
	float Size;

	ImColor Color;
	ImColor SelectedColor;

	float LastFrameMouseX;
	float LastFrameMouseY;
	float LastFrameDelta;

	ImVec2 AvailableRange;
public:
	FEArrowScroller(bool Horizontal = true);

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewPosition);

	float GetSize() const;
	void SetSize(float NewValue);

	bool IsSelected() const;
	void SetSelected(bool NewValue);

	ImColor GetColor() const;
	void SetColor(ImColor NewValue);

	ImColor GetSelectedColor() const;
	void SetSelectedColor(ImColor NewValue);

	float GetLastFrameDelta() const;

	void Render();

	void SetAvailableRange(ImVec2 NewValue);
	void LiftRangeRestrictions();
};

class FERangeConfigurator;
struct FERangeRecord
{
	friend FERangeConfigurator;
private:
	float RangeSpan;
	std::string Caption;
	std::string ToolTipText;
	ImColor Color;

	RECT Rect;
	RECT ScrollRect;

	FEArrowScroller Scroller;
public:
	float GetRangeSpan() const;

	std::string GetCaption();
	void SetCaption(std::string NewValue);

	std::string GetToolTipText();
	void SetToolTipText(std::string NewValue);

	ImColor GetColor() const;
	void SetColor(ImColor NewValue);
};

class FERangeConfigurator
{
	bool bVisible;
	ImVec2 Position;
	ImVec2 Size;
	RECT Rect;

	std::vector<FERangeRecord> Ranges;
	float ScreenX, ScreenY;

	void RecalculateRangeInfo();
	void NormalizeRanges();
	void InputCalculations();
public:
	FERangeConfigurator();

	ImVec2 GetPosition() const;
	void SetPosition(ImVec2 NewPosition);

	ImVec2 GetSize() const;
	void SetSize(ImVec2 NewSize);

	bool IsVisible() const;
	void Render();

	int GetRangesCount() const;

	bool AddRange(float RangeSpan, std::string Caption, std::string ToolTipText, ImColor Color);
	void DeleteRange(size_t Index);

	std::vector<FERangeRecord> GetRangesRecordsCopy();
	FERangeRecord* GetRangesRecord(size_t Index);

	void Clear();
};

void ShowToolTip(const char* Text);
static ImVec4 DefaultColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
static ImVec4 HoveredColor = ImVec4(0.95f, 0.90f, 0.0f, 1.0f);
static ImVec4 ActiveColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);
static ImVec4 SelectedStyle = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);
void SetSelectedStyle(ImGuiImageButton* Button);
void SetDefaultStyle(ImGuiImageButton* Button);

class JustTextWindow : public FEImGuiWindow
{
	TextEditor Editor;
	ImGuiButton* OkButton = nullptr;
public:
	JustTextWindow()
	{
		Flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
		Editor.SetShowWhitespaces(false);
		Editor.SetReadOnly(true);
		Size = ImVec2(800, 600);
		OkButton = new ImGuiButton("OK");
		Editor.SetPalette(TextEditor::GetLightPalette());
	}

	~JustTextWindow()
	{
		delete OkButton;
	}

	void Show(const std::string Text, std::string Caption)
	{
		Editor.SetText(Text);

		if (Caption.empty())
			Caption = "Text view";

		strcpy_s(this->Caption, Caption.size() + 1, Caption.c_str());
		FEImGuiWindow::Show();
	}

	void Render() override
	{
		FEImGuiWindow::Render();

		if (!IsVisible())
			return;

		OkButton->Render();
		if (OkButton->IsClicked())
		{
			FEImGuiWindow::Close();
		}

		Editor.Render("TextEditor");
		FEImGuiWindow::OnRenderEnd();
	}
};
static JustTextWindow JustTextWindowObj;

class MessagePopUp : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(MessagePopUp)
	std::string Message;
public:
	SINGLETON_PUBLIC_PART(MessagePopUp)

	void Show(std::string NewWindowCaption, std::string MessageToShow);
	void Render() override;
};