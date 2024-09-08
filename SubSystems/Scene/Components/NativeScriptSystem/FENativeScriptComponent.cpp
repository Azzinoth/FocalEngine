#include "FENativeScriptComponent.h"
#include "../../FEScene.h"
using namespace FocalEngine;

void FENativeScriptComponent::Awake()
{
	CoreInstance->Awake();
}

void FENativeScriptComponent::OnDestroy()
{
	if (CoreInstance == nullptr)
		return;

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

bool FENativeScriptComponent::SetVariableValue(std::string VariableName, std::any Value)
{
	if (CoreInstance == nullptr)
		return false;

	if (ScriptData->VariablesRegistry.find(VariableName) == ScriptData->VariablesRegistry.end())
		return false;

	try
	{
		ScriptData->VariablesRegistry[VariableName].Setter(CoreInstance, Value);
		return true;
	}
	catch (const std::bad_any_cast&)
	{
		LOG.Add("FENativeScriptComponent::SetVariableValue: Error setting value for '" + VariableName + "'", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return false;
	}

	return true;
}

std::any FENativeScriptComponent::GetVariableValueRaw(const std::string& VariableName) const
{
	if (CoreInstance == nullptr || ScriptData == nullptr)
		return std::any();

	auto VariablesIterator = ScriptData->VariablesRegistry.find(VariableName);
	if (VariablesIterator == ScriptData->VariablesRegistry.end())
	{
		LOG.Add("FENativeScriptComponent::GetVariableValueRaw: Variable with name: " + VariableName + " not found in registry.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
		return std::any();
	}

	return VariablesIterator->second.Getter(CoreInstance);
}