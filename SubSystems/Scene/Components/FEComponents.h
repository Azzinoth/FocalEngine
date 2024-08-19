#pragma once
#include "FETagComponent.h"
#include "FETransformComponent.h"
#include "FECameraComponent.h"
#include "FELightComponent.h"
#include "FEGameModelComponent.h"
#include "FEInstancedComponent.h"
#include "FETerrainComponent.h"
#include "FESkyDomeComponent.h"
#include "FEPrefabInstanceComponent.h"
#include "FEVirtualUIComponent.h"
#include "FENativeScriptComponent.h"

#include "entt.hpp"

namespace FocalEngine
{
	enum class FE_LOGIC_OPERATION
	{
		AND,
		OR,
        NOT,
        XOR
	};

    struct FEComponentTypeInfo;
    struct FEComponentConstraint
    {
        FE_LOGIC_OPERATION LogicOperation;
        std::vector<FEComponentTypeInfo> Components;
    };

    // Define a struct to hold component type information
    struct FEComponentTypeInfo
    {
        std::string Name;
        const std::type_info* Type;

        std::vector<FEComponentConstraint> Constraints;
        int MaxSceneComponentCount = -1;
		bool bCanNotBeRemoved = false;
        // That variable defines the priority of loading the component. Relevant for the entities components list.
        // The lower the value, the earlier the component will be loaded.
        int LoadingPriority = INT_MAX;

        FEComponentTypeInfo();
        FEComponentTypeInfo(const std::string& Name, const std::type_info& Type);

        bool operator==(const FEComponentTypeInfo& Other) const
		{
			return *Type == *Other.Type;
		}

        bool CanBeAddedToEntity(FEEntity* PotentialParentEntity, std::string* ErrorMessage = nullptr);
		bool CanBeRemovedFromEntity(FEEntity* ParentEntity, std::string* ErrorMessage = nullptr);

        std::function<void(FEEntity*, FEEntity*)> DuplicateComponent = nullptr;
        std::function<Json::Value(FEEntity*)> ToJson = nullptr;
        std::function<void(FEEntity*, Json::Value)> FromJson = nullptr;

        bool EvaluateConstraints(const std::vector<FEComponentTypeInfo>& ComponentToCheckTowards, std::string* ErrorMessage) const;
        bool EvaluateConstraint(const FEComponentConstraint& Constraint, const std::vector<FEComponentTypeInfo>& ComponentToCheckTowards) const;
    private:
        std::string LogicOperationToString(FE_LOGIC_OPERATION LogicOperation) const;
    };

    class FEComponentsTools
    {
        SINGLETON_PRIVATE_PART(FEComponentsTools)

        friend class FEEntity;

        friend class FETransformSystem;
        friend class FECameraSystem;
        friend class FELightSystem;
        friend class FEInstancedSystem;
        friend class FETerrainSystem;
        friend class FESkyDomeSystem;
        friend class FEPrefabInstanceSystem;
        friend class FEVirtualUISystem;
		friend class FENativeScriptSystem;

        std::map<entt::id_type, FEComponentTypeInfo> ComponentIDToInfo;
        std::map<const std::type_info*, std::function<std::vector<std::string>(FEScene*)>> FunctionsToGetEntityIDListWith;

        template<typename T>
        void RegisterComponentToJsonFunction(std::function<Json::Value(FEEntity*)> Function)
        {
			ComponentIDToInfo[entt::type_id<T>().hash()].ToJson = Function;
		}
        
		template<typename T>
        void RegisterComponentFromJsonFunction(std::function<void(FEEntity*, Json::Value)> Function)
        {
            ComponentIDToInfo[entt::type_id<T>().hash()].FromJson = Function;
        }

        template<typename T>
        void RegisterComponentDuplicateFunction(std::function<void(FEEntity*, FEEntity*)> Function)
		{
			ComponentIDToInfo[entt::type_id<T>().hash()].DuplicateComponent = Function;
		}
    public:
        SINGLETON_PUBLIC_PART(FEComponentsTools)

        std::vector<FEComponentTypeInfo> GetComponentInfoList();
        std::vector<std::string> GetEntityIDListWithComponent(FEScene* CurrentScene, const FEComponentTypeInfo& ComponentInfo);
        FEComponentTypeInfo* GetComponentInfoByName(std::string Name);
		template<typename T>
        FEComponentTypeInfo* GetComponentInfo()
		{
			return &ComponentIDToInfo[entt::type_id<T>().hash()];
		}

        void SortComponentsByLoadingPriority(std::vector<FEComponentTypeInfo>& Components);
        void SortComponentsByLoadingPriority(std::vector<std::string>& ComponentsNames);
    };

#define COMPONENTS_TOOL FEComponentsTools::getInstance()
}