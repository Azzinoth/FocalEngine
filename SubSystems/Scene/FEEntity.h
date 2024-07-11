#pragma once

#include "../Renderer/FEFramebuffer.h"
#include "../Renderer/FEPrefab.h"
#include "Components/FEComponents.h"
#include "entt.hpp"

namespace FocalEngine
{
	class FEScene;
	class FEEntity : public FEObject
	{
		friend class FEEntityInstanced;
		friend class FERenderer;
		friend class FEResourceManager;
		friend class FEScene;

		FEEntity(entt::entity AssignedEnTTEntity, FEScene* Scene);
		~FEEntity();
		
		entt::entity EnTTEntity = entt::null;
		FEScene* Scene = nullptr;

		entt::registry& GetRegistry();
	public:
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			if (HasComponent<T>())
			{
				LOG.Add("Component already exists in entity!", "FE_LOG_ECS", FE_LOG_WARNING);
				return GetComponent<T>();
			}

			return GetRegistry().emplace<T>(EnTTEntity, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			if (!HasComponent<T>())
			{
				LOG.Add("Component does not exist in entity!", "FE_LOG_ECS", FE_LOG_ERROR);
				return AddComponent<T>();
			}

			return GetRegistry().get<T>(EnTTEntity);
		}

		template<typename T>
		bool HasComponent()
		{
			return GetRegistry().all_of<T>(EnTTEntity);
		}

		template<typename T>
		void RemoveComponent()
		{
			if (!HasComponent<T>())
			{
				LOG.Add("Component does not exist in entity!", "FE_LOG_ECS", FE_LOG_WARNING);
				return;
			}

			GetRegistry().remove<T>(EnTTEntity);
		}
	};
}