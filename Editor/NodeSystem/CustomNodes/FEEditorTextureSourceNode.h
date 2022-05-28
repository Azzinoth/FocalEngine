#pragma once

#include "../FEVisualNodeSystem/FEVisualNodeSystem.h"
#include "../FEngine.h"

class FEEditorTextureSourceNode : public FEVisualNode
{
	FETexture* texture = nullptr;

	bool contextMenu = false;
	bool openContextMenu();
	
	bool canConnect(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* connectedSocket, FE_VISUAL_NODE_SOCKET_EVENT eventType);

	void mouseClick(int mouseButton);

	Json::Value getInfoForSaving();
public:
	FEEditorTextureSourceNode(FETexture* texture);

	void draw();
	FETexture* getTexture();
};