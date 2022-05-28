#pragma once

#include "../FEVisualNodeSystem/FEVisualNodeSystem.h"

class FEEditorTextureCreatingNode : public FEVisualNode
{
	FETexture* resultTexture = nullptr;
	char* incompatibleTypesMsg = "Incompatible socket types.";
	char* tooManyConnectionsMsg = "Too many connections.";
	char* incompatibleResolutionMsg = "Incompatible texture resolution.";
	char* cantInferResolutionMsg = "Can't infer texture resolution.";
	char* tooManyConnectionOfThisTypeMsg = "Too many connections of this type.";

	void putDataFromColorChannelInArray(FEVisualNodeSocket* sourceSocket, unsigned char* dataArray, size_t textureDataLenght, size_t toWhatChannel);
	unsigned char* getInputColorChannelData(size_t channel);

	bool canConnect(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* candidateSocket, char** msgToUser);
	void socketEvent(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* connectedSocket, FE_VISUAL_NODE_SOCKET_EVENT eventType);

	Json::Value getInfoForSaving();
public:
	FEEditorTextureCreatingNode();

	void draw();
	FETexture* getTexture();
};