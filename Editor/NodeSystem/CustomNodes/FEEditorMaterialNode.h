#pragma once

#include "../FEVisualNodeSystem/FEVisualNodeSystem.h"
#include "FEEditorTextureSourceNode.h"

class FEEditorMaterialNode: public FEVisualNode
{
	char* incompatibleTypesMsg = "Incompatible socket types.";
	char* tooManyConnectionsMsg = "Too many connections.";
	char* incompatibleResolutionMsg = "Incompatible texture resolution.";
	char* cantInferResolutionMsg = "Can't infer texture resolution.";
	char* tooManyConnectionOfThisTypeMsg = "Too many connections of this type.";

	FEMaterial* data = nullptr;
	
	bool contextMenu = false;
	bool openContextMenu();

	bool canConnect(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* connectedSocket, FE_VISUAL_NODE_SOCKET_EVENT eventType);

	void mouseClick(int mouseButton);

	Json::Value getInfoForSaving();
public:
	FEEditorMaterialNode(FEMaterial* material);

	void draw();
	FEMaterial* getData();
};