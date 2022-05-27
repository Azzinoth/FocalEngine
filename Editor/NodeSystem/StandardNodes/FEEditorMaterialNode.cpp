#include "FEEditorMaterialNode.h"
using namespace FocalEngine;

FEEditorMaterialNode::FEEditorMaterialNode(FEMaterial* material) : FEEditorNode()
{
	type = "FEEditorMaterialNode";

	data = material;

	setSize(ImVec2(380, 500));
	setName(data->getName());

	titleBackgroundColor = ImColor(200, 50, 200);
	titleBackgroundColorHovered = ImColor(245, 50, 245);
	
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "albedo"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "normal"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "AO"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Roughtness"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Metalness"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Displacement"));

	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "albedo_1"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "normal_1"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "AO_1"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Roughtness_1"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Metalness_1"));
	addInputSocket(new FEEditorNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Displacement_1"));
}

void FEEditorMaterialNode::draw()
{	
	FEEditorNode::draw();

	ImVec2 currentPosition = ImVec2(ImGui::GetCursorScreenPos().x + 180.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 13.0f);
	ImGui::SetCursorScreenPos(currentPosition);
	float fieldWidth = 160.0f;
	float fieldStep = 30.0f;

	bool compactFlag = data->isCompackPacking();
	ImGui::Checkbox("##Compact flag", &compactFlag);
	ImGui::SameLine();
	ImGui::Text("Compact packing");
	data->setCompackPacking(compactFlag);

	FEShaderParam* debugFlag = data->getParameter("debugFlag");
	if (debugFlag != nullptr)
	{
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::Text("Debug flag:");
		int iData = *(int*)debugFlag->data;

		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::SetNextItemWidth(fieldWidth);
		ImGui::SliderInt("##Debug flag", &iData, 0, 10);
		debugFlag->updateData(iData);
	}

	// ************* Normal *************
	currentPosition.y += fieldStep;
	ImGui::SetCursorScreenPos(currentPosition);
	ImGui::Text("Normal map \nintensity:");
	
	float normalMapIntensity = data->getNormalMapIntensity();
	currentPosition.y += fieldStep * 1.5f;
	ImGui::SetCursorScreenPos(currentPosition);
	ImGui::SetNextItemWidth(fieldWidth);
	ImGui::DragFloat("##Normal map intensity", &normalMapIntensity, 0.01f, 0.0f, 1.0f);
	data->setNormalMapIntensity(normalMapIntensity);

	// ************* AO *************
	if (data->getAOMap() == nullptr)
	{
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::Text("AO intensity:");
		ImGui::SetNextItemWidth(fieldWidth);
		float ambientOcclusionIntensity = data->getAmbientOcclusionIntensity();
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::DragFloat("##AO intensity", &ambientOcclusionIntensity, 0.01f, 0.0f, 10.0f);
		data->setAmbientOcclusionIntensity(ambientOcclusionIntensity);
	}
	else
	{
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::Text("AO map \nintensity:");
		ImGui::SetNextItemWidth(fieldWidth);
		float AOMapIntensity = data->getAmbientOcclusionMapIntensity();
		currentPosition.y += fieldStep * 1.5f;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::DragFloat("##AO map intensity", &AOMapIntensity, 0.01f, 0.0f, 10.0f);
		data->setAmbientOcclusionMapIntensity(AOMapIntensity);
	}

	// ************* Roughtness *************
	if (data->getRoughtnessMap() == nullptr)
	{
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::Text("Roughtness:");
		ImGui::SetNextItemWidth(fieldWidth);
		float roughtness = data->getRoughtness();
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::DragFloat("##Roughtness", &roughtness, 0.01f, 0.0f, 1.0f);
		data->setRoughtness(roughtness);
	}
	else
	{
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::Text("Roughtness map \nintensity:");
		ImGui::SetNextItemWidth(fieldWidth);
		float roughtness = data->getRoughtnessMapIntensity();
		currentPosition.y += fieldStep * 1.5f;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::DragFloat("##Roughtness map intensity", &roughtness, 0.01f, 0.0f, 10.0f);
		data->setRoughtnessMapIntensity(roughtness);
	}

	// ************* Metalness *************
	if (data->getMetalnessMap() == nullptr)
	{
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::Text("Metalness:");
		ImGui::SetNextItemWidth(fieldWidth);
		float metalness = data->getMetalness();
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::DragFloat("##Metalness", &metalness, 0.01f, 0.0f, 1.0f);
		data->setMetalness(metalness);
	}
	else
	{
		currentPosition.y += fieldStep;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::Text("Metalness map \nintensity:");
		ImGui::SetNextItemWidth(fieldWidth);
		float metalness = data->getMetalnessMapIntensity();
		currentPosition.y += fieldStep * 1.5f;
		ImGui::SetCursorScreenPos(currentPosition);
		ImGui::DragFloat("##Metalness map intensity", &metalness, 0.01f, 0.0f, 10.0f);
		data->setMetalnessMapIntensity(metalness);
	}

	currentPosition.y += fieldStep;
	ImGui::SetCursorScreenPos(currentPosition);
	ImGui::Text("Tiling :");
	ImGui::SetNextItemWidth(fieldWidth);
	float tiling = data->getTiling();
	currentPosition.y += fieldStep;
	ImGui::SetCursorScreenPos(currentPosition);
	ImGui::DragFloat("##Tiling", &tiling, 0.01f, 0.0f, 64.0f);
	data->setTiling(tiling);
	
	if (contextMenu)
	{
		contextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	//if (ImGui::BeginPopup("##context_menu"))
	//{
	//	/*if (ImGui::MenuItem("Remove float node"))
	//	{
	//		shouldBeDestroyed = true;
	//	}*/

	//	ImGui::EndPopup();
	//}

	ImGui::PopStyleVar();
}

void FEEditorMaterialNode::socketEvent(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* connectedSocket, FE_EDITOR_NODE_SOCKET_EVENT eventType)
{
	FEEditorNode::socketEvent(ownSocket,  connectedSocket, eventType);

	if (eventType == FE_EDITOR_NODE_SOCKET_DESTRUCTION)
		return;

	FETexture* texure = reinterpret_cast<FEEditorTextureSourceNode*>(connectedSocket->getParent())->getTexture();
	if (eventType == FE_EDITOR_NODE_SOCKET_DISCONNECTED)
		texure = nullptr;

	size_t socketIndex = 0; // "r"
	if (connectedSocket->getName() == "g")
	{
		socketIndex = 1;
	}
	else if (connectedSocket->getName() == "b")
	{
		socketIndex = 2;
	}
	else if (connectedSocket->getName() == "a")
	{
		socketIndex = 3;
	}

	if (ownSocket->getName() == "albedo")
	{
		data->setAlbedoMap(texure, 0);
	} 
	else if (ownSocket->getName() == "albedo_1")
	{
		data->setAlbedoMap(texure, 1);
	}

	if (ownSocket->getName() == "normal")
	{
		data->setNormalMap(texure, 0);
	}
	else if (ownSocket->getName() == "normal_1")
	{
		data->setNormalMap(texure, 1);
	}

	if (ownSocket->getName() == "AO")
	{
		data->setAOMap(texure, int(socketIndex), 0);
	}
	else if (ownSocket->getName() == "AO_1")
	{
		data->setAOMap(texure, int(socketIndex), 1);
	}

	if (ownSocket->getName() == "Roughtness")
	{
		data->setRoughtnessMap(texure, int(socketIndex), 0);
	}
	else if (ownSocket->getName() == "Roughtness_1")
	{
		data->setRoughtnessMap(texure, int(socketIndex), 1);
	}

	if (ownSocket->getName() == "Metalness")
	{
		data->setMetalnessMap(texure, int(socketIndex), 0);
	}
	else if (ownSocket->getName() == "Metalness_1")
	{
		data->setMetalnessMap(texure, int(socketIndex), 1);
	}

	if (ownSocket->getName() == "Displacement")
	{
		data->setDisplacementMap(texure, int(socketIndex), 0);
	}
	else if (ownSocket->getName() == "Displacement_1")
	{
		data->setDisplacementMap(texure, int(socketIndex), 1);
	}
}

FEMaterial* FEEditorMaterialNode::getData()
{
	return data;
}

bool FEEditorMaterialNode::canConnect(FEEditorNodeSocket* ownSocket, FEEditorNodeSocket* candidateSocket, char** msgToUser)
{
	if (!FEEditorNode::canConnect(ownSocket, candidateSocket, nullptr))
		return false;

	// For now it is unsupported type.
	if (candidateSocket->getType() == FE_NODE_SOCKET_FLOAT_CHANNEL_OUT)
	{
		*msgToUser = incompatibleTypesMsg;
		return false;
	}

	if ((ownSocket->getName() == "albedo" || ownSocket->getName() == "albedo_1" || ownSocket->getName() == "normal" || ownSocket->getName() == "normal_1") && (candidateSocket->getType() != FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT))
	{
		*msgToUser = incompatibleTypesMsg;
		return false;
	}

	if ((ownSocket->getName() == "AO" || ownSocket->getName() == "AO_1" ||
		ownSocket->getName() == "Roughtness" || ownSocket->getName() == "Roughtness_1" ||
		ownSocket->getName() == "Metalness" || ownSocket->getName() == "Metalness_1" ||
		ownSocket->getName() == "Displacement" || ownSocket->getName() == "Displacement_1") && (candidateSocket->getType() != FE_NODE_SOCKET_COLOR_CHANNEL_OUT && candidateSocket->getType() != FE_NODE_SOCKET_FLOAT_CHANNEL_OUT))
	{
		*msgToUser = incompatibleTypesMsg;
		return false;
	}

	if (ownSocket->getConnections().size() > 0)
	{
		*msgToUser = tooManyConnectionsMsg;
		return false;
	}

	return true;
}

bool FEEditorMaterialNode::openContextMenu()
{
	contextMenu = true;
	return true;
}

Json::Value FEEditorMaterialNode::getInfoForSaving()
{
	return "";
}