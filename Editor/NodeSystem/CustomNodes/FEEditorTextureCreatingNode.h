#pragma once

#include "../FEVisualNodeSystem/FEVisualNodeSystem.h"

class FEEditorTextureCreatingNode : public FEVisualNode
{
	FETexture* ResultTexture = nullptr;
	char* IncompatibleTypesMsg = "Incompatible socket types.";
	char* TooManyConnectionsMsg = "Too many connections.";
	char* IncompatibleResolutionMsg = "Incompatible texture resolution.";
	char* CantInferResolutionMsg = "Can't infer texture resolution.";
	char* TooManyConnectionOfThisTypeMsg = "Too many connections of this type.";

	void PutDataFromColorChannelInArray(FEVisualNodeSocket* SourceSocket, unsigned char* DataArray, size_t TextureDataLenght, size_t ToWhatChannel);
	unsigned char* GetInputColorChannelData(size_t Channel) const;

	bool CanConnect(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* CandidateSocket, char** MsgToUser);
	void SocketEvent(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* ConnectedSocket, FE_VISUAL_NODE_SOCKET_EVENT EventType);

	Json::Value GetInfoForSaving();
public:
	FEEditorTextureCreatingNode();

	void Draw();
	FETexture* GetTexture() const;
};