#pragma once

#include "../FEVisualNodeSystem/FEVisualNodeSystem.h"

class FEEditorFloatSourceNode : public FEVisualNode
{
	float data = 0.0f;

	bool contextMenu = false;
	bool openContextMenu();

	bool canConnect(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* connectedSocket, FE_VISUAL_NODE_SOCKET_EVENT eventType);

	void mouseClick(int mouseButton);

	Json::Value getInfoForSaving();
public:
	FEEditorFloatSourceNode(float initialData = 0.0f);

	void draw();
	float getData();
};