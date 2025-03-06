#pragma once

#include "../ResourceManager/FEResourceManager.h"
#include "FEEntity.h"

namespace FocalEngine
{
	class FENaiveSceneGraphNode : public FEObject
	{
		friend class FENaiveSceneGraph;
		friend class FEScene;
	public:
		FENaiveSceneGraphNode* GetParent();

		void AddChild(FENaiveSceneGraphNode* Child, bool bPreserveWorldTransform = true);
		void DetachChild(FENaiveSceneGraphNode* Child, bool bPreserveWorldTransform = true);
		FENaiveSceneGraphNode* GetChild(std::string ID);
		FENaiveSceneGraphNode* GetChildByEntityID(std::string EntityID);
		std::vector<FENaiveSceneGraphNode*> GetChildByName(std::string Name);
		size_t GetImediateChildrenCount();
		size_t GetRecursiveChildCount();

		std::vector<FENaiveSceneGraphNode*> GetChildren();
		std::vector<FENaiveSceneGraphNode*> GetRecursiveChildren();

		FEEntity* GetEntity();

		Json::Value ToJson(std::function<bool(FEEntity*)> ChildFilter = nullptr);
		void FromJson(Json::Value Root);
	private:
		FENaiveSceneGraphNode(std::string Name = "Unnamed NaiveSceneNode");
		~FENaiveSceneGraphNode();

		FENaiveSceneGraphNode* Parent = nullptr;
		std::vector<FENaiveSceneGraphNode*> Children;
		FEEntity* Entity = nullptr;

		void ApplyTransformHierarchy(FENaiveSceneGraphNode* NodeToWorkOn);
		void ReverseTransformHierarchy(FENaiveSceneGraphNode* NodeToWorkOn);

		std::vector<FENaiveSceneGraphNode*> GetAllNodesInternal();
	};
}