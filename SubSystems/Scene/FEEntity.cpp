#include "FEEntity.h"
using namespace FocalEngine;
#include "FEScene.h"

FEEntity::FEEntity(entt::entity AssignedEnTTEntity, FEScene* Scene) : FEObject(FE_OBJECT_TYPE::FE_ENTITY, "Unnamed Entity")
{
	EnTTEntity = AssignedEnTTEntity;
	ParentScene = Scene;
}

entt::registry& FEEntity::GetRegistry()
{
	return ParentScene->Registry;
}

FEEntity::~FEEntity()
{
	ParentScene->Registry.destroy(EnTTEntity);
	ParentScene->ClearEntityRecords(GetObjectID(), EnTTEntity);
}

Json::Value FEEntity::ToJson()
{
	Json::Value Root;
	Root["FEObjectData"] = RESOURCE_MANAGER.SaveFEObjectPart(this);
	SaveComponents(Root["Components"]);

	return Root;
}

void FEEntity::SaveComponents(Json::Value& Root)
{
	std::vector<FEComponentTypeInfo> List = GetComponentsInfoList();
	for (size_t i = 0; i < List.size(); i++)
	{
		if (List[i].ToJson != nullptr)
			Root[List[i].Name] = List[i].ToJson(this);
	}
}

void FEEntity::FromJson(Json::Value Root)
{
	// ID and Name should be set before calling this function
	std::vector<Json::String> ComponentsList = Root["Components"].getMemberNames();
	// Sort to make sure that components are loaded in proper order.
	std::sort(ComponentsList.begin(), ComponentsList.end(), [](const std::string& FirstComponent, const std::string& SecondComponent) -> bool {
		int FirstPriority = COMPONENTS_TOOL.GetComponentInfoByName(FirstComponent)->LoadingPriority;
		int SecondPriority = COMPONENTS_TOOL.GetComponentInfoByName(SecondComponent)->LoadingPriority;

		bool bResult = FirstPriority < SecondPriority;
		return bResult;
	});

	for (size_t i = 0; i < ComponentsList.size(); i++)
	{
		std::string ComponentName = ComponentsList[i];
		FEComponentTypeInfo* ComponentInfo = COMPONENTS_TOOL.GetComponentInfoByName(ComponentName);

		if (ComponentInfo == nullptr)
		{
			LOG.Add("FEEntity::FromJson: Could not find component info for component: " + ComponentName, "FE_LOG_LOADING", FE_LOG_ERROR);
			continue;
		}

		if (ComponentInfo->FromJson == nullptr)
		{
			LOG.Add("FEEntity::FromJson: Component: " + ComponentName + " does not have FromJson function!", "FE_LOG_LOADING", FE_LOG_ERROR);
			continue;
		}

		ComponentInfo->FromJson(this, Root["Components"][ComponentName]);
	}
}