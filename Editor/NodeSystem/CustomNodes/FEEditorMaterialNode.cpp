#include "FEEditorMaterialNode.h"
using namespace FocalEngine;

FEEditorMaterialNode::FEEditorMaterialNode(FEMaterial* Material) : FEVisualNode()
{
	Type = "FEEditorMaterialNode";

	Data = Material;

	SetSize(ImVec2(380, 500));
	SetName(Data->GetName());

	TitleBackgroundColor = ImColor(200, 50, 200);
	TitleBackgroundColorHovered = ImColor(245, 50, 245);
	
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "albedo"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "normal"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "AO"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Roughtness"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Metalness"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Displacement"));

	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "albedo_1"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_IN, "normal_1"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "AO_1"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Roughtness_1"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Metalness_1"));
	AddInputSocket(new FEVisualNodeSocket(this, FE_NODE_SOCKET_COLOR_CHANNEL_IN, "Displacement_1"));
}

void FEEditorMaterialNode::Draw()
{	
	FEVisualNode::Draw();

	ImVec2 CurrentPosition = ImVec2(ImGui::GetCursorScreenPos().x + 180.0f, ImGui::GetCursorScreenPos().y + NODE_TITLE_HEIGHT + 13.0f);
	ImGui::SetCursorScreenPos(CurrentPosition);
	const float FieldWidth = 160.0f;
	const float FieldStep = 30.0f;

	bool bCompactFlag = Data->IsCompackPacking();
	ImGui::Checkbox("##Compact flag", &bCompactFlag);
	ImGui::SameLine();
	ImGui::Text("Compact packing");
	Data->SetCompackPacking(bCompactFlag);

	FEShaderParam* DebugFlag = Data->GetParameter("debugFlag");
	if (DebugFlag != nullptr)
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Debug flag:");
		int IData = *static_cast<int*>(DebugFlag->Data);

		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::SetNextItemWidth(FieldWidth);
		ImGui::SliderInt("##Debug flag", &IData, 0, 10);
		DebugFlag->UpdateData(IData);
	}

	// ************* Normal *************
	CurrentPosition.y += FieldStep;
	ImGui::SetCursorScreenPos(CurrentPosition);
	ImGui::Text("Normal map \nintensity:");
	
	float NormalMapIntensity = Data->GetNormalMapIntensity();
	CurrentPosition.y += FieldStep * 1.5f;
	ImGui::SetCursorScreenPos(CurrentPosition);
	ImGui::SetNextItemWidth(FieldWidth);
	ImGui::DragFloat("##Normal map intensity", &NormalMapIntensity, 0.01f, 0.0f, 1.0f);
	Data->SetNormalMapIntensity(NormalMapIntensity);

	// ************* AO *************
	if (Data->GetAOMap() == nullptr)
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("AO intensity:");
		ImGui::SetNextItemWidth(FieldWidth);
		float AmbientOcclusionIntensity = Data->GetAmbientOcclusionIntensity();
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##AO intensity", &AmbientOcclusionIntensity, 0.01f, 0.0f, 10.0f);
		Data->SetAmbientOcclusionIntensity(AmbientOcclusionIntensity);
	}
	else
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("AO map \nintensity:");
		ImGui::SetNextItemWidth(FieldWidth);
		float AOMapIntensity = Data->GetAmbientOcclusionMapIntensity();
		CurrentPosition.y += FieldStep * 1.5f;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##AO map intensity", &AOMapIntensity, 0.01f, 0.0f, 10.0f);
		Data->SetAmbientOcclusionMapIntensity(AOMapIntensity);
	}

	// ************* Roughtness *************
	if (Data->GetRoughtnessMap() == nullptr)
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Roughtness:");
		ImGui::SetNextItemWidth(FieldWidth);
		float roughtness = Data->GetRoughtness();
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##Roughtness", &roughtness, 0.01f, 0.0f, 1.0f);
		Data->SetRoughtness(roughtness);
	}
	else
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Roughtness map \nintensity:");
		ImGui::SetNextItemWidth(FieldWidth);
		float roughtness = Data->GetRoughtnessMapIntensity();
		CurrentPosition.y += FieldStep * 1.5f;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##Roughtness map intensity", &roughtness, 0.01f, 0.0f, 10.0f);
		Data->SetRoughtnessMapIntensity(roughtness);
	}

	// ************* Metalness *************
	if (Data->GetMetalnessMap() == nullptr)
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Metalness:");
		ImGui::SetNextItemWidth(FieldWidth);
		float metalness = Data->GetMetalness();
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##Metalness", &metalness, 0.01f, 0.0f, 1.0f);
		Data->SetMetalness(metalness);
	}
	else
	{
		CurrentPosition.y += FieldStep;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::Text("Metalness map \nintensity:");
		ImGui::SetNextItemWidth(FieldWidth);
		float metalness = Data->GetMetalnessMapIntensity();
		CurrentPosition.y += FieldStep * 1.5f;
		ImGui::SetCursorScreenPos(CurrentPosition);
		ImGui::DragFloat("##Metalness map intensity", &metalness, 0.01f, 0.0f, 10.0f);
		Data->SetMetalnessMapIntensity(metalness);
	}

	CurrentPosition.y += FieldStep;
	ImGui::SetCursorScreenPos(CurrentPosition);
	ImGui::Text("Tiling :");
	ImGui::SetNextItemWidth(FieldWidth);
	float tiling = Data->GetTiling();
	CurrentPosition.y += FieldStep;
	ImGui::SetCursorScreenPos(CurrentPosition);
	ImGui::DragFloat("##Tiling", &tiling, 0.01f, 0.0f, 64.0f);
	Data->SetTiling(tiling);
	
	if (bContextMenu)
	{
		bContextMenu = false;
		ImGui::OpenPopup("##context_menu");
	}
	
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	//if (ImGui::BeginPopup("##context_menu"))
	//{
	//	/*if (ImGui::MenuItem("Remove float node"))
	//	{
	//		bShouldBeDestroyed = true;
	//	}*/

	//	ImGui::EndPopup();
	//}

	ImGui::PopStyleVar();
}

void FEEditorMaterialNode::SocketEvent(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* ConnectedSocket, const FE_VISUAL_NODE_SOCKET_EVENT EventType)
{
	FEVisualNode::SocketEvent(OwnSocket,  ConnectedSocket, EventType);

	if (EventType == FE_VISUAL_NODE_SOCKET_DESTRUCTION)
		return;

	FETexture* Texture = reinterpret_cast<FEEditorTextureSourceNode*>(ConnectedSocket->GetParent())->GetTexture();
	if (EventType == FE_VISUAL_NODE_SOCKET_DISCONNECTED)
		Texture = nullptr;

	size_t SocketIndex = 0; // "r"
	if (ConnectedSocket->GetName() == "g")
	{
		SocketIndex = 1;
	}
	else if (ConnectedSocket->GetName() == "b")
	{
		SocketIndex = 2;
	}
	else if (ConnectedSocket->GetName() == "a")
	{
		SocketIndex = 3;
	}

	if (OwnSocket->GetName() == "albedo")
	{
		Data->SetAlbedoMap(Texture, 0);
	} 
	else if (OwnSocket->GetName() == "albedo_1")
	{
		Data->SetAlbedoMap(Texture, 1);
	}

	if (OwnSocket->GetName() == "normal")
	{
		Data->SetNormalMap(Texture, 0);
	}
	else if (OwnSocket->GetName() == "normal_1")
	{
		Data->SetNormalMap(Texture, 1);
	}

	if (OwnSocket->GetName() == "AO")
	{
		Data->SetAOMap(Texture, static_cast<int>(SocketIndex), 0);
	}
	else if (OwnSocket->GetName() == "AO_1")
	{
		Data->SetAOMap(Texture, static_cast<int>(SocketIndex), 1);
	}

	if (OwnSocket->GetName() == "Roughtness")
	{
		Data->SetRoughtnessMap(Texture, static_cast<int>(SocketIndex), 0);
	}
	else if (OwnSocket->GetName() == "Roughtness_1")
	{
		Data->SetRoughtnessMap(Texture, static_cast<int>(SocketIndex), 1);
	}

	if (OwnSocket->GetName() == "Metalness")
	{
		Data->SetMetalnessMap(Texture, static_cast<int>(SocketIndex), 0);
	}
	else if (OwnSocket->GetName() == "Metalness_1")
	{
		Data->SetMetalnessMap(Texture, static_cast<int>(SocketIndex), 1);
	}

	if (OwnSocket->GetName() == "Displacement")
	{
		Data->SetDisplacementMap(Texture, static_cast<int>(SocketIndex), 0);
	}
	else if (OwnSocket->GetName() == "Displacement_1")
	{
		Data->SetDisplacementMap(Texture, static_cast<int>(SocketIndex), 1);
	}
}

FEMaterial* FEEditorMaterialNode::GetData() const
{
	return Data;
}

bool FEEditorMaterialNode::CanConnect(FEVisualNodeSocket* OwnSocket, FEVisualNodeSocket* CandidateSocket, char** MsgToUser)
{
	if (!FEVisualNode::CanConnect(OwnSocket, CandidateSocket, nullptr))
		return false;

	// For now it is unsupported type.
	if (CandidateSocket->GetType() == FE_NODE_SOCKET_FLOAT_CHANNEL_OUT)
	{
		*MsgToUser = IncompatibleTypesMsg;
		return false;
	}

	if ((OwnSocket->GetName() == "albedo" || OwnSocket->GetName() == "albedo_1" || OwnSocket->GetName() == "normal" || OwnSocket->GetName() == "normal_1") && (CandidateSocket->GetType() != FE_NODE_SOCKET_COLOR_RGBA_CHANNEL_OUT))
	{
		*MsgToUser = IncompatibleTypesMsg;
		return false;
	}

	if ((OwnSocket->GetName() == "AO" || OwnSocket->GetName() == "AO_1" ||
		OwnSocket->GetName() == "Roughtness" || OwnSocket->GetName() == "Roughtness_1" ||
		OwnSocket->GetName() == "Metalness" || OwnSocket->GetName() == "Metalness_1" ||
		OwnSocket->GetName() == "Displacement" || OwnSocket->GetName() == "Displacement_1") && (CandidateSocket->GetType() != FE_NODE_SOCKET_COLOR_CHANNEL_OUT && CandidateSocket->GetType() != FE_NODE_SOCKET_FLOAT_CHANNEL_OUT))
	{
		*MsgToUser = IncompatibleTypesMsg;
		return false;
	}

	if (!OwnSocket->GetConnections().empty())
	{
		*MsgToUser = TooManyConnectionsMsg;
		return false;
	}

	return true;
}

bool FEEditorMaterialNode::OpenContextMenu()
{
	bContextMenu = true;
	return true;
}

Json::Value FEEditorMaterialNode::GetInfoForSaving()
{
	return "";
}