#pragma once
#include "FENaiveSceneGraphNode.h"
#include <unordered_set>

namespace FocalEngine
{
	class FENaiveSceneGraph
	{
		friend class FEScene;

		FENaiveSceneGraph();
		~FENaiveSceneGraph();
	public:
		FENaiveSceneGraphNode* GetRoot() const;

		std::string AddNode(FEEntity* Entity, bool bPreserveWorldTransform = true);
		bool MoveNode(std::string NodeID, std::string NewParentID, bool bPreserveWorldTransform = true);
		void DetachNode(FENaiveSceneGraphNode* NodeToDetach, bool bPreserveWorldTransform = true);
		void DeleteNode(FENaiveSceneGraphNode* NodeToDelete);
		FENaiveSceneGraphNode* DuplicateNode(std::string NodeIDToDuplicate, std::string NewParentID, bool bAddCopyInName = true);
		FENaiveSceneGraphNode* DuplicateNode(FENaiveSceneGraphNode* NodeToDuplicate, FENaiveSceneGraphNode* NewParent, bool bAddCopyInName = true);

		FENaiveSceneGraphNode* ImportNode(FENaiveSceneGraphNode* NodeFromDifferentSceneGraph, FENaiveSceneGraphNode* TargetParent = nullptr);

		size_t GetNodeCount();

		bool IsDescendant(FENaiveSceneGraphNode* PotentialAncestor, FENaiveSceneGraphNode* PotentialDescendant);
        bool HasCycle(FENaiveSceneGraphNode* NodeToCheck);

		FENaiveSceneGraphNode* GetNode(std::string ID);
		FENaiveSceneGraphNode* GetNodeByEntityID(std::string EntityID);
		std::vector<FENaiveSceneGraphNode*> GetNodeByName(std::string Name);

		void Clear();

		Json::Value ToJson();
		void FromJson(Json::Value Root);
	private:
		FEScene* ParentScene = nullptr;

		bool bClearing = false;
		FENaiveSceneGraphNode* Root;

		void AddNodeInternal(FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform = true);
		void AddNodeInternal(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform = true);

        bool HasCycleInternal(FENaiveSceneGraphNode* NodeToCheck,
							  std::unordered_set<FENaiveSceneGraphNode*>& Visited,
							  std::unordered_set<FENaiveSceneGraphNode*>& RecursionStack);

		std::vector<FENaiveSceneGraphNode*> GetAllNodes();

		void Initialize(FEScene* Scene);

		bool DuplicateNodeInternal(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToDuplicate, bool bAddCopyInName = true);
	};
}