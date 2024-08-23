#include "FENativeScriptComponent.h"
#include "../../FEScene.h"
using namespace FocalEngine;

void FENativeScriptComponent::OnCreate()
{
	CoreInstance->OnCreate();
}

void FENativeScriptComponent::OnDestroy()
{
	CoreInstance->OnDestroy();
	delete CoreInstance;
	CoreInstance = nullptr;
}

void FENativeScriptComponent::OnUpdate(double DeltaTime)
{
	CoreInstance->OnUpdate(DeltaTime);
}

bool FENativeScriptComponent::IsInitialized()
{
	return CoreInstance != nullptr;
}

std::string FENativeScriptComponent::GetModuleID()
{
	return ModuleID;
}

class FENativeScriptCore* FENativeScriptComponent::GetCoreInstance() const
{
	return CoreInstance;
}

const struct FEScriptData* FENativeScriptComponent::GetScriptData() const
{
	return ScriptData;
}