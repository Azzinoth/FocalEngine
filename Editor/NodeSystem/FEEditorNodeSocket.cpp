#include "FEEditorNodeSocket.h"
using namespace FocalEngine;

FEEditorNodeSocket::FEEditorNodeSocket(FEEditorNode* parent, FEEditorNodeSocketType type, std::string name)
{
	this->parent = parent;
	this->type = type;
	this->name = name;
}

bool FEEditorNodeSocket::isLookingForConnection()
{
	return lookingForConnection;
}

void FEEditorNodeSocket::setIsLookingForConnection(bool newValue)
{
	lookingForConnection = newValue;
}

FEEditorNode* FEEditorNodeSocket::getParent()
{
	return parent;
}

std::vector<FEEditorNodeSocket*> FEEditorNodeSocket::getConnections()
{
	return connections;
}

std::string FEEditorNodeSocket::getName()
{
	return name;
}

FEEditorNodeSocketType FEEditorNodeSocket::getType()
{
	return type;
}

FEEditorNodeConnection::FEEditorNodeConnection(FEEditorNodeSocket* out, FEEditorNodeSocket* in)
{
	this->out = out;
	this->in = in;
}