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
		void AddChild(FENaiveSceneEntity* Child);
		void RemoveChild(FENaiveSceneEntity* Child);
		FENaiveSceneEntity* GetChild(std::string ID);
		std::vector<FENaiveSceneEntity*> GetChildByName(std::string Name);

		std::vector<FENaiveSceneEntity*> GetChildren();

		FEObject* GetOldStyleEntity();
	private:
		FENaiveSceneEntity(std::string Name = "UnNamedNaiveSceneEntity");
		~FENaiveSceneEntity();

		std::vector<FENaiveSceneEntity*> Children;
		FEObject* OldStyleEntity = nullptr;
	};
}