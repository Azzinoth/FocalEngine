#pragma once
#include "FENaiveSceneEntity.h"

namespace FocalEngine
{
	class FENaiveSceneGraph
	{
		friend class FEScene;
	public:
		FENaiveSceneGraph();
		~FENaiveSceneGraph();

		FENaiveSceneEntity* GetRoot() const;

		std::string AddEntity(FEObject* OldStyleEntity);
		bool MoveEntity(std::string EntityID, std::string NewParentID);
		void RemoveEntity(FENaiveSceneEntity* Entity);

		FENaiveSceneEntity* GetEntity(std::string ID);
		FENaiveSceneEntity* GetEntityByOldEntityID(std::string OldEntityID);
		std::vector<FENaiveSceneEntity*> GetEntityByName(std::string Name);

		void Clear();
	private:
		FENaiveSceneEntity* Root;

		void AddEntity(FENaiveSceneEntity* Entity);
		void AddEntity(FENaiveSceneEntity* Parent, FENaiveSceneEntity* Entity);
	};
}