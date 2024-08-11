#pragma once

#include "../Renderer/FEFramebuffer.h"
#include "../Renderer/FEPrefab.h"
#include "Components/FEComponents.h"

namespace FocalEngine
{
	class FEScene;
	class FEEntity : public FEObject
	{
		friend class FEEntityInstanced;
		friend class FERenderer;
		friend class FEResourceManager;
		friend class FEScene;
		friend class FENaiveSceneGraphNode;
		friend class FENaiveSceneGraph;

		FEEntity(entt::entity AssignedEnTTEntity, FEScene* Scene);
		~FEEntity();
		
		entt::entity EnTTEntity = entt::null;
		FEScene* ParentScene = nullptr;

		entt::registry& GetRegistry();

		Json::Value ToJson();
		void FromJson(Json::Value Root);

		void SaveComponents(Json::Value& Root);
	public:
		template<typename T, typename... Args>
		bool AddComponent(Args&&... args)
		{
			if (HasComponent<T>())
			{
				LOG.Add("Component already exists in entity!", "FE_LOG_ECS", FE_LOG_WARNING);
				return false;
			}

			if (COMPONENTS_TOOL.ComponentIDToInfo.find(entt::type_id<T>().hash()) == COMPONENTS_TOOL.ComponentIDToInfo.end())
				return false;

			std::string ErrorMessage;
			if (!COMPONENTS_TOOL.ComponentIDToInfo[entt::type_id<T>().hash()].IsCompatible(this, &ErrorMessage))
			{
				LOG.Add("Can not add component: " + ErrorMessage, "FE_LOG_ECS", FE_LOG_ERROR);
				return false;
			}

			GetRegistry().emplace<T>(EnTTEntity, std::forward<Args>(args)...);
			return true;
		}

		template<typename T>
		T& GetComponent()
		{
			if (!HasComponent<T>())
			{
				LOG.Add("Component does not exist in entity!", "FE_LOG_ECS", FE_LOG_ERROR);
				// FIX ME! Should make it return nullptr pointer not throw exception
				throw std::runtime_error("Component does not exist in entity");
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

		std::vector<FEComponentTypeInfo> GetComponentsInfoList()
		{
			std::vector<FEComponentTypeInfo> Result;

			// Loop through all components types
			for (auto&& CurrentComponent : GetRegistry().storage())
			{
				entt::id_type ComponentID = CurrentComponent.first;
				// Add only components that current entity has
				if (auto& Storage = CurrentComponent.second; Storage.contains(EnTTEntity))
				{
					if (COMPONENTS_TOOL.ComponentIDToInfo.find(ComponentID) != COMPONENTS_TOOL.ComponentIDToInfo.end())
					{
						Result.push_back(COMPONENTS_TOOL.ComponentIDToInfo[ComponentID]);
					}
				}
			}

			return Result;
		}

		FEScene* GetParentScene()
		{
			return ParentScene;
		}
	};
}