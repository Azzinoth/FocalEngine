#include "FENativeScriptComponent.h"
#include "../FEScene.h"
using namespace FocalEngine;

CoreScriptManager::CoreScriptManager() {}
CoreScriptManager::~CoreScriptManager() {}

extern "C" __declspec(dllexport) size_t GetScriptCount()
{
    //return CORE_SCRIPT_MANAGER.GetAllScripts().size();
	return 2;
}

std::unordered_map<std::string, CoreScriptManager::ScriptCreator> CoreScriptManager::GetAllScripts() const
{
	return GetRegistry();
}

extern "C" __declspec(dllexport) char** GetScriptMap()
{
    std::vector<std::string> scriptsName;
	scriptsName.push_back("PlayerScript");
	scriptsName.push_back("EnemyScript");

    int Count = static_cast<int>(scriptsName.size());
    char** result = new char* [scriptsName.size()];

    int i = 0;
    for (const auto& script : scriptsName)
    {
		if (script.size() > 1024)
		{
			std::string error = "Script name is too long!";
            result[i] = new char[error.size() + 1];
            strcpy_s(result[i], error.size() + 1, error.c_str());
		}
        else
        {
            result[i] = new char[script.size() + 1];
            strcpy_s(result[i], script.size() + 1, script.c_str());
        }
       
        i++;
    }

    return result;
}

//FENativeScriptComponent::FENativeScriptComponent()
//{
//	
//}

//void FENativeScriptComponent::CopyFrom(const FENativeScriptComponent& Other)
//{
//	ParentEntity = Other.ParentEntity;
//
//}
//
//FENativeScriptComponent::FENativeScriptComponent(const FENativeScriptComponent& Other)
//{
//	if (this == &Other)
//		return;
//
//	CopyFrom(Other);
//}
//
//FENativeScriptComponent& FENativeScriptComponent::operator=(const FENativeScriptComponent& Other)
//{
//	if (this == &Other)
//		return *this;
//
//	CopyFrom(Other);
//
//	return *this;
//}

//FENativeScriptComponent::~FENativeScriptComponent()
//{
//}