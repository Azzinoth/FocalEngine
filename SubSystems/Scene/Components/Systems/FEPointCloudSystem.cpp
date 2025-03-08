#include "FEPointCloudSystem.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetPointCloudSystem()
{
	return FEPointCloudSystem::GetInstancePointer();
}
#endif

FEPointCloudSystem::FEPointCloudSystem()
{
	RegisterOnComponentCallbacks();
	//COMPONENTS_TOOL.RegisterComponentToJsonFunction<FEPointCloudComponent>(PointCloudComponentToJson);
	//COMPONENTS_TOOL.RegisterComponentFromJsonFunction<FEPointCloudComponent>(PointCloudComponentFromJson);
	COMPONENTS_TOOL.RegisterComponentDuplicateFunction<FEPointCloudComponent>(DuplicatePointCloudComponent);
}

void FEPointCloudSystem::RegisterOnComponentCallbacks()
{
	SCENE_MANAGER.RegisterOnComponentConstructCallback<FEPointCloudComponent>(OnMyComponentAdded);
	SCENE_MANAGER.RegisterOnComponentDestroyCallback<FEPointCloudComponent>(OnMyComponentDestroy);
}

void FEPointCloudSystem::OnMyComponentAdded(FEEntity* Entity)
{
	if (POINT_CLOUD_SYSTEM.bInternalAdd)
		return;

	if (Entity == nullptr || !Entity->HasComponent<FEPointCloudComponent>())
		return;

	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();
}

void FEPointCloudSystem::DuplicatePointCloudComponent(FEEntity* SourceEntity, FEEntity* TargetEntity)
{
	if (SourceEntity == nullptr || TargetEntity == nullptr || !SourceEntity->HasComponent<FEPointCloudComponent>())
		return;

	FEPointCloudComponent& PointCloudComponent = SourceEntity->GetComponent<FEPointCloudComponent>();

	POINT_CLOUD_SYSTEM.bInternalAdd = true;
	TargetEntity->AddComponent<FEPointCloudComponent>();
	POINT_CLOUD_SYSTEM.bInternalAdd = false;
	FEPointCloudComponent& NewPointCloudComponent = TargetEntity->GetComponent<FEPointCloudComponent>();

	NewPointCloudComponent = PointCloudComponent;
}

void FEPointCloudSystem::OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing)
{
	if (Entity == nullptr || !Entity->HasComponent<FEPointCloudComponent>())
		return;

	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();
}

FEPointCloudSystem::~FEPointCloudSystem() {};

//Json::Value FEPointCloudSystem::PointCloudComponentToJson(FEEntity* Entity)
//{
//	Json::Value Root;
//	if (Entity == nullptr || !Entity->HasComponent<FEPointCloudComponent>())
//	{
//		LOG.Add("FEPointCloudSystem::PointCloudComponentToJson Entity is nullptr or does not have FEPointCloudComponent", "FE_LOG_ECS", FE_LOG_WARNING);
//		return Root;
//	}
//	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();
//
//	Root["Type"] = PointCloudComponent.GetType();
//
//	Root["Color"] = Json::Value();
//	Root["Color"]["R"] = PointCloudComponent.Color.x;
//	Root["Color"]["G"] = PointCloudComponent.Color.y;
//	Root["Color"]["B"] = PointCloudComponent.Color.z;
//	Root["Intensity"] = PointCloudComponent.Intensity;
//	Root["IsEnabled"] = PointCloudComponent.bEnabled;
//
//	Root["CastShadows"] = PointCloudComponent.bCastShadows;
//	Root["IsUsingCascadeShadows"] = PointCloudComponent.bUseCascadeShadows;
//	Root["ActiveCascades"] = PointCloudComponent.ActiveCascades;
//	Root["ShadowCoverage"] = PointCloudComponent.ShadowCoverage;
//	Root["CSMXYDepth"] = PointCloudComponent.CSMXYDepth;
//	Root["CSMZDepth"] = PointCloudComponent.CSMZDepth;
//
//	Root["Static ShadowBias"] = PointCloudComponent.IsStaticShadowBias();
//	Root["ShadowBias"] = PointCloudComponent.GetShadowBias();
//	Root["ShadowBias VariableIntensity"] = PointCloudComponent.GetShadowBiasVariableIntensity();
//	Root["Shadow BlurFactor"] = PointCloudComponent.GetShadowBlurFactor();
//
//	Root["SpotAngle"] = PointCloudComponent.GetSpotAngle();
//	Root["SpotAngle Outer"] = PointCloudComponent.GetSpotAngleOuter();
//	Root["Direction"]["X"] = PointCloudComponent.Direction.x;
//	Root["Direction"]["Y"] = PointCloudComponent.Direction.y;
//	Root["Direction"]["Z"] = PointCloudComponent.Direction.z;
//
//	Root["Range"] = PointCloudComponent.GetRange();
//
//	return Root;
//}
//
//void FEPointCloudSystem::PointCloudComponentFromJson(FEEntity* Entity, Json::Value Root)
//{
//	if (Entity == nullptr)
//	{
//		LOG.Add("FEPointCloudSystem::PointCloudComponentFromJson Entity is nullptr", "FE_LOG_ECS", FE_LOG_WARNING);
//		return;
//	}
//
//	int LightType = Root["Type"].asInt();
//	FE_LIGHT_TYPE FELightType;
//	switch (LightType)
//	{
//		case 0:
//			FELightType = FE_NULL_LIGHT;
//		break;
//
//		case 1:
//			FELightType = FE_DIRECTIONAL_LIGHT;
//		break;
//
//		case 2:
//			FELightType = FE_POINT_LIGHT;
//		break;
//
//		case 3:
//			FELightType = FE_SPOT_LIGHT;
//		break;
//
//	default:
//		FELightType = FE_NULL_LIGHT;
//		break;
//	}
//
//	Entity->AddComponent<FEPointCloudComponent>(FELightType);
//	FEPointCloudComponent& PointCloudComponent = Entity->GetComponent<FEPointCloudComponent>();
//
//	PointCloudComponent.SetColor(glm::vec3(Root["Color"]["R"].asFloat(), Root["Color"]["G"].asFloat(), Root["Color"]["B"].asFloat()));
//	PointCloudComponent.SetIntensity(Root["Intensity"].asFloat());
//	PointCloudComponent.SetLightEnabled(Root["IsEnabled"].asBool());
//
//	PointCloudComponent.SetCastShadows(Root["CastShadows"].asBool());
//	PointCloudComponent.bUseCascadeShadows = Root["IsUsingCascadeShadows"].asBool();
//	PointCloudComponent.SetActiveCascades(Root["ActiveCascades"].asInt());
//	PointCloudComponent.SetShadowCoverage(Root["ShadowCoverage"].asFloat());
//	PointCloudComponent.SetCSMXYDepth(Root["CSMXYDepth"].asFloat());
//	PointCloudComponent.SetCSMZDepth(Root["CSMZDepth"].asFloat());
//
//	PointCloudComponent.SetIsStaticShadowBias(Root["Static ShadowBias"].asBool());
//	PointCloudComponent.SetShadowBias(Root["ShadowBias"].asFloat());
//	PointCloudComponent.SetShadowBiasVariableIntensity(Root["ShadowBias VariableIntensity"].asBool());
//	PointCloudComponent.SetShadowBlurFactor(Root["Shadow BlurFactor"].asFloat());
//
//	PointCloudComponent.SetSpotAngle(Root["SpotAngle"].asFloat());
//	PointCloudComponent.SetSpotAngleOuter(Root["SpotAngle Outer"].asFloat());
//	PointCloudComponent.Direction.x = Root["Direction"]["X"].asFloat();
//	PointCloudComponent.Direction.y = Root["Direction"]["Y"].asFloat();
//	PointCloudComponent.Direction.z = Root["Direction"]["Z"].asFloat();
//
//	PointCloudComponent.SetRange(Root["Range"].asFloat());
//}