#include "FEEditorTextureSourceNode.h"
using namespace FocalEngine;

FEEditorTextureSourceNode::FEEditorTextureSourceNode(FETexture* texture) : FEVisualNode()
{
	type = "FEEditorTextureSourceNode";
	
	this->texture = texture;
	if (texture == nullptr)
		this->texture = RESOURCE_MANAGER.noTexture;

	setSize(ImVec2(230, 180));
	setName(texture->getName());

	if (texture->getInternalFormat() == GL_RED)
	{
		addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "r"));
	}
	else
	{
		addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "r"));
		addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "g"));
		addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "b"));
		addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "a"));

		addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT, "rgb"));
		addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT, "rgba"));
	}
}

void FEEditorTextureSourceNode::draw()
{
	FEVisualNode::draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 10.0f));
	ImGui::Image((void*)(intptr_t)texture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	if (contextMenu)
	{
		contextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		if (ImGui::MenuItem("Remove texture node"))
		{
			shouldBeDestroyed = true;
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
}

void FEEditorTextureSourceNode::socketEvent(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* connectedSocket, FE_VISUAL_NODE_SOCKET_EVENT eventType)
{
	FEVisualNode::socketEvent(ownSocket,  connectedSocket, eventType);
}

FETexture* FEEditorTextureSourceNode::getTexture()
{
	return texture;
}

bool FEEditorTextureSourceNode::canConnect(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* candidateSocket, char** msgToUser)
{
	if (!FEVisualNode::canConnect(ownSocket, candidateSocket, nullptr))
		return false;

	return false;
}

bool FEEditorTextureSourceNode::openContextMenu()
{
	contextMenu = true;
	return true;
}

Json::Value FEEditorTextureSourceNode::getInfoForSaving()
{
	return "";
}