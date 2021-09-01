#pragma once

#include "FEEditorNodeSocket.h"

#define FEEDITOR_NODE_NAME_MAX_LENGHT 1024
#define NODE_TITLE_HEIGHT 30.0f

enum FE_EDITOR_NODE_SOCKET_EVENT
{
	FE_EDITOR_NODE_SOCKET_CONNECTED = 0,
	FE_EDITOR_NODE_SOCKET_DISCONNECTED = 1,
	FE_EDITOR_NODE_SOCKET_DESTRUCTION = 2,
	FE_EDITOR_NODE_SOCKET_UPDATE = 3
};

class FEEditorNodeSystem;
class FEEditorNodeArea;
class FEEditorNodeSocket;

class FEEditorNode
{
protected:
	friend FEEditorNodeSystem;
	friend FEEditorNodeArea;

	FEEditorNodeArea* parentArea;
	int id;
	ImVec2 position;
	ImVec2 size;
	std::string name;
	bool shouldBeDestroyed = false;

	std::vector<FEEditorNodeSocket*> input;
	std::vector<FEEditorNodeSocket*> output;

	ImVec2 leftTop;
	ImVec2 rightBottom;

	FEEditorNodeSocket* isAnySocketHovered();
	FEEditorNodeSocket* isAnySocketLookingForConnection();

	ImColor titleBackgroundColor = ImColor(120, 150, 25);
	ImColor titleBackgroundColorHovered = ImColor(140, 190, 35);

	virtual void draw();
	virtual bool canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser = nullptr);
	virtual void socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType);

	virtual void mouseClick(int mouseButton);
public:
	FEEditorNode(int id = -1);
	~FEEditorNode();

	int getID();

	ImVec2 getPosition();
	void setPosition(ImVec2 newValue);

	ImVec2 getSize();
	void setSize(ImVec2 newValue);

	std::string getName();
	void setName(std::string newValue);

	void addInputSocket(FEEditorNodeSocket* socket);
	void addOutputSocket(FEEditorNodeSocket* socket);

	static bool isSocketTypeIn(FEEditorNodeSocketType type);
};