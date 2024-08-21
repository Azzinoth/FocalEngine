#include "FEComponents.h"
#include "../FEScene.h"
using namespace FocalEngine;

FEComponentsTools::FEComponentsTools()
{
	// ************************* TAG COMPONENT *************************
	FEComponentTypeInfo TagComponentInfo("Tag", typeid(FETagComponent));
	FunctionsToGetEntityIDListWith[TagComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FETagComponent>(); };
	TagComponentInfo.bCanNotBeRemoved = true;
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

	TagComponentInfo.DuplicateComponent = [](FEEntity* SourceEntity, FEEntity* TargetEntity) {
		TargetEntity->GetComponent<FETagComponent>() = SourceEntity->GetComponent<FETagComponent>();
	};
	TagComponentInfo.LoadingPriority = 0;
	ComponentIDToInfo[entt::type_id<FETagComponent>().hash()] = TagComponentInfo;
	FEComponentTypeInfo& TagInfo = ComponentIDToInfo[entt::type_id<FETagComponent>().hash()];

	// ************************* TRANSFORM COMPONENT *************************
	FEComponentTypeInfo TransformComponentInfo("Transform", typeid(FETransformComponent));
	FunctionsToGetEntityIDListWith[TransformComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FETransformComponent>(); };
	TransformComponentInfo.bCanNotBeRemoved = true;
	TransformComponentInfo.LoadingPriority = 1;
	ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()] = TransformComponentInfo;
	FEComponentTypeInfo& TransformInfo = ComponentIDToInfo[entt::type_id<FETransformComponent>().hash()];

	// ************************* CAMERA COMPONENT *************************
	FEComponentTypeInfo CameraComponentInfo("Camera", typeid(FECameraComponent));
	FunctionsToGetEntityIDListWith[CameraComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FECameraComponent>(); };
	ComponentIDToInfo[entt::type_id<FECameraComponent>().hash()] = CameraComponentInfo;
	FEComponentTypeInfo& CameraInfo = ComponentIDToInfo[entt::type_id<FECameraComponent>().hash()];

	// ************************* LIGHT COMPONENT *************************
	FEComponentTypeInfo LightComponentInfo("Light", typeid(FELightComponent));
	FunctionsToGetEntityIDListWith[LightComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FELightComponent>(); };
	ComponentIDToInfo[entt::type_id<FELightComponent>().hash()] = LightComponentInfo;
	FEComponentTypeInfo& LightInfo = ComponentIDToInfo[entt::type_id<FELightComponent>().hash()];

	// ************************* GAME MODEL COMPONENT *************************
	FEComponentTypeInfo GameModelComponentInfo("Game Model", typeid(FEGameModelComponent));
	FunctionsToGetEntityIDListWith[GameModelComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FEGameModelComponent>(); };
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

	GameModelComponentInfo.DuplicateComponent = [](FEEntity* SourceEntity, FEEntity* TargetEntity) {
		if (TargetEntity->AddComponent<FEGameModelComponent>())
			TargetEntity->GetComponent<FEGameModelComponent>() = SourceEntity->GetComponent<FEGameModelComponent>();
	};
	GameModelComponentInfo.LoadingPriority = 2;
	ComponentIDToInfo[entt::type_id<FEGameModelComponent>().hash()] = GameModelComponentInfo;
	FEComponentTypeInfo& GameModelInfo = ComponentIDToInfo[entt::type_id<FEGameModelComponent>().hash()];

	// ************************* INSTANCED COMPONENT *************************
	FEComponentTypeInfo InstancedComponentInfo("Instanced", typeid(FEInstancedComponent));
	FunctionsToGetEntityIDListWith[InstancedComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FEInstancedComponent>(); };
	ComponentIDToInfo[entt::type_id<FEInstancedComponent>().hash()] = InstancedComponentInfo;
	FEComponentTypeInfo& InstancedInfo = ComponentIDToInfo[entt::type_id<FEInstancedComponent>().hash()];

	// ************************* TERRAIN COMPONENT *************************
	FEComponentTypeInfo TerrainComponentInfo("Terrain", typeid(FETerrainComponent));
	FunctionsToGetEntityIDListWith[TerrainComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FETerrainComponent>(); };
	ComponentIDToInfo[entt::type_id<FETerrainComponent>().hash()] = TerrainComponentInfo;
	FEComponentTypeInfo& TerrainInfo = ComponentIDToInfo[entt::type_id<FETerrainComponent>().hash()];

	// ************************* SKY DOME COMPONENT *************************
	FEComponentTypeInfo SkyDomeComponentInfo("SkyDome", typeid(FESkyDomeComponent));
	FunctionsToGetEntityIDListWith[SkyDomeComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FESkyDomeComponent>(); };
	ComponentIDToInfo[entt::type_id<FESkyDomeComponent>().hash()] = SkyDomeComponentInfo;
	FEComponentTypeInfo& SkyDomeInfo = ComponentIDToInfo[entt::type_id<FESkyDomeComponent>().hash()];

	// ************************* PREFAB INSTANCE COMPONENT *************************
	FEComponentTypeInfo PrefabInstanceComponentInfo("Prefab Instance", typeid(FEPrefabInstanceComponent));
	FunctionsToGetEntityIDListWith[PrefabInstanceComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FEPrefabInstanceComponent>(); };
	PrefabInstanceComponentInfo.LoadingPriority = 2;
	ComponentIDToInfo[entt::type_id<FEPrefabInstanceComponent>().hash()] = PrefabInstanceComponentInfo;
	FEComponentTypeInfo& PrefabInstanceInfo = ComponentIDToInfo[entt::type_id<FEPrefabInstanceComponent>().hash()];

	// ************************* VIRTUAL UI COMPONENT *************************
	FEComponentTypeInfo VirtualUIComponentInfo("Virtual UI", typeid(FEVirtualUIComponent));
	FunctionsToGetEntityIDListWith[VirtualUIComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FEVirtualUIComponent>(); };
	VirtualUIComponentInfo.LoadingPriority = 3;
	ComponentIDToInfo[entt::type_id<FEVirtualUIComponent>().hash()] = VirtualUIComponentInfo;
	FEComponentTypeInfo& VirtualUIInfo = ComponentIDToInfo[entt::type_id<FEVirtualUIComponent>().hash()];

	// ************************* NATIVE SCRIPT COMPONENT *************************
	FEComponentTypeInfo NativeScriptComponentInfo("Native Script", typeid(FENativeScriptComponent));
	FunctionsToGetEntityIDListWith[NativeScriptComponentInfo.Type] = [](FEScene* CurrentScene) { return CurrentScene->GetEntityIDListWith<FENativeScriptComponent>(); };
	NativeScriptComponentInfo.LoadingPriority = 4;
	ComponentIDToInfo[entt::type_id<FENativeScriptComponent>().hash()] = NativeScriptComponentInfo;
	FEComponentTypeInfo& NativeScriptInfo = ComponentIDToInfo[entt::type_id<FENativeScriptComponent>().hash()];

	// Define constraints
	TagInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {TagInfo} });
	TransformInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {TransformInfo} });
	CameraInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {CameraInfo} });
	LightInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {LightInfo} });
	GameModelInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {GameModelInfo} });

	InstancedInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {InstancedInfo} });
	InstancedInfo.Constraints.push_back({ FE_LOGIC_OPERATION::XOR, {GameModelInfo, PrefabInstanceComponentInfo} });

	TerrainInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {TerrainInfo} });
	SkyDomeInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {SkyDomeInfo} });
	PrefabInstanceInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {PrefabInstanceInfo} });
	VirtualUIInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {VirtualUIInfo, GameModelInfo} });
	// TO_DO: Maybe somehow allow multiple native script components on one entity.
	NativeScriptInfo.Constraints.push_back({ FE_LOGIC_OPERATION::NOT, {NativeScriptInfo} });
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

void FEComponentsTools::SortComponentsByLoadingPriority(std::vector<FEComponentTypeInfo>& Components)
{
	std::sort(Components.begin(), Components.end(), [](const FEComponentTypeInfo& FirstComponent, const FEComponentTypeInfo& SecondComponent) -> bool {
		int FirstPriority = FirstComponent.LoadingPriority;
		int SecondPriority = SecondComponent.LoadingPriority;

		bool bResult = FirstPriority < SecondPriority;
		return bResult;
	});
}
void FEComponentsTools::SortComponentsByLoadingPriority(std::vector<std::string>& ComponentsNames)
{
	std::sort(ComponentsNames.begin(), ComponentsNames.end(), [](const std::string& FirstComponent, const std::string& SecondComponent) -> bool {

		if (COMPONENTS_TOOL.GetComponentInfoByName(FirstComponent) == nullptr)
		{
			LOG.Add("Component info not found for component: " + FirstComponent, "FE_LOG_ECS", FE_LOG_ERROR);
			return false;
		}

		if (COMPONENTS_TOOL.GetComponentInfoByName(SecondComponent) == nullptr)
		{
			LOG.Add("Component info not found for component: " + SecondComponent, "FE_LOG_ECS", FE_LOG_ERROR);
			return false;
		}

		int FirstPriority = COMPONENTS_TOOL.GetComponentInfoByName(FirstComponent)->LoadingPriority;
		int SecondPriority = COMPONENTS_TOOL.GetComponentInfoByName(SecondComponent)->LoadingPriority;

		bool bResult = FirstPriority < SecondPriority;
		return bResult;
	});
}

FEComponentTypeInfo::FEComponentTypeInfo() : Name(), Type(nullptr) {}
FEComponentTypeInfo::FEComponentTypeInfo(const std::string& Name, const std::type_info& Type)
{
	this->Name = Name;
	this->Type = &Type;
}

bool FEComponentTypeInfo::CanBeAddedToEntity(FEEntity* PotentialParentEntity, std::string* ErrorMessage)
{
	std::string LocalErrorMessage;
	if (ErrorMessage != nullptr)
		*ErrorMessage = "";

	if (PotentialParentEntity == nullptr)
	{
		LOG.Add("ProspectParentEntity is nullptr in FEComponentTypeInfo::CanBeAddedToEntity", "FE_LOG_ECS", FE_LOG_ERROR);
		if (ErrorMessage != nullptr)
			*ErrorMessage = "ProspectParentEntity is nullptr";

		return false;
	}

	if (PotentialParentEntity->GetParentScene() == nullptr)
	{
		LOG.Add("ProspectParentEntity parent scene is nullptr in FEComponentTypeInfo::CanBeAddedToEntity", "FE_LOG_ECS", FE_LOG_ERROR);
		if (ErrorMessage != nullptr)
			*ErrorMessage = "ProspectParentEntity parent scene is nullptr";

		return false;
	}

	FEScene* ParentScene = PotentialParentEntity->GetParentScene();
	if (MaxSceneComponentCount != -1)
	{
		size_t CurrentComponentCount = COMPONENTS_TOOL.GetEntityIDListWithComponent(ParentScene, *this).size();
		if (CurrentComponentCount >= MaxSceneComponentCount)
		{
			LOG.Add("Max component count reached in FEComponentTypeInfo::CanBeAddedToEntity", "FE_LOG_ECS", FE_LOG_WARNING);

			if (ErrorMessage != nullptr)
				*ErrorMessage = "Max component count reached";

			return false;
		}
	}

	std::vector<FEComponentTypeInfo> CurrentlyExistingComponents = PotentialParentEntity->GetComponentsInfoList();

	// First we are checking if adding this component will not break any restrictions of current component
	bool bNewComponentEvaluation = EvaluateConstraints(CurrentlyExistingComponents, ErrorMessage);
	if (!bNewComponentEvaluation)
		return false;

	// Then we need to check that other components evaluations are not broken by adding this component
	for (size_t i = 0; i < CurrentlyExistingComponents.size(); i++)
	{
		// For each already existing component we need to create a list of components that will be evaluated.
		// That list should not contain the component that we are currently querying with .EvaluateConstraints.
		std::vector<FEComponentTypeInfo> NewComponentList = CurrentlyExistingComponents;
		NewComponentList.erase(NewComponentList.begin() + i);
		NewComponentList.push_back(*this);

		if (!CurrentlyExistingComponents[i].EvaluateConstraints(NewComponentList, ErrorMessage))
		{
			if (ErrorMessage != nullptr)
				*ErrorMessage = "Adding this component will break restrictions of " + CurrentlyExistingComponents[i].Name;

			return false;
		}
	}

	return true;
}

bool FEComponentTypeInfo::CanBeRemovedFromEntity(FEEntity* ParentEntity, std::string* ErrorMessage)
{
	std::string LocalErrorMessage;
	if (ErrorMessage != nullptr)
		*ErrorMessage = "";

	if (bCanNotBeRemoved)
	{
		if (ErrorMessage != nullptr)
			*ErrorMessage = "Can not be removed";

		return false;
	}

	if (ParentEntity == nullptr)
	{
		LOG.Add("ParentEntity is nullptr in FEComponentTypeInfo::CanBeRemovedFromEntity", "FE_LOG_ECS", FE_LOG_ERROR);
		if (ErrorMessage != nullptr)
			*ErrorMessage = "ParentEntity is nullptr";

		return false;
	}

	if (ParentEntity->GetParentScene() == nullptr)
	{
		LOG.Add("ParentEntity parent scene is nullptr in FEComponentTypeInfo::CanBeRemovedFromEntity", "FE_LOG_ECS", FE_LOG_ERROR);
		if (ErrorMessage != nullptr)
			*ErrorMessage = "ParentEntity parent scene is nullptr";

		return false;
	}

	FEScene* ParentScene = ParentEntity->GetParentScene();
	std::vector<FEComponentTypeInfo> CurrentlyExistingComponents = ParentEntity->GetComponentsInfoList();
	CurrentlyExistingComponents.erase(std::remove(CurrentlyExistingComponents.begin(), CurrentlyExistingComponents.end(), *this), CurrentlyExistingComponents.end());

	// After we remove the component we need to check if the remaining components are still valid after the removal.
	// Then we need to check that other components evaluations are not broken by adding this component
	for (size_t i = 0; i < CurrentlyExistingComponents.size(); i++)
	{
		// For each already existing component we need to create a list of components that will be evaluated.
		// That list should not contain the component that we are currently querying with .EvaluateConstraints.
		std::vector<FEComponentTypeInfo> NewComponentList = CurrentlyExistingComponents;
		NewComponentList.erase(NewComponentList.begin() + i);

		if (!CurrentlyExistingComponents[i].EvaluateConstraints(NewComponentList, ErrorMessage))
		{
			if (ErrorMessage != nullptr)
				*ErrorMessage = "Removing this component will break restrictions of " + CurrentlyExistingComponents[i].Name;

			return false;
		}
	}
	
	return true;
}

bool FEComponentTypeInfo::EvaluateConstraints(const std::vector<FEComponentTypeInfo>& ComponentToCheckTowards, std::string* ErrorMessage) const
{
	for (const auto& CurrentConstraint : Constraints)
	{
		if (!EvaluateConstraint(CurrentConstraint, ComponentToCheckTowards))
		{
			if (ErrorMessage)
			{
				*ErrorMessage = "Constraint failed: ";
				for (const auto& CurrentComponent : CurrentConstraint.Components)
					*ErrorMessage += CurrentComponent.Name + ", ";

				*ErrorMessage += "with operation " + LogicOperationToString(CurrentConstraint.LogicOperation);
			}
			return false;
		}
	}
	return true;
}

bool FEComponentTypeInfo::EvaluateConstraint(const FEComponentConstraint& Constraint, const std::vector<FEComponentTypeInfo>& ComponentToCheckTowards) const
{
	auto ComponentExists = [&ComponentToCheckTowards](const FEComponentTypeInfo& comp) {
		return std::find(ComponentToCheckTowards.begin(), ComponentToCheckTowards.end(), comp) != ComponentToCheckTowards.end();
	};

	switch (Constraint.LogicOperation)
	{
		case FE_LOGIC_OPERATION::AND:
			return std::all_of(Constraint.Components.begin(), Constraint.Components.end(), ComponentExists);
		case FE_LOGIC_OPERATION::OR:
			return std::any_of(Constraint.Components.begin(), Constraint.Components.end(), ComponentExists);
		case FE_LOGIC_OPERATION::NOT:
			return std::none_of(Constraint.Components.begin(), Constraint.Components.end(), ComponentExists);
		case FE_LOGIC_OPERATION::XOR:
			return std::count_if(Constraint.Components.begin(), Constraint.Components.end(), ComponentExists) == 1;
		default:
			return false;
	}
}

std::string FEComponentTypeInfo::LogicOperationToString(FE_LOGIC_OPERATION LogicOperation) const
{
	switch (LogicOperation)
	{
		case FocalEngine::FE_LOGIC_OPERATION::AND:
			return "AND";
		case FocalEngine::FE_LOGIC_OPERATION::OR:
			return "OR";
		case FocalEngine::FE_LOGIC_OPERATION::NOT:
			return "NOT";
		case FocalEngine::FE_LOGIC_OPERATION::XOR:
			return "XOR";
		default:
			break;
	}

	return "UNKNOWN";
}