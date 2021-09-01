#pragma once

#include "../FEEditorNodeSystem.h"

class FEEditorTextureSourceNode : public FEEditorNode
{
	FETexture* texture = nullptr;
	bool openContextMenu = false;
	
	bool canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType);

	void mouseClick(int mouseButton);
public:
	FEEditorTextureSourceNode(FETexture* texture);

	void draw();
	FETexture* getTexture();
};