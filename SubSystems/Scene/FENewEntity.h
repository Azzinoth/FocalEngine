#pragma once

#include "../Renderer/FEFramebuffer.h"
#include "../Renderer/FEPrefab.h"

#include "FEScene.h"
#include "entt.hpp"

namespace FocalEngine
{
	struct FETagComponent
	{
		std::string Tag = "";
		FETagComponent() {};
		FETagComponent(std::string TagToSet)
		{
			Tag = TagToSet;
		}
	};

	struct FEGameModelComponent
	{
		FEGameModel* GameModel = nullptr;
		FEGameModelComponent() {};
		FEGameModelComponent(FEGameModel* GameModelToSet)
		{
			GameModel = GameModelToSet;
		}
	};

	class FERenderableComponent
	{
	public:
		
		FERenderableComponent() {};
		FERenderableComponent(FEEntity* OldStyleEntityToInitialize)
		{
			OldStyleEntity = OldStyleEntityToInitialize;
		}

		~FERenderableComponent() {};
	//private:
		FEEntity* OldStyleEntity = nullptr;
	};

	class FENewEntity : public FEObject
	{
		friend class FEEntityInstanced;
		friend class FERenderer;
		friend class FEResourceManager;
		friend class FEScene;

		FENewEntity(entt::entity AssignedEnTTEntity, FEScene* Scene);
		~FENewEntity();
		
		entt::entity EnTTEntity = entt::null;
		FEScene* Scene = nullptr;
	public:
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			/*if (GetName() == "TransformationXGizmoEntity")
			{
				int y = 0;
				y++;
			}*/

			if (HasComponent<T>())
			{
				LOG.Add("Component already exists in entity!", "FE_LOG_ECS", FE_LOG_WARNING);
				return GetComponent<T>();
			}

			return Scene->Registry.emplace<T>(EnTTEntity, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			//if (GetName() == "TransformationXGizmoEntity")
			//{
			//	int y = 0;
			//	y++;
			//}

			if (!HasComponent<T>())
			{
				LOG.Add("Component does not exist in entity!", "FE_LOG_ECS", FE_LOG_ERROR);
				return AddComponent<T>();
			}

			return Scene->Registry.get<T>(EnTTEntity);
		}

		template<typename T>
		bool HasComponent()
		{
			return Scene->Registry.all_of<T>(EnTTEntity);
		}

		template<typename T>
		void RemoveComponent()
		{
			if (!HasComponent<T>())
			{
				LOG.Add("Component does not exist in entity!", "FE_LOG_ECS", FE_LOG_WARNING);
				return;
			}

			Scene->Registry.remove<T>(EnTTEntity);
		}

	protected:
		
	};
}