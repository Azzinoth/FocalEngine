#include "FEComponents.h"
using namespace FocalEngine;

FEComponentsTools* FEComponentsTools::Instance = nullptr;
FEComponentsTools::FEComponentsTools()
{
	ComponentIDToInfo[entt::type_id<FETagComponent>().hash()] = FEComponentTypeInfo("Tag", typeid(FETagComponent));
	ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()] = FEComponentTypeInfo("Transform", typeid(FETransformComponent));
	ComponentIDToInfo[entt::type_id<FELightComponent>().hash()] = FEComponentTypeInfo("Light", typeid(FELightComponent));
	ComponentIDToInfo[entt::type_id<FEGameModelComponent>().hash()] = FEComponentTypeInfo("Game Model", typeid(FEGameModelComponent));
	ComponentIDToInfo[entt::type_id<FEInstancedComponent>().hash()] = FEComponentTypeInfo("Instanced", typeid(FEInstancedComponent));
	ComponentIDToInfo[entt::type_id<FETerrainComponent>().hash()] = FEComponentTypeInfo("Terrain", typeid(FETerrainComponent));
	ComponentIDToInfo[entt::type_id<FESkyDomeComponent>().hash()] = FEComponentTypeInfo("SkyDome", typeid(FESkyDomeComponent));


	ComponentIDToInfo[entt::type_id<FETagComponent>().hash()].IncompatibleWith.push_back({ ComponentIDToInfo[entt::type_id<FETagComponent>().hash()] });
	ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()].IncompatibleWith.push_back({ ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()] });
	ComponentIDToInfo[entt::type_id<FELightComponent>().hash()].IncompatibleWith.push_back({ ComponentIDToInfo[entt::type_id<FELightComponent>().hash()] });
	ComponentIDToInfo[entt::type_id<FEGameModelComponent>().hash()].IncompatibleWith.push_back({ ComponentIDToInfo[entt::type_id<FEGameModelComponent>().hash()] });
	ComponentIDToInfo[entt::type_id<FEInstancedComponent>().hash()].IncompatibleWith.push_back({ ComponentIDToInfo[entt::type_id<FEInstancedComponent>().hash()] });
	ComponentIDToInfo[entt::type_id<FETerrainComponent>().hash()].IncompatibleWith.push_back({ ComponentIDToInfo[entt::type_id<FETerrainComponent>().hash()] });
	ComponentIDToInfo[entt::type_id<FESkyDomeComponent>().hash()].IncompatibleWith.push_back({ ComponentIDToInfo[entt::type_id<FESkyDomeComponent>().hash()] });
	//ComponentIDToInfo[entt::type_id<FELightComponent>().hash()].RequiredComponents.push_back({ ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()] });
	//ComponentIDToInfo[entt::type_id<FELightComponent>().hash()].RequiredComponents.push_back({ ComponentIDToInfo[entt::type_id<FETerrainComponent>().hash()] });
	//ComponentIDToInfo[entt::type_id<FELightComponent>().hash()].IncompatibleCombinations.push_back({ { ComponentIDToInfo[entt::type_id<FETagComponent>().hash()] }, { ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()] } });
	//ComponentIDToInfo[entt::type_id<FELightComponent>().hash()].IncompatibleWith.push_back({ ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()] });
}

std::vector<FEComponentTypeInfo> FEComponentsTools::GetComponentInfoList()
{
	std::vector<FEComponentTypeInfo> Result;
	for (const auto& Component : ComponentIDToInfo)
		Result.push_back(Component.second);

	return Result;
}

bool FEComponentTypeInfo::IsCompatible(std::vector<FEComponentTypeInfo>& CurrentlyExistingComponents, std::string* ErrorMessage)
{
	std::string LocalErrorMessage;
	if (ErrorMessage != nullptr)
		*ErrorMessage = "";

	// Check if any of the existing components are incompatible
	for (const auto& ExistingComponent : CurrentlyExistingComponents)
	{
		if (std::find(IncompatibleWith.begin(), IncompatibleWith.end(), ExistingComponent) != IncompatibleWith.end())
		{
			LocalErrorMessage = "Incompatible with " + ExistingComponent.Name;
			if (ErrorMessage != nullptr)
				*ErrorMessage = LocalErrorMessage;

			return false;
		}
	}

	// Check for incompatible combinations
	for (const auto& Combination : IncompatibleCombinations)
	{
		bool bAllPresent = true;
		for (const auto& CurrentIncompatiableComponent : Combination)
		{
			if (std::find(CurrentlyExistingComponents.begin(), CurrentlyExistingComponents.end(), CurrentIncompatiableComponent) == CurrentlyExistingComponents.end())
			{
				bAllPresent = false;
				break;
			}
		}

		if (bAllPresent)
		{
			LocalErrorMessage = "Incompatible with combination: ";
			for (const auto& CurrentIncompatiableComponent : Combination)
			{
				LocalErrorMessage += CurrentIncompatiableComponent.Name + ", ";
			}

			LocalErrorMessage.erase(LocalErrorMessage.end() - 2, LocalErrorMessage.end());

			if (ErrorMessage != nullptr)
				*ErrorMessage = LocalErrorMessage;

			return false;
		}

	}

	// Check for required components
	for (const auto& RequiredComponent : RequiredComponents)
	{
		if (std::find(CurrentlyExistingComponents.begin(), CurrentlyExistingComponents.end(), RequiredComponent) == CurrentlyExistingComponents.end())
		{
			LocalErrorMessage = "Requires " + RequiredComponent.Name;

			if (ErrorMessage != nullptr)
				*ErrorMessage = LocalErrorMessage;

			return false;
		}
	}

	return true;
}

FEComponentTypeInfo::FEComponentTypeInfo() : Name(), Type(nullptr) {}
FEComponentTypeInfo::FEComponentTypeInfo(const std::string& name, const std::type_info& type)
{
	Name = name;
	Type = &type;
}