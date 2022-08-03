#include "FEEditorFloatSourceNode.h"
using namespace FocalEngine;

FEEditorFloatSourceNode::FEEditorFloatSourceNode(const float InitialData) : FEVisualNode()
{
	Type = "FEEditorFloatSourceNode";

	Data = InitialData;

	SetSize(ImVec2(220, 78));
	SetName("Float");

	TitleBackgroundColor = ImColor(31, 117, 208);
	TitleBackgroundColorHovered = ImColor(35, 145, 255);
	
	AddOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_FLOAT_CHANNEL_OUT, "out"));
}

void FEEditorFloatSourceNode::Draw()
{	
	FEVisualNode::Draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 13.0f));
	ImGui::SetNextItemWidth(140);
	if (ImGui::InputFloat("##data", &Data))
	{
		ParentArea->PropagateUpdateToConnectedNodes(this);
	}
	
	if (ContextMenu)
	{
		ContextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		if (ImGui::MenuItem("Remove float node"))
		{
			bShouldBeDestroyed = true;
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
}

void FEEditorFloatSourceNode::SocketEvent(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* ConnectedSocket, const FE_VISUAL_NODE_SOCKET_EVENT EventType)
{
	FEVisualNode::SocketEvent(OwnSocket,  ConnectedSocket, EventType);
}

float FEEditorFloatSourceNode::GetData() const
{
	return Data;
}

bool FEEditorFloatSourceNode::CanConnect(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* CandidateSocket, char** MsgToUser)
{
	if (!FEVisualNode::CanConnect(OwnSocket, CandidateSocket, nullptr))
		return false;

	return false;
}

bool FEEditorFloatSourceNode::OpenContextMenu()
{
	ContextMenu = true;
	return true;
}

Json::Value FEEditorFloatSourceNode::GetInfoForSaving()
{
	return "";
}