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

		std::string AddNode(FEObject* OldStyleEntity);
		bool MoveNode(std::string NodeID, std::string NewParentID);
		void DetachNode(FENaiveSceneGraphNode* NodeToRemove);
		void DeleteNode(FENaiveSceneGraphNode* NodeToDelete);
		size_t GetNodeCount();

		bool IsDescendant(FENaiveSceneGraphNode* PotentialAncestor, FENaiveSceneGraphNode* PotentialDescendant);
        bool HasCycle(FENaiveSceneGraphNode* NodeToCheck);

		FENaiveSceneGraphNode* GetNode(std::string ID);
		FENaiveSceneGraphNode* GetNodeByOldEntityID(std::string OldEntityID);
		std::vector<FENaiveSceneGraphNode*> GetNodeByName(std::string Name);

		void Clear();
	private:
		bool bClearing = false;
		FENaiveSceneGraphNode* Root;

		void AddNode(FENaiveSceneGraphNode* NodeToAdd);
		void AddNode(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToAdd);

        bool HasCycleInternal(FENaiveSceneGraphNode* NodeToCheck,
							  std::unordered_set<FENaiveSceneGraphNode*>& Visited,
							  std::unordered_set<FENaiveSceneGraphNode*>& RecursionStack);
	};
}