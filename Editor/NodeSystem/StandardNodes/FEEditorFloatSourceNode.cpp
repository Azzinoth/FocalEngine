#include "FEEditorFloatSourceNode.h"
using namespace FocalEngine;

FEEditorFloatSourceNode::FEEditorFloatSourceNode(float initialData)
{
	FEEditorNode::FEEditorNode();
	type = "FEEditorFloatSourceNode";

	data = initialData;

	setSize(ImVec2(220, 78));
	setName("Float");

	titleBackgroundColor = ImColor(31, 117, 208);
	titleBackgroundColorHovered = ImColor(35, 145, 255);
	
	addOutputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_FLOAT_CHANNEL_OUT, "out"));
}

void FEEditorFloatSourceNode::draw()
{	
	FEEditorNode::draw();
	ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + 10.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 13.0f));
	ImGui::SetNextItemWidth(140);
	if (ImGui::InputFloat("##data", &data))
	{
		parentArea->propagateUpdateToConnectedNodes(this);
	}
	
	if (openContextMenu)
	{
		openContextMenu = false;
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

void FEEditorFloatSourceNode::socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType)
{
	FEEditorNode::socketEvent(ownSocket,  connectedSocket, eventType);
}

float FEEditorFloatSourceNode::getData()
{
	return data;
}

bool FEEditorFloatSourceNode::canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser)
{
	if (!FEEditorNode::canConnect(ownSocket, candidateSocket, nullptr))
		return false;

	return false;
}

void FEEditorFloatSourceNode::mouseClick(int mouseButton)
{
	FEEditorNode::mouseClick(mouseButton);

	openContextMenu = false;
	if (mouseButton == 1)
		openContextMenu = true;
}