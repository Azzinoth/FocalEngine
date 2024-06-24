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

		void AddChild(FENaiveSceneGraphNode* Child);
		void RemoveChild(FENaiveSceneGraphNode* NodeToAdd);
		FENaiveSceneGraphNode* GetChild(std::string ID);
		FENaiveSceneGraphNode* GetChildByOldEntityID(std::string OldEntityID);
		std::vector<FENaiveSceneGraphNode*> GetChildByName(std::string Name);

		std::vector<FENaiveSceneGraphNode*> GetChildren();

		FEObject* GetOldStyleEntity();
	private:
		FENaiveSceneGraphNode(std::string Name = "Unnamed NaiveSceneNode");
		~FENaiveSceneGraphNode();

		FENaiveSceneGraphNode* Parent = nullptr;
		std::vector<FENaiveSceneGraphNode*> Children;
		FEObject* OldStyleEntity = nullptr;
	};
}