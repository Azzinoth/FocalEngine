#pragma once

#include "../FEVisualNodeSystem/FEVisualNodeSystem.h"
#include "../FEngine.h"

class FEEditorTextureSourceNode : public FEVisualNode
{
	FETexture* Texture = nullptr;

	bool bContextMenu = false;
	bool OpenContextMenu();
	
	bool CanConnect(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* ConnectedSocket, FE_VISUAL_NODE_SOCKET_EVENT EventType);

	void MouseClick(int MouseButton);

	Json::Value GetInfoForSaving();
public:
	FEEditorTextureSourceNode(FETexture* Texture);

	void Draw();
	FETexture* GetTexture() const;
};