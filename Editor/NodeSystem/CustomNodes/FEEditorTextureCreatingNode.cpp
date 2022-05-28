#include "FEEditorStandardNodes.h"

using namespace FocalEngine;

FEEditorTextureCreatingNode::FEEditorTextureCreatingNode() : FEVisualNode()
{
	type = "FEEditorTextureCreatingNode";

	this->resultTexture = RESOURCE_MANAGER.noTexture;

	setSize(ImVec2(280, 180));
	setName(resultTexture->getName());

	addInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "r"));
	addInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "g"));
	addInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "b"));
	addInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "a"));
	addInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGB_CHANNEL_IN, "rgb"));
	addInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "rgba"));

	addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "r"));
	addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "g"));
	addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "b"));
	addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "a"));
	addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT, "rgb"));
	addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT, "rgba"));
}

void FEEditorTextureCreatingNode::draw()
{
	FEVisualNode::draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 75.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 10.0f));
	ImGui::Image((void*)(intptr_t)resultTexture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
}

void FEEditorTextureCreatingNode::putDataFromColorChannelInArray(FEVisualNodeSocket* sourceSocket, unsigned char* dataArray, size_t textureDataLenght, size_t toWhatChannel)
{
	FEEditorTextureSourceNode* sourceNode = reinterpret_cast<FEEditorTextureSourceNode*>(sourceSocket->getConnections()[0]->getParent());
	FETexture* sourceTexture = sourceNode->getTexture();
	if (sourceTexture->getWidth() == sourceNode->getTexture()->getWidth() &&
		sourceTexture->getHeight() == sourceNode->getTexture()->getHeight())
	{
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, sourceTexture->getTextureID()));
		unsigned char* textureData = new unsigned char[textureDataLenght];
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData));

		for (size_t i = 0; i < textureDataLenght; i += 4)
		{
			dataArray[i] = textureData[i + toWhatChannel];
			dataArray[i + 1] = textureData[i + toWhatChannel];
			dataArray[i + 2] = textureData[i + toWhatChannel];
			dataArray[i + 3] = 255;
		}

		delete[] textureData;
	}
}

unsigned char* FEEditorTextureCreatingNode::getInputColorChannelData(size_t channel)
{
	// Check if we have source on this channel
	if (input.size() > channel && input[channel]->getConnections().size() > 0)
	{
		if (input[channel]->getConnections()[0]->getType() == FE_NODE_SOCKET_FLOAT_CHANNEL_OUT)
		{
			FEEditorFloatSourceNode* sourceNode = reinterpret_cast<FEEditorFloatSourceNode*>(input[channel]->getConnections()[0]->getParent());
			float data = sourceNode->getData();
			if (data < 0.0f)
				data = 0.0f;

			data *= 255.0f;

			size_t dataLenght = resultTexture->getWidth() * resultTexture->getHeight();
			unsigned char* result = new unsigned char[dataLenght];
			for (size_t i = 0; i < dataLenght; i++)
			{
				result[i] = unsigned char(data);
			}
			
			return result;
		}
		else if (input[channel]->getConnections()[0]->getType() == FE_NODE_SOCKET_COLOR_CHANNEL_OUT)
		{
			FEEditorTextureSourceNode* sourceNode = reinterpret_cast<FEEditorTextureSourceNode*>(input[channel]->getConnections()[0]->getParent());
			FETexture* sourceTexture = sourceNode->getTexture();

			size_t sourceChannel = 0; // "r"
			if (input[channel]->getConnections()[0]->getName() == "g")
			{
				sourceChannel = 1;
			}
			else if (input[channel]->getConnections()[0]->getName() == "b")
			{
				sourceChannel = 2;
			}
			else if (input[channel]->getConnections()[0]->getName() == "a")
			{
				sourceChannel = 3;
			}

			size_t textureDataLenght = sourceTexture->getWidth() * sourceTexture->getHeight() * 4;
			FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, sourceTexture->getTextureID()));
			unsigned char* textureData = new unsigned char[textureDataLenght];
			FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData));

			unsigned char* result = new unsigned char[sourceTexture->getWidth() * sourceTexture->getHeight()];
			int index = 0;
			for (size_t i = 0; i < textureDataLenght; i += 4)
			{
				result[index++] = textureData[i + sourceChannel];
			}

			delete[] textureData;
			return result;
		}
	}
	else if ((input[4]->getConnections().size() > 0 && input[4]->getConnections()[0]->getType() == FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT && channel < 3) ||
			 (input[5]->getConnections().size() > 0 && input[5]->getConnections()[0]->getType() == FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT))
	{
		FEEditorTextureSourceNode* sourceNode = 
			reinterpret_cast<FEEditorTextureSourceNode*>(
				input[4]->getConnections().size() > 0 ? input[4]->getConnections()[0]->getParent() : input[5]->getConnections()[0]->getParent()
				);

		FETexture* sourceTexture = sourceNode->getTexture();

		size_t textureDataLenght = sourceTexture->getWidth() * sourceTexture->getHeight() * 4;
		FE_GL_ERROR(glBindTexture(GL_TEXTURE_2D, sourceTexture->getTextureID()));
		unsigned char* textureData = new unsigned char[textureDataLenght];
		FE_GL_ERROR(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData));

		unsigned char* result = new unsigned char[sourceTexture->getWidth() * sourceTexture->getHeight()];
		int index = 0;
		for (size_t i = 0; i < textureDataLenght; i += 4)
		{
			result[index++] = textureData[i + channel];
		}

		delete[] textureData;
		return result;
	}
	else
	{
		return nullptr;
	}

	return nullptr;
}

void FEEditorTextureCreatingNode::socketEvent(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* connectedSocket, FE_VISUAL_NODE_SOCKET_EVENT eventType)
{
	FEVisualNode::socketEvent(ownSocket, connectedSocket, eventType);

	if (eventType == FE_VISUAL_NODE_SOCKET_DESTRUCTION)
		return;

	size_t textureWidth = 0;
	size_t textureHeight = 0;

	if (connectedSocket->getType() == FE_NODE_SOCKET_FLOAT_CHANNEL_OUT)
	{
		if (resultTexture == RESOURCE_MANAGER.noTexture)
			return;

		FEEditorFloatSourceNode* sourceNode = reinterpret_cast<FEEditorFloatSourceNode*>(connectedSocket->getParent());
		textureWidth = resultTexture->getWidth();
		textureHeight = resultTexture->getHeight();
	}
	else if (connectedSocket->getType() == FE_NODE_SOCKET_COLOR_CHANNEL_OUT || FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT || FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT)
	{
		FEEditorTextureSourceNode* sourceNode = reinterpret_cast<FEEditorTextureSourceNode*>(connectedSocket->getParent());
		textureWidth = sourceNode->getTexture()->getWidth();
		textureHeight = sourceNode->getTexture()->getHeight();
	}

	size_t textureDataLenght = textureWidth * textureHeight * 4;
	unsigned char* resultPixels = new unsigned char[textureDataLenght];

	unsigned char* redChannel = getInputColorChannelData(0);
	unsigned char* greenChannel = getInputColorChannelData(1);
	unsigned char* blueChannel = getInputColorChannelData(2);
	unsigned char* alphaChannel = getInputColorChannelData(3);

	if (redChannel == nullptr && greenChannel == nullptr && blueChannel == nullptr && alphaChannel == nullptr)
	{
		if (resultTexture != RESOURCE_MANAGER.noTexture)
			RESOURCE_MANAGER.deleteFETexture(resultTexture);

		delete[] resultPixels;
		delete[] redChannel;
		delete[] greenChannel;
		delete[] blueChannel;
		delete[] alphaChannel;

		resultTexture = RESOURCE_MANAGER.noTexture;
		return;
	}

	int index = 0;
	for (size_t i = 0; i < textureDataLenght; i += 4)
	{
		resultPixels[i] = redChannel == nullptr ? 0 : redChannel[index];
		resultPixels[i + 1] = greenChannel == nullptr ? 0 : greenChannel[index];
		resultPixels[i + 2] = blueChannel == nullptr ? 0 : blueChannel[index];
		resultPixels[i + 3] = alphaChannel == nullptr ? 255 : alphaChannel[index];
		index++;
	}

	resultTexture = RESOURCE_MANAGER.rawDataToFETexture(resultPixels, int(textureWidth), int(textureHeight), -1);

	delete[] resultPixels;
	delete[] redChannel;
	delete[] greenChannel;
	delete[] blueChannel;
	delete[] alphaChannel;
}

bool FEEditorTextureCreatingNode::canConnect(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* candidateSocket, char** msgToUser)
{
	if (!FEVisualNode::canConnect(ownSocket, candidateSocket, nullptr))
		return false;

	// we reject if sockets have incompatible types.
	bool correctType = true;
	if (candidateSocket->getType() == FE_NODE_SOCKET_COLOR_CHANNEL_OUT &&
		ownSocket->getType() != FE_NODE_SOCKET_COLOR_CHANNEL_IN)
		correctType = false;

	if (candidateSocket->getType() == FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT &&
		ownSocket->getType() != FE_NODE_SOCKET_COLOR_RGB_CHANNEL_IN)
		correctType = false;

	if (candidateSocket->getType() == FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT &&
		ownSocket->getType() != FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN)
		correctType = false;

	if (candidateSocket->getType() == FE_NODE_SOCKET_FLOAT_CHANNEL_OUT &&
		ownSocket->getType() != FE_NODE_SOCKET_COLOR_CHANNEL_IN)
		correctType = false;

	if (!correctType)
	{
		*msgToUser = incompatibleTypesMsg;
		return false;
	}

	// ***************************** tooManyConnectionOfThisTypeMsg *****************************
	if ((input[0]->getConnections().size() > 0 || input[1]->getConnections().size() > 0 || input[2]->getConnections().size() > 0) &&
		(candidateSocket->getType() == FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT))
	{
		*msgToUser = tooManyConnectionOfThisTypeMsg;
		return false;
	}

	if (input[4]->getConnections().size() > 0 && (FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT || FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT))
	{
		if (ownSocket->getName() == "a" && (candidateSocket->getType() == FE_NODE_SOCKET_FLOAT_CHANNEL_OUT || candidateSocket->getType() == FE_NODE_SOCKET_COLOR_CHANNEL_OUT))
		{

		}
		else
		{
			*msgToUser = tooManyConnectionOfThisTypeMsg;
			return false;
		}
	}

	if ((input[5]->getConnections().size() > 0) &&
		(candidateSocket->getType() == FE_NODE_SOCKET_FLOAT_CHANNEL_OUT || FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT || FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT))
	{
		*msgToUser = tooManyConnectionOfThisTypeMsg;
		return false;
	}

	if ((input[0]->getConnections().size() > 0 || input[1]->getConnections().size() > 0 || input[2]->getConnections().size() > 0 || input[3]->getConnections().size() > 0) &&
		(candidateSocket->getType() == FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT))
	{
		*msgToUser = tooManyConnectionOfThisTypeMsg;
		return false;
	}

	if ((input[0]->getConnections().size() > 0 || input[1]->getConnections().size() > 0 || input[2]->getConnections().size() > 0) &&
		(candidateSocket->getType() == FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT))
	{
		*msgToUser = tooManyConnectionOfThisTypeMsg;
		return false;
	}
	// ***************************** tooManyConnectionOfThisTypeMsg END *****************************

	if (candidateSocket->getType() == FE_NODE_SOCKET_FLOAT_CHANNEL_OUT &&
		resultTexture == RESOURCE_MANAGER.noTexture)
	{
		*msgToUser = cantInferResolutionMsg;
		return false;
	}

	// In current node own sockets can't be connected.
	if (candidateSocket->getParent() == this)
	{
		*msgToUser = incompatibleTypesMsg;
		return false;
	}

	// If we have connection on this input we should reject
	if (ownSocket->getConnections().size() > 0)
	{
		*msgToUser = tooManyConnectionsMsg;
		return false;
	}

	// We should check do we already have some texture created.
	// If not we can accept any resolution.
	if (resultTexture == RESOURCE_MANAGER.noTexture)
		return true;

	// But if we already created texture we will check that candidate node have texture with same resolution.
	if (candidateSocket->getType() != FE_NODE_SOCKET_FLOAT_CHANNEL_OUT)
	{
		FEEditorTextureSourceNode* sourceNode = reinterpret_cast<FEEditorTextureSourceNode*>(candidateSocket->getParent());
		if (sourceNode->getTexture()->getWidth() == resultTexture->getWidth() &&
			sourceNode->getTexture()->getHeight() == resultTexture->getHeight())
		{
			return true;
		}
		else
		{
			*msgToUser = incompatibleResolutionMsg;
			return false;
		}
	}
	else
	{
		return true;
	}

	return false;
}

FETexture* FEEditorTextureCreatingNode::getTexture()
{
	return resultTexture;
}

Json::Value FEEditorTextureCreatingNode::getInfoForSaving()
{
	return "";
}