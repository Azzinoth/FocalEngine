#pragma once

#include "../FEEditorNodeSystem.h"

class FEEditorTextureSourceNode : public FEEditorNode
{
	FETexture* texture = nullptr;

	bool contextMenu = false;
	bool openContextMenu();
	
	bool canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType);

	void mouseClick(int mouseButton);

	Json::Value getInfoForSaving();
public:
	FEEditorTextureSourceNode(FETexture* texture);

	void draw();
	FETexture* getTexture();
};