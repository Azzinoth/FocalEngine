#pragma once

#include "../FEVisualNodeSystem/FEVisualNodeSystem.h"
#include "FEEditorTextureSourceNode.h"

class FEEditorMaterialNode: public FEVisualNode
{
	char* IncompatibleTypesMsg = "Incompatible socket types.";
	char* TooManyConnectionsMsg = "Too many connections.";
	char* IncompatibleResolutionMsg = "Incompatible texture resolution.";
	char* CantInferResolutionMsg = "Can't infer texture resolution.";
	char* TooManyConnectionOfThisTypeMsg = "Too many connections of this type.";

	FEMaterial* Data = nullptr;
	
	bool bContextMenu = false;
	bool OpenContextMenu();

	bool CanConnect(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* ConnectedSocket, FE_VISUAL_NODE_SOCKET_EVENT EventType);

	void MouseClick(int MouseButton);

	Json::Value GetInfoForSaving();
public:
	FEEditorMaterialNode(FEMaterial* Material);

	void Draw();
	FEMaterial* GetData() const;
};