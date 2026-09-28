#pragma once

#include "../Scene/FESceneManager.h"

// Forward declarations, so that LeiaSR SDK headers are only needed in Leia_3D_Manager.cpp.
namespace SR
{
	class SRContext;
	class SwitchableLensHint;
}

namespace FocalEngine
{
	class FOCAL_ENGINE_API Leia3DManager
	{
		friend class FEngine;
	public:
		SINGLETON_PUBLIC_PART(Leia3DManager)

		bool Set3DModeEnabled(bool bNewValue);
		bool Is3DModeEnabled() const;
	private:
		SINGLETON_PRIVATE_PART(Leia3DManager)

		SR::SRContext* Context = nullptr;
		SR::SwitchableLensHint* LensHint = nullptr;
		bool b3DModeEnabled = false;

		bool InitializeContext(bool bInitialLensState);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetLeia3DManager();
	#define LEIA_3D_MANAGER (*static_cast<Leia3DManager*>(GetLeia3DManager()))
#else
	#define LEIA_3D_MANAGER Leia3DManager::GetInstance()
#endif
}