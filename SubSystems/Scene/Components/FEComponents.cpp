#include "FEComponents.h"
#include "../FEScene.h"
using namespace FocalEngine;

FEComponentsTools* FEComponentsTools::Instance = nullptr;
FEComponentsTools::FEComponentsTools()
{
	FEComponentTypeInfo TagComponentInfo("Tag", typeid(FETagComponent));
	FunctionsToGetEntityIDListWith[TagComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FETagComponent>(); };
	TagComponentInfo.IncompatibleWith.push_back({ TagComponentInfo });
	TagComponentInfo.ToJson = [](FEEntity* ParentEntity) -> Json::Value {
		Json::Value Root;
		FETagComponent& CurrentComponent = ParentEntity->GetComponent<FETagComponent>();
		Root["Tag"] = CurrentComponent.GetTag();
		return Root;
	};
	TagComponentInfo.FromJson = [](FEEntity* ParentEntity, Json::Value Root) {
		FETagComponent& CurrentComponent = ParentEntity->GetComponent<FETagComponent>();
		CurrentComponent.SetTag(Root["Tag"].asString());
	};
	TagComponentInfo.LoadingPriority = 0;
	ComponentIDToInfo[entt::type_id<FETagComponent>().hash()] = TagComponentInfo;

	FEComponentTypeInfo TransformComponentInfo("Transform", typeid(FETransformComponent));
	FunctionsToGetEntityIDListWith[TransformComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FETransformComponent>(); };
	TransformComponentInfo.IncompatibleWith.push_back({ TransformComponentInfo });
	TransformComponentInfo.LoadingPriority = 1;
	ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()] = TransformComponentInfo;

	FEComponentTypeInfo CameraComponentInfo("Camera", typeid(FECameraComponent));
	FunctionsToGetEntityIDListWith[CameraComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FECameraComponent>(); };
	CameraComponentInfo.IncompatibleWith.push_back({ CameraComponentInfo });
	ComponentIDToInfo[entt::type_id<FECameraComponent>().hash()] = CameraComponentInfo;

	FEComponentTypeInfo LightComponentInfo("Light", typeid(FELightComponent));
	FunctionsToGetEntityIDListWith[LightComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FELightComponent>(); };
	LightComponentInfo.IncompatibleWith.push_back({ LightComponentInfo });
	ComponentIDToInfo[entt::type_id<FELightComponent>().hash()] = LightComponentInfo;

	FEComponentTypeInfo GameModelComponentInfo("Game Model", typeid(FEGameModelComponent));
	FunctionsToGetEntityIDListWith[GameModelComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FEGameModelComponent>(); };
	GameModelComponentInfo.IncompatibleWith.push_back({ GameModelComponentInfo });
	GameModelComponentInfo.ToJson = [](FEEntity* ParentEntity) -> Json::Value {
		Json::Value Root;
		FEGameModelComponent& CurrentComponent = ParentEntity->GetComponent<FEGameModelComponent>();
		Root["ModelID"] = CurrentComponent.GetGameModel()->GetObjectID();

		Root["bVisible"] = CurrentComponent.IsVisible();
		Root["bCastShadows"] = CurrentComponent.IsCastShadows();
		Root["bReceiveShadows"] = CurrentComponent.IsReceivingShadows();
		Root["bUniformLighting"] = CurrentComponent.IsUniformLighting();
		Root["bApplyPostprocess"] = CurrentComponent.IsPostprocessApplied();
		Root["bWireframeMode"] = CurrentComponent.IsWireframeMode();

		return Root;
	};
	GameModelComponentInfo.FromJson = [](FEEntity* ParentEntity, Json::Value Root) {
		ParentEntity->AddComponent<FEGameModelComponent>(RESOURCE_MANAGER.GetGameModel(Root["ModelID"].asString()));
		FEGameModelComponent& CurrentComponent = ParentEntity->GetComponent<FEGameModelComponent>();

		CurrentComponent.SetVisibility(Root["bVisible"].asBool());
		CurrentComponent.SetCastShadows(Root["bCastShadows"].asBool());
		CurrentComponent.SetReceivingShadows(Root["bReceiveShadows"].asBool());
		CurrentComponent.SetUniformLighting(Root["bUniformLighting"].asBool());
		CurrentComponent.SetIsPostprocessApplied(Root["bApplyPostprocess"].asBool());
		CurrentComponent.SetWireframeMode(Root["bWireframeMode"].asBool());
	};
	GameModelComponentInfo.LoadingPriority = 2;
	ComponentIDToInfo[entt::type_id<FEGameModelComponent>().hash()] = GameModelComponentInfo;

	FEComponentTypeInfo InstancedComponentInfo("Instanced", typeid(FEInstancedComponent));
	FunctionsToGetEntityIDListWith[InstancedComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FEInstancedComponent>(); };
	InstancedComponentInfo.IncompatibleWith.push_back({ InstancedComponentInfo });
	ComponentIDToInfo[entt::type_id<FEInstancedComponent>().hash()] = InstancedComponentInfo;

	FEComponentTypeInfo TerrainComponentInfo("Terrain", typeid(FETerrainComponent));
	FunctionsToGetEntityIDListWith[TerrainComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FETerrainComponent>(); };
	TerrainComponentInfo.IncompatibleWith.push_back({ TerrainComponentInfo });
	ComponentIDToInfo[entt::type_id<FETerrainComponent>().hash()] = TerrainComponentInfo;

	FEComponentTypeInfo SkyDomeComponentInfo("SkyDome", typeid(FESkyDomeComponent));
	FunctionsToGetEntityIDListWith[SkyDomeComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FESkyDomeComponent>(); };
	SkyDomeComponentInfo.IncompatibleWith.push_back({ SkyDomeComponentInfo });
	SkyDomeComponentInfo.IncompatibleWith.push_back({ GameModelComponentInfo });
	ComponentIDToInfo[entt::type_id<FESkyDomeComponent>().hash()] = SkyDomeComponentInfo;

	FEComponentTypeInfo PrefabInstanceComponentInfo("Prefab Instance", typeid(FEPrefabInstanceComponent));
	FunctionsToGetEntityIDListWith[PrefabInstanceComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FEPrefabInstanceComponent>(); };
	PrefabInstanceComponentInfo.IncompatibleWith.push_back({ PrefabInstanceComponentInfo });
	PrefabInstanceComponentInfo.LoadingPriority = 2;
	ComponentIDToInfo[entt::type_id<FEPrefabInstanceComponent>().hash()] = PrefabInstanceComponentInfo;
}

std::vector<FEComponentTypeInfo> FEComponentsTools::GetComponentInfoList()
{
	std::vector<FEComponentTypeInfo> Result;
	for (const auto& Component : ComponentIDToInfo)
		Result.push_back(Component.second);

	return Result;
}

std::vector<std::string> FEComponentsTools::GetEntityIDListWithComponent(FEScene* CurrentScene, const FEComponentTypeInfo& ComponentInfo)
{
	if (FunctionsToGetEntityIDListWith.find(ComponentInfo.Type) == FunctionsToGetEntityIDListWith.end())
	{
		LOG.Add("Function to get entity id list with component not found in FEComponentsTools::GetEntityIDListWithComponent", "FE_LOG_ECS", FE_LOG_ERROR);
		return {};
	}

	return FunctionsToGetEntityIDListWith[ComponentInfo.Type](CurrentScene);
}

FEComponentTypeInfo* FEComponentsTools::GetComponentInfoByName(std::string Name)
{
	for (auto& Component : ComponentIDToInfo)
	{
		if (Component.second.Name == Name)
			return &Component.second;
	}

	return nullptr;
}

bool FEComponentTypeInfo::IsCompatible(FEEntity* ProspectParentEntity, std::string* ErrorMessage)
{
	std::string LocalErrorMessage;
	if (ErrorMessage != nullptr)
		*ErrorMessage = "";

	if (ProspectParentEntity == nullptr)
	{
		LOG.Add("ProspectParentEntity is nullptr in FEComponentTypeInfo::IsCompatible", "FE_LOG_ECS", FE_LOG_ERROR);
		if (ErrorMessage != nullptr)
			*ErrorMessage = "ProspectParentEntity is nullptr";

		return false;
	}

	if (ProspectParentEntity->GetParentScene() == nullptr)
	{
		LOG.Add("ProspectParentEntity parent scene is nullptr in FEComponentTypeInfo::IsCompatible", "FE_LOG_ECS", FE_LOG_ERROR);
		if (ErrorMessage != nullptr)
			*ErrorMessage = "ProspectParentEntity parent scene is nullptr";

		return false;
	}

	FEScene* ParentScene = ProspectParentEntity->GetParentScene();
	if (MaxSceneComponentCount != -1)
	{
		size_t CurrentComponentCount = COMPONENTS_TOOL.GetEntityIDListWithComponent(ParentScene, *this).size();
		if (CurrentComponentCount >= MaxSceneComponentCount)
		{
			LOG.Add("Max component count reached in FEComponentTypeInfo::IsCompatible", "FE_LOG_ECS", FE_LOG_WARNING);

			if (ErrorMessage != nullptr)
				*ErrorMessage = "Max component count reached";

			return false;
		}
	}

	std::vector<FEComponentTypeInfo> CurrentlyExistingComponents = ProspectParentEntity->GetComponentsInfoList();

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
FEComponentTypeInfo::FEComponentTypeInfo(const std::string& Name, const std::type_info& Type)
{
	this->Name = Name;
	this->Type = &Type;
}