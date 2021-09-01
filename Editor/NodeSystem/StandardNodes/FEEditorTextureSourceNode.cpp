#include "FEEditorTextureSourceNode.h"
using namespace FocalEngine;

FEEditorTextureSourceNode::FEEditorTextureSourceNode(FETexture* texture)
{
	FEEditorNode::FEEditorNode();
	
	this->texture = texture;
	if (texture == nullptr)
		this->texture = RESOURCE_MANAGER.noTexture;

	setSize(ImVec2(230, 180));
	setName(texture->getName());
	
	addOutputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "r"));
	addOutputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "g"));
	addOutputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "b"));
	addOutputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "a"));

	addOutputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT, "rgb"));
	addOutputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT, "rgba"));
}

void FEEditorTextureSourceNode::draw()
{
	FEEditorNode::draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 10.0f));
	ImGui::Image((void*)(intptr_t)texture->getTextureID(), ImVec2(128, 128), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

	if (openContextMenu)
	{
		openContextMenu = false;
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

void FEEditorTextureSourceNode::socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType)
{
	FEEditorNode::socketEvent(ownSocket,  connectedSocket, eventType);
}

FETexture* FEEditorTextureSourceNode::getTexture()
{
	return texture;
}

bool FEEditorTextureSourceNode::canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser)
{
	if (!FEEditorNode::canConnect(ownSocket, candidateSocket, nullptr))
		return false;

	return false;
}

void FEEditorTextureSourceNode::mouseClick(int mouseButton)
{
	FEEditorNode::mouseClick(mouseButton);

	openContextMenu = false;
	if (mouseButton == 1)
		openContextMenu = true;
}