#pragma once

template<typename T>
bool FENativeScriptComponent::GetVariableValue(const std::string& VariableName, T& OutValue) const
{
    if (CoreInstance == nullptr || ScriptData == nullptr)
        return false;

    auto VariablesIterator = ScriptData->VariablesRegistry.find(VariableName);
    if (VariablesIterator == ScriptData->VariablesRegistry.end())
    {
        LOG.Add("FENativeScriptComponent::GetVariableValueRaw: Variable with name: " + VariableName + " not found in registry.", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
        return false;
    }

    const auto& VariableInfo = VariablesIterator->second;
    try
    {
        std::any Value = VariableInfo.Getter(CoreInstance);
        OutValue = std::any_cast<T>(Value);
        return true;
    }
    catch (const std::bad_any_cast&)
    {
		LOG.Add("FENativeScriptComponent::GetVariableValue: Error std::bad_any_cast for '" + VariableName + "'", "FE_SCRIPT_SYSTEM", FE_LOG_ERROR);
        return false;
    }
}