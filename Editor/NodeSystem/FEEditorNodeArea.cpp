#include "FEEditorNodeArea.h"
using namespace FocalEngine;

FEEditorNodeArea::FEEditorNodeArea() {};

void FEEditorNodeArea::addNode(FEEditorNode* newNode)
{
	newNode->id = nodeIDCounter++;
	newNode->parentArea = this;
	newNode->setPosition(ImVec2(newNode->getPosition().x - getAreaRenderOffset().x, newNode->getPosition().y - getAreaRenderOffset().y));

	nodes.push_back(newNode);
}

FEEditorNodeArea::~FEEditorNodeArea() 
{
	clear();
}

bool ColorPicker3U32(const char* label, ImU32* color)
{
	float col[3];
	col[0] = (float)((*color) & 0xFF) / 255.0f;
	col[1] = (float)((*color >> 8) & 0xFF) / 255.0f;
	col[2] = (float)((*color >> 16) & 0xFF) / 255.0f;

	bool result = ImGui::ColorPicker3(label, col);

	*color = ((ImU32)(col[0] * 255.0f)) |
		((ImU32)(col[1] * 255.0f) << 8) |
		((ImU32)(col[2] * 255.0f) << 16);

	return result;
}

void FEEditorNodeArea::renderNode(FEEditorNode* node)
{
	if (currentDrawList == nullptr || node == nullptr)
		return;

	ImGui::PushID(node->getID());

	node->leftTop = ImVec2(ImGui::GetCurrentWindow()->Pos.x + node->getPosition().x + renderOffset.x, ImGui::GetCurrentWindow()->Pos.y + node->getPosition().y + renderOffset.y);
	node->rightBottom = ImVec2(node->leftTop.x + node->getSize().x, node->leftTop.y + node->getSize().y);

	currentDrawList->ChannelsSetCurrent(2);
	
	ImGui::SetCursorScreenPos(node->leftTop);
	node->draw();

	currentDrawList->ChannelsSetCurrent(1);

	// Mouse inputs
	ImGui::SetCursorScreenPos(node->leftTop);
	ImGui::InvisibleButton("Node body", node->getSize());

	if (ImGui::IsItemHovered())
	{
		nodeHoveredID = node->getID();

		if (ImGui::IsMouseClicked(1))
		{
			rightMouseClickWasOnNode = true;
			node->mouseClick(1);
		}
	}

	if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0) && node->isAnySocketHovered() == nullptr && node->isAnySocketLookingForConnection() == nullptr)
	{
		nodeWasDragged = true;
		node->setPosition(ImVec2(node->getPosition().x + ImGui::GetIO().MouseDelta.x, node->getPosition().y + ImGui::GetIO().MouseDelta.y));
	}

	// Drawing node background layer.
	ImU32 nodeBackgroundColor = nodeHoveredID == node->getID() ? IM_COL32(75, 75, 75, 125) : IM_COL32(60, 60, 60, 125);
	currentDrawList->AddRectFilled(node->leftTop, node->rightBottom, nodeBackgroundColor, 8.0f);

	// Drawing caption area.
	ImVec2 titleArea = node->rightBottom;
	titleArea.y = node->leftTop.y + NODE_TITLE_HEIGHT;
	ImU32 nodeTitleBackgroundColor = nodeHoveredID == node->getID() ? node->titleBackgroundColorHovered : node->titleBackgroundColor;
	currentDrawList->AddRectFilled(ImVec2(node->leftTop.x + 1, node->leftTop.y + 1), titleArea, nodeTitleBackgroundColor, 8.0f);
	currentDrawList->AddRect(node->leftTop, node->rightBottom, ImColor(100, 100, 100), 8.0f);

	ImVec2 textSize = ImGui::CalcTextSize(node->getName().c_str());
	ImVec2 textPosition;
	textPosition.x = node->leftTop.x + (node->getSize().x / 2) - textSize.x / 2;
	textPosition.y = node->leftTop.y + (NODE_TITLE_HEIGHT / 2) - textSize.y / 2;

	ImGui::SetCursorScreenPos(textPosition);
	ImGui::Text(node->getName().c_str());

	renderNodeSockets(node);

	ImGui::PopID();
}

void FEEditorNodeArea::renderNodeSockets(FEEditorNode* node)
{
	for (size_t i = 0; i < node->input.size(); i++)
	{
		renderNodeSocket(node->input[i]);
	}

	for (size_t i = 0; i < node->output.size(); i++)
	{
		renderNodeSocket(node->output[i]);
	}
}

void FEEditorNodeArea::renderNodeSocket(FEEditorNodeSocket* socket)
{
	ImVec2 socketPosition = socketToPosition(socket);
	bool input = FEEditorNode::isSocketTypeIn(socket->getType());
	// Socket description.
	ImVec2 textSize = ImGui::CalcTextSize(socket->getName().c_str());

	float textX = socketPosition.x;
	textX += input ? NODE_SOCKET_SIZE * 2.0f : -NODE_SOCKET_SIZE * 2.0f - textSize.x;

	ImGui::SetCursorScreenPos(ImVec2(textX, socketPosition.y - textSize.y / 2.0f));
	ImGui::Text(socket->getName().c_str());

	// Is socket hovered.
	socket->hovered = false;
	ImVec2 mousePosition = ImGui::GetIO().MousePos;
	ImColor socketColor = ImColor(150, 150, 150);
	if (mousePosition.x >= socketPosition.x - NODE_SOCKET_SIZE &&
		mousePosition.x <= socketPosition.x + NODE_SOCKET_SIZE &&
		mousePosition.y >= socketPosition.y - NODE_SOCKET_SIZE &&
		mousePosition.y <= socketPosition.y + NODE_SOCKET_SIZE)
	{
		socket->hovered = true;
		socketColor = ImColor(200, 200, 200);

		// If potential connection can't be established we will provide visual indication.
		if (socketHovered != nullptr && socketLookingForConnection != nullptr && socketHovered == socket)
		{
			char** msg = new char*;
			*msg = nullptr;
			socketColor = socketHovered->getParent()->canConnect(socketHovered, socketLookingForConnection, msg) ?
				ImColor(50, 200, 50) : ImColor(200, 50, 50);

			if (*msg != nullptr)
			{
				ImGui::Begin("socket connection info", NULL, ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
				ImGui::Text(*msg);
				ImGui::End();

				delete msg;
			}
		}
	}

	// Should socket be looking for connection.
	if (ImGui::GetIO().MouseReleased[0])
		socket->setIsLookingForConnection(false);

	if (socket->hovered && ImGui::GetIO().MouseClicked[0] && !input)
		socket->setIsLookingForConnection(true);

	if (socket->isLookingForConnection())
	{
		drawHermiteLine(socketPosition, ImGui::GetIO().MousePos, 12);
	}

	// Draw socket icon.
	currentDrawList->AddCircleFilled(socketPosition, NODE_SOCKET_SIZE, socketColor);
}

void FEEditorNodeArea::renderGrid(ImVec2 currentPosition)
{
	currentDrawList->ChannelsSplit(2);

	currentPosition.x += renderOffset.x;
	currentPosition.y += renderOffset.y;
	
	// Horizontal lines
	int startingStep = int(ceil(-10000.0f / FEEDITOR_NODE_GRID_STEP));
	int stepCount = int(ceil(10000.0f / FEEDITOR_NODE_GRID_STEP));
	for (int i = startingStep; i < stepCount; i++)
	{
		ImVec2 from = ImVec2(currentPosition.x - 10000.0f , currentPosition.y + i * FEEDITOR_NODE_GRID_STEP);
		ImVec2 to = ImVec2(currentPosition.x + 10000.0f, currentPosition.y + i * FEEDITOR_NODE_GRID_STEP);

		if (i % 10 != 0)
		{
			currentDrawList->ChannelsSetCurrent(1);
			currentDrawList->AddLine(from, to, ImGui::GetColorU32(gridLinesColor), 1);
		}
		else
		{
			currentDrawList->ChannelsSetCurrent(0);
			currentDrawList->AddLine(from, to, ImGui::GetColorU32(gridBoldLinesColor), 3);
		}
	}

	// Vertical lines
	for (int i = startingStep; i < stepCount; i++)
	{
		ImVec2 from = ImVec2(currentPosition.x + i * FEEDITOR_NODE_GRID_STEP, currentPosition.y - 10000.0f);
		ImVec2 to = ImVec2(currentPosition.x + i * FEEDITOR_NODE_GRID_STEP, currentPosition.y + 10000.0f);

		if (i % 10 != 0)
		{
			currentDrawList->ChannelsSetCurrent(1);
			currentDrawList->AddLine(from, to, ImGui::GetColorU32(gridLinesColor), 1);
		}
		else
		{
			currentDrawList->ChannelsSetCurrent(0);
			currentDrawList->AddLine(from, to, ImGui::GetColorU32(gridBoldLinesColor), 3);
		}
	}

	currentDrawList->ChannelsMerge();
}

void FEEditorNodeArea::render()
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, gridBackgroundColor);

	ImVec2 currentPosition = ImVec2(ImGui::GetCurrentWindow()->Pos.x + areaPosition.x, ImGui::GetCurrentWindow()->Pos.y + areaPosition.y);
	ImGui::SetNextWindowPos(currentPosition);
	ImGui::BeginChild("Nodes area", getAreaSize(), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

	currentDrawList = ImGui::GetWindowDrawList();

	renderGrid(currentPosition);

	currentDrawList->ChannelsSplit(3);

	// Connection should be on node's top layer.
	currentDrawList->ChannelsSetCurrent(0);
	for (size_t i = 0; i < connections.size(); i++)
	{
		renderConnection(connections[i]);
	}

	rightMouseClickWasOnNode = false;
	nodeWasDragged = false;
	nodeHoveredID = -1;
	for (int i = 0; i < int(nodes.size()); i++)
	{
		if (nodes[i]->shouldBeDestroyed)
		{
			deleteNode(nodes[i]);
			i--;
			break;
		}

		renderNode(nodes[i]);
	}

	if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0) && !nodeWasDragged)
	{
		renderOffset.x += ImGui::GetIO().MouseDelta.x;
		renderOffset.y += ImGui::GetIO().MouseDelta.y;

		if (renderOffset.x > 10000.0f)
			renderOffset.x = 10000.0f;

		if (renderOffset.x < -10000.0f)
			renderOffset.x = -10000.0f;

		if (renderOffset.y > 10000.0f)
			renderOffset.y = 10000.0f;

		if (renderOffset.y < -10000.0f)
			renderOffset.y = -10000.0f;
	}

	// ************************* RENDER CONTEXT MENU *************************
	bool openContextMenu = false;
	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(1) && !rightMouseClickWasOnNode)
		openContextMenu = true;

	if (openContextMenu && mainContextMenuFunc != nullptr)
		ImGui::OpenPopup("##main_context_menu");

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
	if (ImGui::BeginPopup("##main_context_menu"))
	{
		if (mainContextMenuFunc != nullptr)
			mainContextMenuFunc();

		ImGui::EndPopup();
	}

	ImGui::PopStyleVar();
	// ************************* RENDER CONTEXT MENU END *************************

	currentDrawList->ChannelsMerge();
	currentDrawList = nullptr;

	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);
}

ImVec2 FEEditorNodeArea::getAreaSize()
{
	return areaSize;
}

void FEEditorNodeArea::setAreaSize(ImVec2 newValue)
{
	if (newValue.x < 1 || newValue.y < 1)
		return;

	areaSize = newValue;
}

ImVec2 FEEditorNodeArea::getAreaPosition()
{
	return areaPosition;
}

void FEEditorNodeArea::setAreaPosition(ImVec2 newValue)
{
	if (newValue.x < 0 || newValue.y < 0)
		return;

	areaPosition = newValue;
}

void FEEditorNodeArea::drawHermiteLine(ImVec2 p1, ImVec2 p2, int steps)
{
	ImVec2 t1 = ImVec2(80.0f, 0.0f);
	ImVec2 t2 = ImVec2(80.0f, 0.0f);

	for (int step = 0; step <= steps; step++)
	{
		float t = (float)step / (float)steps;
		float h1 = +2 * t * t * t - 3 * t * t + 1.0f;
		float h2 = -2 * t * t * t + 3 * t * t;
		float h3 = t * t * t - 2 * t * t + t;
		float h4 = t * t * t - t * t;
		currentDrawList->PathLineTo(ImVec2(h1 * p1.x + h2 * p2.x + h3 * t1.x + h4 * t2.x, h1 * p1.y + h2 * p2.y + h3 * t1.y + h4 * t2.y));
	}

	currentDrawList->PathStroke(lineColor, false, 3.0f);
}

void FEEditorNodeArea::renderConnection(FEEditorNodeConnection* connection)
{
	if (connection->out == nullptr || connection->in == nullptr)
		return;

	drawHermiteLine(socketToPosition(connection->out), socketToPosition(connection->in), 12);
}

ImVec2 FEEditorNodeArea::socketToPosition(FEEditorNodeSocket* socket)
{
	bool input = FEEditorNode::isSocketTypeIn(socket->type);
	float socketX = input ? socket->parent->leftTop.x + 15.0f : socket->parent->rightBottom.x - 15.0f;

	float heightForSockets = socket->parent->getSize().y - NODE_TITLE_HEIGHT;
	float socketSpacing = heightForSockets / (input ? socket->parent->input.size() : socket->parent->output.size());

	int socketIndex = -1;
	if (input)
	{
		for (size_t i = 0; i < socket->parent->input.size(); i++)
		{
			if (socket->parent->input[i] == socket)
			{
				socketIndex = i;
				break;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < socket->parent->output.size(); i++)
		{
			if (socket->parent->output[i] == socket)
			{
				socketIndex = i;
				break;
			}
		}
	}

	float socketY = socket->parent->leftTop.y + NODE_TITLE_HEIGHT + socketSpacing * (socketIndex + 1) - socketSpacing / 2.0f;

	return ImVec2(socketX, socketY);
}

void FEEditorNodeArea::update()
{
	// Should we connect two sockets ?
	socketLookingForConnection = nullptr;
	socketHovered = nullptr;

	for (size_t i = 0; i < nodes.size(); i++)
	{
		FEEditorNodeSocket* tempResult = nodes[i]->isAnySocketLookingForConnection();
		if (tempResult != nullptr)
			socketLookingForConnection = tempResult;

		tempResult = nodes[i]->isAnySocketHovered();
		if (tempResult != nullptr)
			socketHovered = tempResult;
	}

	if (socketLookingForConnection != nullptr &&
		socketHovered != nullptr &&
		ImGui::GetIO().MouseReleased[0])
	{
		char* msg = nullptr;
		if (socketHovered->getParent()->canConnect(socketHovered, socketLookingForConnection, &msg))
		{
			socketLookingForConnection->connections.push_back(socketHovered);
			socketHovered->connections.push_back(socketLookingForConnection);

			connections.push_back(new FEEditorNodeConnection(socketLookingForConnection, socketHovered));

			socketLookingForConnection->parent->socketEvent(socketLookingForConnection, socketHovered, FE_EDITOR_NODE_SOCKET_CONNECTED);
			socketHovered->parent->socketEvent(socketHovered, socketLookingForConnection, FE_EDITOR_NODE_SOCKET_CONNECTED);
		}
	}

	// Should we disconnect sockets
	if (socketHovered != nullptr && socketHovered->connections.size() > 0 && ImGui::GetIO().MouseReleased[1])
	{
		std::vector<FEEditorNodeConnection*> impactedConnections = getAllConnections(socketHovered);
		for (size_t i = 0; i < impactedConnections.size(); i++)
		{
			disconnect(impactedConnections[i]);
		}
	}

	render();
}

std::vector<FEEditorNodeConnection*> FEEditorNodeArea::getAllConnections(FEEditorNodeSocket* socket)
{
	std::vector<FEEditorNodeConnection*> result;

	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i]->in == socket || connections[i]->out == socket)
			result.push_back(connections[i]);
	}

	return result;
}

void FEEditorNodeArea::disconnect(FEEditorNodeConnection*& connection)
{
	for (int i = 0; i < int(connection->in->connections.size()); i++)
	{
		if (connection->in->connections[i] == connection->out)
		{
			connection->in->connections.erase(connection->in->connections.begin() + i, connection->in->connections.begin() + i + 1);
			connection->in->parent->socketEvent(connection->in, connection->out, clearing ? FE_EDITOR_NODE_SOCKET_DESTRUCTION : FE_EDITOR_NODE_SOCKET_DISCONNECTED);
			i--;
		}
	}

	for (int i = 0; i < int(connection->out->connections.size()); i++)
	{
		if (connection->out->connections[i] == connection->in)
		{
			connection->out->connections.erase(connection->out->connections.begin() + i, connection->out->connections.begin() + i + 1);
			i--;
		}
	}

	for (size_t i = 0; i < connections.size(); i++)
	{
		if (connections[i] == connection)
		{
			connections.erase(connections.begin() + i, connections.begin() + i + 1);
			delete connection;
			connection = nullptr;
			return;
		}
	}
}

void FEEditorNodeArea::deleteNode(FEEditorNode* node)
{
	for (size_t i = 0; i < nodes.size(); i++)
	{
		if (nodes[i] == node)
		{
			for (size_t j = 0; j < nodes[i]->input.size(); j++)
			{
				auto connections = getAllConnections(nodes[i]->input[j]);
				for (size_t p = 0; p < connections.size(); p++)
				{
					disconnect(connections[p]);
				}
			}

			for (size_t j = 0; j < nodes[i]->output.size(); j++)
			{
				auto connections = getAllConnections(nodes[i]->output[j]);
				for (size_t p = 0; p < connections.size(); p++)
				{
					disconnect(connections[p]);
				}
			}

			delete nodes[i];
			nodes.erase(nodes.begin() + i, nodes.begin() + i + 1);

			break;
		}
	}
}

void FEEditorNodeArea::setMainContextMenuFunc(void(*func)())
{
	mainContextMenuFunc = func;
}

void FEEditorNodeArea::clear()
{
	clearing = true;

	for (int i = 0; i < int(nodes.size()); i++)
	{
		deleteNode(nodes[i]);
		i--;
	}

	clearing = false;
}

void FEEditorNodeArea::propagateUpdateToConnectedNodes(FEEditorNode* callerNode)
{
	if (callerNode == nullptr)
		return;

	for (size_t i = 0; i < callerNode->input.size(); i++)
	{
		auto connections = getAllConnections(callerNode->input[i]);
		for (size_t j = 0; j < connections.size(); j++)
		{
			connections[j]->in->getParent()->socketEvent(connections[j]->in, connections[j]->out, FE_EDITOR_NODE_SOCKET_UPDATE);
		}
	}

	for (size_t i = 0; i < callerNode->output.size(); i++)
	{
		auto connections = getAllConnections(callerNode->output[i]);
		for (size_t j = 0; j < connections.size(); j++)
		{
			connections[j]->in->getParent()->socketEvent(connections[j]->in, connections[j]->out, FE_EDITOR_NODE_SOCKET_UPDATE);
		}
	}
}

ImVec2 FEEditorNodeArea::getAreaRenderOffset()
{
	return renderOffset;
}

//bool FEEditorNodeArea::tryToConnect(FEEditorNodeSocket* outSocket, FEEditorNodeSocket* inSocket)
//{
//	char* msg = nullptr;
//	bool result = inSocket->getParent()->canConnect(inSocket, outSocket, &msg);
//	
//	if (result)
//	{
//		outSocket->connections.push_back(inSocket);
//		inSocket->connections.push_back(outSocket);
//
//		connections.push_back(new FEEditorNodeConnection(outSocket, inSocket));
//
//		outSocket->getParent()->socketEvent(outSocket, inSocket, FE_EDITOR_NODE_SOCKET_CONNECTED);
//		inSocket->getParent()->socketEvent(inSocket, outSocket, FE_EDITOR_NODE_SOCKET_CONNECTED);
//	}
//
//	return result;
//}

bool FEEditorNodeArea::tryToConnect(FEEditorNode* outNode, size_t outNodeSocketIndex, FEEditorNode* inNode, size_t inNodeSocketIndex)
{
	if (outNode->output.size() <= outNodeSocketIndex)
		return false;

	if (inNode->input.size() <= inNodeSocketIndex)
		return false;

	FEEditorNodeSocket* outSocket = outNode->output[outNodeSocketIndex];
	FEEditorNodeSocket* inSocket = inNode->input[inNodeSocketIndex];

	char* msg = nullptr;
	bool result = inSocket->getParent()->canConnect(inSocket, outSocket, &msg);

	if (result)
	{
		outSocket->connections.push_back(inSocket);
		inSocket->connections.push_back(outSocket);

		connections.push_back(new FEEditorNodeConnection(outSocket, inSocket));

		outSocket->getParent()->socketEvent(outSocket, inSocket, FE_EDITOR_NODE_SOCKET_CONNECTED);
		inSocket->getParent()->socketEvent(inSocket, outSocket, FE_EDITOR_NODE_SOCKET_CONNECTED);
	}

	return result;
}