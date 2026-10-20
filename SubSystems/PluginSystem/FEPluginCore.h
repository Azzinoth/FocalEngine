#pragma once
#include "../../Core/FECoreIncludes.h"

namespace FocalEngine
{
	class FEScene;
	enum class FESceneFlag : uint32_t;

	class FEPluginCore
	{
		friend class FEPluginManager;

		// Set by plugin manager before OnLoad.
		std::string PluginDirectory = "";
	public:
		// Increase whenever layout of FEPluginCore (virtual functions or data members) or functions exported by FEPluginConnector change.
		// Plugin manager refuses to load plugins compiled with different version.
		static constexpr int InterfaceVersion = 2;

		virtual ~FEPluginCore() = default;
		virtual void OnLoad() {}
		// Plugin must remove everything it registered with the engine, its DLL is unloaded right after.
		virtual void OnUnload() {}
		// Called once per frame, before native scripts and transform update.
		virtual void OnUpdate(double DeltaTime) {}

		// Called once per frame, after native scripts and transform update, before active scenes are rendered.
		virtual void OnBeforeRender() {}
		// Called once per frame, after all active scenes are rendered, before main window renders UI.
		// Plugin that changes main camera, viewport or bound framebuffer should restore them before returning.
		virtual void OnAfterRender() {}

		// Scenes should not be deleted inside scene hooks.
		// Called once for every scene, at the beginning of the first frame after scene was added, so its content is already loaded.
		// Scenes that existed before plugin was loaded are reported in the first frame.
		virtual void OnSceneAdded(FEScene* Scene) {}
		// Called at the beginning of the frame if scene flags differ from the previously reported ones, only net change between frames is reported.
		virtual void OnSceneFlagsChanged(FEScene* Scene, FESceneFlag PreviousFlags) {}
		// Called right before scene is deleted, scene and its entities are still valid.
		// Plugin should release everything related to that scene, including pointers to the scene and its entities.
		virtual void OnSceneDeleting(FEScene* Scene) {}

		// Absolute path of the folder that plugin was loaded from, ends with "/".
		const std::string& GetPluginDirectory() const { return PluginDirectory; }
	};
}