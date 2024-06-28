#pragma once
#include "../Core/FEObject.h"
#include "../ResourceManager/FEResourceManager.h"

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
		FENaiveSceneGraphNode* GetChildByOldEntityID(std::string OldEntityID);
		std::vector<FENaiveSceneGraphNode*> GetChildByName(std::string Name);
		size_t GetImediateChildrenCount();
		size_t GetRecursiveChildCount();

		std::vector<FENaiveSceneGraphNode*> GetChildren();
		std::vector<FENaiveSceneGraphNode*> GetRecursiveChildren();

		FEObject* GetOldStyleEntity();

		Json::Value ToJson();
		void FromJson(Json::Value Root);
	private:
		FENaiveSceneGraphNode(std::string Name = "Unnamed NaiveSceneNode");
		~FENaiveSceneGraphNode();

		FENaiveSceneGraphNode* Parent = nullptr;
		std::vector<FENaiveSceneGraphNode*> Children;
		FEObject* OldStyleEntity = nullptr;

		void ApplyTransformHierarchy(FENaiveSceneGraphNode* NodeToWorkOn);
		void ReverseTransformHierarchy(FENaiveSceneGraphNode* NodeToWorkOn);

		std::vector<FENaiveSceneGraphNode*> GetAllNodesInternal();
	};
}