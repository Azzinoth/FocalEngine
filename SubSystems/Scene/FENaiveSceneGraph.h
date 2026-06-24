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
		FEScene* GetParentScene() const;

		std::string AddNode(FEEntity* Entity, bool bPreserveWorldTransform = true);
		bool MoveNode(std::string NodeID, std::string NewParentID, bool bPreserveWorldTransform = true);
		void DetachNode(FENaiveSceneGraphNode* NodeToDetach, bool bPreserveWorldTransform = true);
		void DeleteNode(FENaiveSceneGraphNode* NodeToDelete);
		FENaiveSceneGraphNode* DuplicateNode(std::string NodeIDToDuplicate, std::string NewParentID, bool bAddCopyInName = true, std::function<bool(FEEntity*)> Filter = nullptr);
		FENaiveSceneGraphNode* DuplicateNode(FENaiveSceneGraphNode* NodeToDuplicate, FENaiveSceneGraphNode* NewParent, bool bAddCopyInName = true, std::function<bool(FEEntity*)> Filter = nullptr);

		FENaiveSceneGraphNode* ImportNode(FENaiveSceneGraphNode* NodeFromDifferentSceneGraph, FENaiveSceneGraphNode* TargetParent = nullptr, std::function<bool(FEEntity*)> Filter = nullptr);

		size_t GetNodeCount();

		bool IsDescendant(FENaiveSceneGraphNode* PotentialAncestor, FENaiveSceneGraphNode* PotentialDescendant);
        bool HasCycle(FENaiveSceneGraphNode* NodeToCheck);

		FENaiveSceneGraphNode* GetNodeByID(std::string ID);
		FENaiveSceneGraphNode* GetNodeByEntityID(std::string EntityID);
		std::vector<FENaiveSceneGraphNode*> GetNodeByName(std::string Name);

		// Walks the ancestor chain upward and returns the first node whose entity has T.
		template<typename T>
		FENaiveSceneGraphNode* GetFirstRecursiveParentNodeWithComponent(FENaiveSceneGraphNode* Node);

		// Returns the first immediate child whose entity has T (depth 1 only).
		template<typename T>
		FENaiveSceneGraphNode* GetFirstImmediateChildNodeWithComponent(FENaiveSceneGraphNode* Node);

		// Walks the descendant subtree and returns the closest node whose entity has T.
		template<typename T>
		FENaiveSceneGraphNode* GetFirstRecursiveChildNodeWithComponent(FENaiveSceneGraphNode* Node);

		void Clear();

		Json::Value ToJson(std::function<bool(FEEntity*)> Filter = nullptr);
		void FromJson(Json::Value Root);

		FEAABB GetNodeAABB(FEAABB& CumulativeAABB, FENaiveSceneGraphNode* TargetNode = nullptr, std::function<bool(FEEntity*)> Filter = nullptr);
	private:
		FEScene* ParentScene = nullptr;

		bool bClearing = false;
		FENaiveSceneGraphNode* Root = nullptr;

		void AddNodeInternal(FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform = true);
		void AddNodeInternal(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform = true);

        bool HasCycleInternal(FENaiveSceneGraphNode* NodeToCheck,
							  std::unordered_set<FENaiveSceneGraphNode*>& Visited,
							  std::unordered_set<FENaiveSceneGraphNode*>& RecursionStack);

		std::vector<FENaiveSceneGraphNode*> GetAllNodes();

		void Initialize(FEScene* Scene);

		bool DuplicateNodeInternal(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToDuplicate, bool bAddCopyInName = true, std::function<bool(FEEntity*)> Filter = nullptr);

		std::vector<FENaiveSceneGraphNode*> GetNodeByNameInternal(std::string Name, FENaiveSceneGraphNode* CurrentNode, std::vector<FENaiveSceneGraphNode*> CurrentResult);
	};
#include "FENaiveSceneGraph.inl"
}