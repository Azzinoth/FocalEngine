#pragma once

#include "FEEditorNode.h"

#define FEEDITOR_NODE_GRID_STEP 15.0f

class FEEditorNodeSystem;

class FEEditorNodeArea
{
	friend FEEditorNodeSystem;

	FEEditorNodeArea();
	~FEEditorNodeArea();

	int nodeIDCounter = 0;
	bool clearing = false;
	ImDrawList* currentDrawList = nullptr;

	std::vector<FEEditorNode*> nodes;
	void renderNode(FEEditorNode* node);
	void renderNodeSockets(FEEditorNode* node);
	void renderNodeSocket(FEEditorNodeSocket* socket);
	bool rightMouseClickWasOnNode = false;
	bool nodeWasDragged = false;

	void drawHermiteLine(ImVec2 p1, ImVec2 p2, int steps);
	void renderConnection(FEEditorNodeConnection* connection);
	ImVec2 socketToPosition(FEEditorNodeSocket* socket);

	std::vector<FEEditorNodeConnection*> connections;
	std::vector<FEEditorNodeConnection*> getAllConnections(FEEditorNodeSocket* socket);
	void disconnect(FEEditorNodeConnection*& connection);

	int nodeHoveredID = -1;
	FEEditorNodeSocket* socketLookingForConnection = nullptr;
	FEEditorNodeSocket* socketHovered = nullptr;

	ImVec2 areaPosition;
	ImVec2 areaSize;
	ImU32 gridBackgroundColor = IM_COL32(32, 32, 32, 255);
	ImVec4 gridLinesColor = ImVec4(53.0f / 255.0f, 53.0f / 255.0f, 53.0f / 255.0f, 0.5f);
	ImVec4 gridBoldLinesColor = ImVec4(27.0f / 255.0f, 27.0f / 255.0f, 27.0f / 255.0f, 1.0f);
	ImColor lineColor = ImColor(200, 200, 200);
	ImVec2 renderOffset = ImVec2(0.0, 0.0);

	void(*mainContextMenuFunc)() = nullptr;
	void render();
	void renderGrid(ImVec2 currentPosition);
	void clear();
public:
	void update();
	
	ImVec2 getAreaPosition();
	void setAreaPosition(ImVec2 newValue);

	ImVec2 getAreaSize();
	void setAreaSize(ImVec2 newValue);

	ImVec2 getAreaRenderOffset();

	void addNode(FEEditorNode* newNode);
	void deleteNode(FEEditorNode* node);

	void setMainContextMenuFunc(void(*func)());

	void propagateUpdateToConnectedNodes(FEEditorNode* callerNode);
	//bool tryToConnect(FEEditorNodeSocket* outSocket, FEEditorNodeSocket* inSocket);
	bool tryToConnect(FEEditorNode* outNode, size_t outNodeSocketIndex, FEEditorNode* inNode, size_t inNodeSocketIndex);
};
