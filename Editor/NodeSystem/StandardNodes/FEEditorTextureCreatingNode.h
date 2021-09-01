#pragma once

#include "../FEEditorNodeSystem.h"

class FEEditorTextureCreatingNode : public FEEditorNode
{
	FETexture* resultTexture = nullptr;
	char* incompatibleTypesMsg = "Incompatible socket types.";
	char* tooManyConnectionsMsg = "Too many connections.";
	char* incompatibleResolutionMsg = "Incompatible texture resolution.";
	char* cantInferResolutionMsg = "Can't infer texture resolution.";
	char* tooManyConnectionOfThisTypeMsg = "Too many connections of this type.";

	void putDataFromColorChannelInArray(FEEditorNodeSocket* sourceSocket, unsigned char* dataArray, size_t textureDataLenght, size_t toWhatChannel);
	unsigned char* getInputColorChennelData(size_t channel);

	bool canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType);
public:
	FEEditorTextureCreatingNode();

	void draw();
	FETexture* getTexture();
};