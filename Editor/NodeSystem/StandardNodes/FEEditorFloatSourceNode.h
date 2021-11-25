#pragma once

#include "../FEEditorNodeSystem.h"

class FEEditorFloatSourceNode : public FEEditorNode
{
	float data = 0.0f;

	bool contextMenu = false;
	bool openContextMenu();

	bool canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType);

	void mouseClick(int mouseButton);

	Json::Value getInfoForSaving();
public:
	FEEditorFloatSourceNode(float initialData = 0.0f);

	void draw();
	float getData();
};