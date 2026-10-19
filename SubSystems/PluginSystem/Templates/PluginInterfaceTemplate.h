#pragma once

// Public API of PLUGIN_NAME_PLACE_HOLDER, applications include only this header.
// Usage:
//	using PluginInterface = INTERFACE_NAME_PLACE_HOLDER;
//	PluginInterface* Plugin = PLUGIN_MANAGER.GetPlugin<PluginInterface>(UNIQUE_ID.FromString(PluginInterface::PluginID));
//	if (Plugin != nullptr && Plugin->GetInterfaceVersion() == PluginInterface::InterfaceVersion)
//		Plugin->...;
class INTERFACE_NAME_PLACE_HOLDER
{
public:
	// Same as "ID" in PLUGIN_NAME_PLACE_HOLDER.json.
	static constexpr const char* PluginID = "PLUGIN_ID_PLACE_HOLDER";

	// Increase when anything in this file changes.
	static constexpr int InterfaceVersion = 1;

	// Order of virtual functions is part of the interface, new functions should be added only at the end.
	virtual ~INTERFACE_NAME_PLACE_HOLDER() = default;
	virtual int GetInterfaceVersion() const { return InterfaceVersion; }
};