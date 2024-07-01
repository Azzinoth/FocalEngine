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

		std::string AddNode(FENewEntity* Entity, bool bPreserveWorldTransform = true);
		bool MoveNode(std::string NodeID, std::string NewParentID, bool bPreserveWorldTransform = true);
		void DetachNode(FENaiveSceneGraphNode* NodeToDetach, bool bPreserveWorldTransform = true);
		void DeleteNode(FENaiveSceneGraphNode* NodeToDelete);
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
		bool bClearing = false;
		FENaiveSceneGraphNode* Root;

		void AddNode(FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform = true);
		void AddNode(FENaiveSceneGraphNode* Parent, FENaiveSceneGraphNode* NodeToAdd, bool bPreserveWorldTransform = true);

        bool HasCycleInternal(FENaiveSceneGraphNode* NodeToCheck,
							  std::unordered_set<FENaiveSceneGraphNode*>& Visited,
							  std::unordered_set<FENaiveSceneGraphNode*>& RecursionStack);

		std::vector<FENaiveSceneGraphNode*> GetAllNodes();

		//FENaiveSceneGraphNode* GetNodeByOldEntityID(std::string OldEntityID);
		FENaiveSceneGraphNode* GetNodeByNewEntityID(std::string NewEntityID);

		void InitializeRoot(FENaiveSceneGraphNode* NewRoot);
	};
}