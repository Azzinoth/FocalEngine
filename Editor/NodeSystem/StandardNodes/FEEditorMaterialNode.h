#pragma once

#include "../FEEditorNodeSystem.h"
#include "FEEditorTextureSourceNode.h"

class FEEditorMaterialNode: public FEEditorNode
{
	char* incompatibleTypesMsg = "Incompatible socket types.";
	char* tooManyConnectionsMsg = "Too many connections.";
	char* incompatibleResolutionMsg = "Incompatible texture resolution.";
	char* cantInferResolutionMsg = "Can't infer texture resolution.";
	char* tooManyConnectionOfThisTypeMsg = "Too many connections of this type.";

	FEMaterial* data = nullptr;
	bool openContextMenu = false;

	bool canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType);

	void mouseClick(int mouseButton);
public:
	FEEditorMaterialNode(FEMaterial* material);

	void draw();
	FEMaterial* getData();
};