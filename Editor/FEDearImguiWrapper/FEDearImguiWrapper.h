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

	void registerWindow(FEImGuiWindow* window);
	void registerPopup(ImGuiModalPopup* popup);

	void closeAllPopups();
	void closeAllWindows();
private:
	SINGLETON_PRIVATE_PART(WindowsManager)

	std::vector<ImGuiModalPopup*> popUps;
	std::vector<FEImGuiWindow*> windows;
};

class ImGuiModalPopup
{
	friend WindowsManager;
protected:
	bool shouldOpen;
	bool opened;
	std::string popupCaption;
	ImGuiModalPopup();
public:
	virtual void show();
	virtual void close();
	virtual void render();
	bool isOpened();
};

class ImGuiButton
{
	static const int BUTTON_CAPTION_SIZE = 512;
protected:
	char caption[BUTTON_CAPTION_SIZE];
	ImVec2 position = ImVec2(-1.0f, -1.0f);
	ImVec2 size = ImVec2(120.0f, 0.0f);

	ImVec4 defaultColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	ImVec4 hoveredColor = ImVec4(0.95f, 0.90f, 0.0f, 1.0f);
	ImVec4 activeColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);

	bool hovered = false;
	bool wasClicked = false;
	void renderBegin();
	void renderEnd();
public:
	ImGuiButton(std::string caption);
	void render();

	void setCaption(std::string newCaption);

	ImVec4 getDefaultColor();
	void setDefaultColor(ImVec4 newDefaultColor);

	ImVec4 getHoveredColor();
	void setHoveredColor(ImVec4 newHoveredColor);

	ImVec4 getActiveColor();
	void setActiveColor(ImVec4 newActiveColor);

	ImVec2 getPosition();
	void setPosition(ImVec2 newPosition);

	ImVec2 getSize();
	void setSize(ImVec2 newSize);

	bool getWasClicked();
};

class ImGuiImageButton
{
protected:
	ImVec2 position = ImVec2(-1.0f, -1.0f);
	ImVec2 size = ImVec2(32.0f, 32.0f);
	ImVec2 uv0 = ImVec2(0.0f, 0.0f);
	ImVec2 uv1 = ImVec2(1.0f, 1.0f);
	int framePadding = 4;
	ImVec4 backgroundColor = ImColor(0.0f, 0.0f, 0.0f, 0.0f);
	ImVec4 tintColor = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

	ImVec4 defaultColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	ImVec4 hoveredColor = ImVec4(0.95f, 0.90f, 0.0f, 1.0f);
	ImVec4 activeColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);

	FETexture* texture = nullptr;

	bool hovered = false;
	bool wasClicked = false;
	void renderBegin();
	void renderEnd();
public:
	ImGuiImageButton(FETexture* texture);
	void render();

	ImVec4 getDefaultColor();
	void setDefaultColor(ImVec4 newDefaultColor);

	ImVec4 getHoveredColor();
	void setHoveredColor(ImVec4 newHoveredColor);

	ImVec4 getActiveColor();
	void setActiveColor(ImVec4 newActiveColor);

	ImVec2 getPosition();
	void setPosition(ImVec2 newPosition);

	ImVec2 getSize();
	void setSize(ImVec2 newSize);

	ImVec2 getUV0();
	void setUV0(ImVec2 newUV0);

	ImVec2 getUV1();
	void setUV1(ImVec2 newUV1);
	
	int getFramePadding();
	void setFramePadding(int newFramePadding);

	ImVec4 getBackgroundColor();
	void setBackgroundColor(ImVec4 newBackgroundColor);

	ImVec4 getTintColor();
	void setTintColor(ImVec4 newTintColor);

	FETexture* getTexture();
	void setTexture(FETexture* newTexture);

	bool isHovered();

	bool getWasClicked();
};

class FEImGuiWindow
{
	friend WindowsManager;
protected:
	bool visible;
	char caption[512];
	ImVec2 position;
	ImVec2 size;
	int flags = ImGuiWindowFlags_None;
	bool wasClosedLastFrame = false;
	FEImGuiWindow();
	ImGuiWindow* window = nullptr;
public:
	virtual ~FEImGuiWindow();
	virtual void show();
	virtual void close();
	virtual void render();
	virtual void onRenderEnd();
	bool isVisible();
	bool isMouseHovered();
};

class FEArrowScroller
{
	bool horizontal;

	ImVec2 position;
	bool selected;
	bool mouseHover;

	bool windowFlagWasAdded;
	int originalWindowFlags;

	RECT area;
	float size;

	ImColor color;
	ImColor selectedColor;

	float lastFrameMouseX;
	float lastFrameMouseY;
	float lastFrameDelta;

	ImVec2 availableRange;
public:
	FEArrowScroller(bool Horizontal = true);

	ImVec2 getPosition();
	void setPosition(ImVec2 newPosition);

	float getSize();
	void setSize(float newValue);

	bool isSelected();
	void setSelected(bool newValue);

	ImColor getColor();
	void setColor(ImColor newValue);

	ImColor getSelectedColor();
	void setSelectedColor(ImColor newValue);

	float getLastFrameDelta();

	void render();

	void setAvailableRange(ImVec2 newValue);
	void liftRangeRestrictions();
};

class FERangeConfigurator;
struct FERangeRecord
{
	friend FERangeConfigurator;
private:
	float rangeSpan;
	std::string caption;
	std::string toolTipText;
	ImColor color;

	RECT rect;
	RECT scrollRect;

	FEArrowScroller scroller;
public:
	float getRangeSpan();

	std::string getCaption();
	void setCaption(std::string newValue);

	std::string getToolTipText();
	void setToolTipText(std::string newValue);

	ImColor getColor();
	void setColor(ImColor newValue);
};

class FERangeConfigurator
{
	bool visible;
	ImVec2 position;
	ImVec2 size;
	RECT rect;

	std::vector<FERangeRecord> ranges;
	float screenX, screenY;

	void recalculateRangeInfo();
	void normalizeRanges();
	void inputCalculations();
public:
	FERangeConfigurator();

	ImVec2 getPosition();
	void setPosition(ImVec2 newPosition);

	ImVec2 getSize();
	void setSize(ImVec2 newSize);

	bool isVisible();
	void render();

	int getRangesCount();

	bool addRange(float rangeSpan, std::string caption, std::string toolTipText, ImColor color);
	void deleteRange(size_t index);

	std::vector<FERangeRecord> getRangesRecordsCopy();
	FERangeRecord* getRangesRecord(size_t index);

	void clear();
};

void showToolTip(const char* text);
static ImVec4 defaultColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
static ImVec4 hoveredColor = ImVec4(0.95f, 0.90f, 0.0f, 1.0f);
static ImVec4 activeColor = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);
static ImVec4 selectedStyle = ImVec4(0.1f, 1.0f, 0.1f, 1.0f);
void setSelectedStyle(ImGuiImageButton* button);
void setDefaultStyle(ImGuiImageButton* button);

class justTextWindow : public FEImGuiWindow
{
	TextEditor editor;
	ImGuiButton* okButton = nullptr;
public:
	justTextWindow()
	{
		flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar;
		editor.SetShowWhitespaces(false);
		editor.SetReadOnly(true);
		size = ImVec2(800, 600);
		okButton = new ImGuiButton("OK");
		editor.SetPalette(TextEditor::GetLightPalette());
	}

	~justTextWindow()
	{
		if (okButton != nullptr)
			delete okButton;
	}

	void show(std::string text, std::string caption)
	{
		editor.SetText(text);

		if (caption.size() == 0)
			caption = "Text view";

		strcpy_s(this->caption, caption.size() + 1, caption.c_str());
		FEImGuiWindow::show();
	}

	void render() override
	{
		FEImGuiWindow::render();

		if (!isVisible())
			return;

		okButton->render();
		if (okButton->getWasClicked())
		{
			FEImGuiWindow::close();
		}

		editor.Render("TextEditor");
		FEImGuiWindow::onRenderEnd();
	}
};
static justTextWindow justTextWindowObj;

class messagePopUp : public ImGuiModalPopup
{
	SINGLETON_PRIVATE_PART(messagePopUp)
	std::string message;
public:
	SINGLETON_PUBLIC_PART(messagePopUp)

	void show(std::string newWindowCaption, std::string messageToShow);
	void render() override;
};