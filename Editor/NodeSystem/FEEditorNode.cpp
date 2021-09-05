#include "FEEditorNode.h"
using namespace FocalEngine;

FEEditorNode::FEEditorNode(int id)
{
	this->id = id;
}

FEEditorNode::~FEEditorNode()
{
	for (int i = 0; i < int(input.size()); i++)
	{
		delete input[i];
		input.erase(input.begin() + i, input.begin() + i + 1);
		i--;
	}

	for (int i = 0; i < int(output.size()); i++)
	{
		delete output[i];
		output.erase(output.begin() + i, output.begin() + i + 1);
		i--;
	}
}

int FEEditorNode::getID()
{
	return id;
}

ImVec2 FEEditorNode::getPosition()
{
	return position;
}

void FEEditorNode::setPosition(ImVec2 newValue)
{
	position = newValue;
}

ImVec2 FEEditorNode::getSize()
{
	return size;
}

void FEEditorNode::setSize(ImVec2 newValue)
{
	size = newValue;
}

std::string FEEditorNode::getName()
{
	return name;
}

void FEEditorNode::setName(std::string newValue)
{
	if (newValue.size() > FEEDITOR_NODE_NAME_MAX_LENGHT)
		return;

	name = newValue;
}

FEEditorNodeSocket* FEEditorNode::isAnySocketHovered()
{
	for (size_t i = 0; i < input.size(); i++)
	{
		if (input[i]->hovered)
			return input[i];
	}

	for (size_t i = 0; i < output.size(); i++)
	{
		if (output[i]->hovered)
			return output[i];
	}

	return nullptr;
}

FEEditorNodeSocket* FEEditorNode::isAnySocketLookingForConnection()
{
	for (size_t i = 0; i < input.size(); i++)
	{
		if (input[i]->isLookingForConnection())
			return input[i];
	}

	for (size_t i = 0; i < output.size(); i++)
	{
		if (output[i]->isLookingForConnection())
			return output[i];
	}

	return nullptr;
}

void FEEditorNode::addInputSocket(FEEditorNodeSocket* socket)
{
	if (socket == nullptr || !FEEditorNode::isSocketTypeIn(socket->type))
		return;

	input.push_back(socket);
}

void FEEditorNode::addOutputSocket(FEEditorNodeSocket* socket)
{
	if (socket == nullptr || FEEditorNode::isSocketTypeIn(socket->type))
		return;

	output.push_back(socket);
}

bool FEEditorNode::isSocketTypeIn(FEEditorNodeSocketType type)
{
	static FEEditorNodeSocketType inSocketTypes[] = { FE_NODE_SOCKET_COLOR_CHANNEL_IN,
													  FE_NODE_SOCKET_FLOAT_CHANNEL_IN,
													  FE_NODE_SOCKET_COLOR_RGB_CHANNEL_IN,
													  FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN };

	static FEEditorNodeSocketType outSocketTypes[] = { FE_NODE_SOCKET_COLOR_CHANNEL_OUT,
													   FE_NODE_SOCKET_FLOAT_CHANNEL_OUT,
													   FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT,
													   FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT};

	int typesCount = (sizeof(inSocketTypes) + sizeof(outSocketTypes)) / sizeof(FEEditorNodeSocketType);
	for (size_t i = 0; i < sizeof(inSocketTypes) / sizeof(FEEditorNodeSocketType); i++)
	{
		if (inSocketTypes[i] == type)
			return true;
	}

	return false;
}

void FEEditorNode::draw()
{

}

void FEEditorNode::socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType)
{

}

bool FEEditorNode::canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser)
{
	if (ownSocket == candidateSocket)
		return false;

	return true;
}

void FEEditorNode::mouseClick(int mouseButton)
{

}

std::string FEEditorNode::getType()
{
	return type;
}