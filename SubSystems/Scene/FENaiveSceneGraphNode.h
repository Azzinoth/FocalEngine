#pragma once
#include "../Core/FEObject.h"
#include "../ResourceManager/FEResourceManager.h"


namespace FocalEngine
{
	class FEEntity;
	class FENaiveSceneGraphNode : public FEObject
	{
		friend class FENaiveSceneGraph;
		friend class FEScene;
	public:

		FENaiveSceneGraphNode* GetParent();

		void AddChild(FENaiveSceneGraphNode* Child, bool bPreserveWorldTransform = true);
		void DetachChild(FENaiveSceneGraphNode* Child, bool bPreserveWorldTransform = true);
		FENaiveSceneGraphNode* GetChild(std::string ID);
		FENaiveSceneGraphNode* GetChildByNewEntityID(std::string NewEntityID);
		std::vector<FENaiveSceneGraphNode*> GetChildByName(std::string Name);
		size_t GetImediateChildrenCount();
		size_t GetRecursiveChildCount();

		std::vector<FENaiveSceneGraphNode*> GetChildren();
		std::vector<FENaiveSceneGraphNode*> GetRecursiveChildren();

		FEEntity* GetNewStyleEntity();

		Json::Value ToJson();
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