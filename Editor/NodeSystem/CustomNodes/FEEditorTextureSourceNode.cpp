#include "FEEditorTextureSourceNode.h"
using namespace FocalEngine;

FEEditorTextureSourceNode::FEEditorTextureSourceNode(FETexture* Texture) : FEVisualNode()
{
	Type = "FEEditorTextureSourceNode";
	
	this->Texture = Texture;
	if (Texture == nullptr)
		this->Texture = RESOURCE_MANAGER.NoTexture;

	SetSize(ImVec2(230, 180));
	SetName(Texture->GetName());

	if (Texture->GetInternalFormat() == GL_RED)
	{
		AddOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "r"));
	}
	else
	{
		AddOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "r"));
		AddOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "g"));
		AddOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "b"));
		AddOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_OUT, "a"));

		AddOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGB_CHANNEL_OUT, "rgb"));
		AddOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT, "rgba"));
	}
}

void FEEditorTextureSourceNode::Draw()
{
	FEVisualNode::Draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 10.0f));
	ImGui::Image((void*)(intptr_t)Texture->GetTextureID(), ImVec2(128, 128), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));

	if (bContextMenu)
	{
		bContextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		if (ImGui::MenuItem("Remove texture node"))
		{
			bShouldBeDestroyed = true;
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
}

void FEEditorTextureSourceNode::SocketEvent(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* ConnectedSocket, const FE_VISUAL_NODE_SOCKET_EVENT EventType)
{
	FEVisualNode::SocketEvent(OwnSocket,  ConnectedSocket, EventType);
}

FETexture* FEEditorTextureSourceNode::GetTexture() const
{
	return Texture;
}

bool FEEditorTextureSourceNode::CanConnect(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* CandidateSocket, char** MsgToUser)
{
	if (!FEVisualNode::CanConnect(OwnSocket, CandidateSocket, nullptr))
		return false;

	return false;
}

bool FEEditorTextureSourceNode::OpenContextMenu()
{
	bContextMenu = true;
	return true;
}

Json::Value FEEditorTextureSourceNode::GetInfoForSaving()
{
	return "";
}