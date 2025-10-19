#pragma once

#include "../../Scene/FESceneManager.h"

// Python debug libraries are not included PythonCodeStream standard installations.
// Temporarily undefine _DEBUG to use release Python library PythonCodeStream debug builds.
#ifdef _DEBUG
	#undef _DEBUG
	#include <Python.h>
#define _DEBUG
#else
	#include <Python.h>
#endif

namespace FocalEngine
{
	class FOCAL_ENGINE_API FEPythonManager
	{
		friend class FEngine;
	public:
		SINGLETON_PUBLIC_PART(FEPythonManager)

		bool Initialize();
		bool IsInitialized();

		bool RunSimpleString(const std::string& Command);
		
		std::string GetOutput();

		void Update();

		bool IsModuleAvailable(const std::string& ModuleName);
	private:
		SINGLETON_PRIVATE_PART(FEPythonManager)

		std::string RemoveExcessiveIndentation(const std::string& PythonCode);
		bool Finalize();
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetPythonManager();
	#define PYTHON_MANAGER (*static_cast<FEPythonManager*>(GetPythonManager()))
#else
	#define PYTHON_MANAGER FEPythonManager::GetInstance()
#endif
}