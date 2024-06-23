#pragma once
#include "../Core/FEObject.h"
#include "../ResourceManager/FEResourceManager.h"

namespace FocalEngine
{
	class FENaiveSceneEntity : public FEObject
	{
		friend class FENaiveSceneGraph;
		friend class FEScene;
	public:

		FENaiveSceneEntity* GetParent();

		void AddChild(FENaiveSceneEntity* Child);
		void RemoveChild(FENaiveSceneEntity* Child);
		FENaiveSceneEntity* GetChild(std::string ID);
		FENaiveSceneEntity* GetChildByOldEntityID(std::string OldEntityID);
		std::vector<FENaiveSceneEntity*> GetChildByName(std::string Name);

		std::vector<FENaiveSceneEntity*> GetChildren();

		FEObject* GetOldStyleEntity();
	private:
		FENaiveSceneEntity(std::string Name = "Unnamed NaiveSceneEntity");
		~FENaiveSceneEntity();

		FENaiveSceneEntity* Parent = nullptr;
		std::vector<FENaiveSceneEntity*> Children;
		FEObject* OldStyleEntity = nullptr;
	};
}