#pragma once

#include "../FEVisualNodeSystem/FEVisualNodeSystem.h"

class FEEditorFloatSourceNode : public FEVisualNode
{
	float Data = 0.0f;

	bool ContextMenu = false;
	bool OpenContextMenu();

	bool CanConnect(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* ConnectedSocket, FE_VISUAL_NODE_SOCKET_EVENT EventType);

	void MouseClick(int MouseButton);

	Json::Value GetInfoForSaving();
public:
	FEEditorFloatSourceNode(float InitialData = 0.0f);

	void Draw();
	float GetData() const;
};