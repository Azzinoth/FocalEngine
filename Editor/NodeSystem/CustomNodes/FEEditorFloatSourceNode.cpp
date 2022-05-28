#include "FEEditorFloatSourceNode.h"
using namespace FocalEngine;

FEEditorFloatSourceNode::FEEditorFloatSourceNode(float initialData) : FEVisualNode()
{
	type = "FEEditorFloatSourceNode";

	data = initialData;

	setSize(ImVec2(220, 78));
	setName("Float");

	titleBackgroundColor = ImColor(31, 117, 208);
	titleBackgroundColorHovered = ImColor(35, 145, 255);
	
	addOutputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_FLOAT_CHANNEL_OUT, "out"));
}

void FEEditorFloatSourceNode::draw()
{	
	FEVisualNode::draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 13.0f));
	ImGui::SetNextItemWidth(140);
	if (ImGui::InputFloat("##data", &data))
	{
		parentArea->propagateUpdateToConnectedNodes(this);
	}
	
	if (contextMenu)
	{
		contextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##context_menu"))
	{
		if (ImGui::MenuItem("Remove float node"))
		{
			shouldBeDestroyed = true;
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
}

void FEEditorFloatSourceNode::socketEvent(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* connectedSocket, FE_VISUAL_NODE_SOCKET_EVENT eventType)
{
	FEVisualNode::socketEvent(ownSocket,  connectedSocket, eventType);
}

float FEEditorFloatSourceNode::getData()
{
	return data;
}

bool FEEditorFloatSourceNode::canConnect(FEVisualNodeSocket* ownSocket, FEVisualNodeSocket* candidateSocket, char** msgToUser)
{
	if (!FEVisualNode::canConnect(ownSocket, candidateSocket, nullptr))
		return false;

	return false;
}

bool FEEditorFloatSourceNode::openContextMenu()
{
	contextMenu = true;
	return true;
}

Json::Value FEEditorFloatSourceNode::getInfoForSaving()
{
	return "";
}