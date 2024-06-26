#pragma once
#include "FENaiveSceneGraphNode.h"
#include <unordered_set>

namespace FocalEngine
{
	class FENaiveSceneGraph
	{
		friend class FEScene;
	public:
		FENaiveSceneGraph();
		~FENaiveSceneGraph();

		FENaiveSceneGraphNode* GetRoot() const;

		std::string AddNode(FEObject* OldStyleEntity, bool bPreserveWorldTransform = true);
		bool MoveNode(std::string NodeID, std::string NewParentID, bool bPreserveWorldTransform = true);
		void DetachNode(FENaiveSceneGraphNode* NodeToDetach, bool bPreserveWorldTransform = true);
		void DeleteNode(FENaiveSceneGraphNode* NodeToDelete);
		size_t GetNodeCount();

		bool IsDescendant(FENaiveSceneGraphNode* PotentialAncestor, FENaiveSceneGraphNode* PotentialDescendant);
        bool HasCycle(FENaiveSceneGraphNode* NodeToCheck);

		FENaiveSceneGraphNode* GetNode(std::string ID);
		FENaiveSceneGraphNode* GetNodeByOldEntityID(std::string OldEntityID);
		std::vector<FENaiveSceneGraphNode*> GetNodeByName(std::string Name);

		void Clear();

		Json::Value ToJson();
	private:
		bool bClearing = false;
		FENaiveSceneGraphNode* Root;

		void AddNode(FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform = true);
		void AddNode(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform = true);

        bool HasCycleInternal(FENaiveSceneGraphNode* NodeToCheck,
							  std::unordered_set<FENaiveSceneGraphNode*>& Visited,
							  std::unordered_set<FENaiveSceneGraphNode*>& RecursionStack);

		std::vector<FENaiveSceneGraphNode*> GetAllNodes();
	};
}