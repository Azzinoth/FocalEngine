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

#include "entt.hpp"

namespace FocalEngine
{
    // Define a struct to hold component type information
    struct FEComponentTypeInfo
    {
        std::string Name;
        const std::type_info* Type;

        std::vector<FEComponentTypeInfo> IncompatibleWith;
        std::vector<FEComponentTypeInfo> RequiredComponents;
        std::vector<std::vector<FEComponentTypeInfo>> IncompatibleCombinations;
        int MaxSceneComponentCount = -1;

        FEComponentTypeInfo();
        FEComponentTypeInfo(const std::string& name, const std::type_info& type);

        bool operator==(const FEComponentTypeInfo& other) const
		{
			return *Type == *other.Type;
		}

        bool IsCompatible(FEEntity* ProspectParentEntity, std::string* ErrorMessage = nullptr);
    };

    class FEComponentsTools
    {
        SINGLETON_PRIVATE_PART(FEComponentsTools)

        friend class FEEntity;

        std::map<entt::id_type, FEComponentTypeInfo> ComponentIDToInfo;
        std::map<const std::type_info*, std::function<std::vector<std::string>(FEScene*)>> FunctionsToGetEntityIDListWith;
    public:
        SINGLETON_PUBLIC_PART(FEComponentsTools)

        std::vector<FEComponentTypeInfo> GetComponentInfoList();
        std::vector<std::string> GetEntityIDListWithComponent(FEScene* CurrentScene, const FEComponentTypeInfo& ComponentInfo);
    };

#define COMPONENTS_TOOL FEComponentsTools::getInstance()
}