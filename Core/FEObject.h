#pragma once

#ifndef FEOBJECT_H
#define FEOBJECT_H

#include "../Core/FECoreIncludes.h"

namespace FocalEngine
{
	enum FE_OBJECT_TYPE
	{
		FE_NULL = 0,
		FE_SHADER = 1,
		FE_TEXTURE = 2,
		FE_MESH = 3,
		FE_MATERIAL = 4,
		FE_GAMEMODEL = 5,
		FE_ENTITY = 6,
		FE_TERRAIN = 7,
		FE_ENTITY_INSTANCED = 8,
		FE_DIRECTIONAL_LIGHT = 9,
		FE_POINT_LIGHT = 10,
		FE_SPOT_LIGHT = 11,
		FE_CAMERA = 12,
		FE_FRAME_BUFFER = 13,
		FE_POST_PROCESS = 14,
		FE_TERRAIN_LAYER = 15,
		FE_PREFAB = 16,
		FE_VIRTUAL_UI_CONTEXT = 17,
		FE_SCENE_ENTITY = 18,
		FE_NEW_ENTITY = 19
	};

	class FEObjectManager
	{
		friend class FEObject;
		friend class FEngine;
		friend class FERenderer;
		friend class FEResourceManager;
		friend class FEScene;
	public:
		SINGLETON_PUBLIC_PART(FEObjectManager)
		FEObject* GetFEObject(std::string ID);
	private:
		SINGLETON_PRIVATE_PART(FEObjectManager)
		std::unordered_map<std::string, FEObject*> AllObjects;
		std::vector<std::unordered_map<std::string, FEObject*>> ObjectsByType;
	};

	static std::string FEObjectTypeToString(const FE_OBJECT_TYPE Type)
	{
		switch (Type)
		{
			case FocalEngine::FE_NULL:
			{
				return "FE_NULL";
			}
			case FocalEngine::FE_SHADER:
			{
				return "FE_SHADER";
			}
			case FocalEngine::FE_TEXTURE:
			{
				return "FE_TEXTURE";
			}
			case FocalEngine::FE_MESH:
			{
				return "FE_MESH";
			}
			case FocalEngine::FE_MATERIAL:
			{
				return "FE_MATERIAL";
			}
			case FocalEngine::FE_GAMEMODEL:
			{
				return "FE_GAMEMODEL";
			}
			case FocalEngine::FE_ENTITY:
			{
				return "FE_ENTITY";
			}
			case FocalEngine::FE_TERRAIN:
			{
				return "FE_TERRAIN";
			}
			case FocalEngine::FE_ENTITY_INSTANCED:
			{
				return "FE_ENTITY_INSTANCED";
			}
			case FocalEngine::FE_DIRECTIONAL_LIGHT:
			{
				return "FE_DIRECTIONAL_LIGHT";
			}
			case FocalEngine::FE_POINT_LIGHT:
			{
				return "FE_POINT_LIGHT";
			}
			case FocalEngine::FE_SPOT_LIGHT:
			{
				return "FE_SPOT_LIGHT";
			}
			case FocalEngine::FE_CAMERA:
			{
				return "FE_CAMERA";
			}
			case FocalEngine::FE_FRAME_BUFFER:
			{
				return "FE_FRAME_BUFFER";
			}
			case FocalEngine::FE_POST_PROCESS:
			{
				return "FE_POST_PROCESS";
			}
			case FocalEngine::FE_PREFAB:
			{
				return "FE_PREFAB";
			}
			case FocalEngine::FE_VIRTUAL_UI_CONTEXT:
			{
				return "FE_VIRTUAL_UI_CONTEXT";
			}
			case FocalEngine::FE_SCENE_ENTITY:
			{
				return "FE_SCENE_ENTITY";
			}
			case FocalEngine::FE_NEW_ENTITY:
			{
				return "FE_NEW_ENTITY";
			}
			default:
				break;
		}

		return "FE_NULL";
	}

	class FEObject
	{
		friend class FEngine;
		friend class FEShader;
		friend class FEMesh;
		friend class FETexture;
		friend class FEMaterial;
		friend class FEGameModel;
		friend class FEEntity;
		friend class FETerrain;
		friend class FEEntityInstanced;
		friend class FEResourceManager;
		friend class FENaiveSceneGraphNode;
		friend class FEScene;
	public:
		FEObject(FE_OBJECT_TYPE ObjectType, std::string ObjectName);
		~FEObject();

		std::string GetObjectID() const;
		FE_OBJECT_TYPE GetType() const;

		bool IsDirty() const;
		void SetDirtyFlag(bool NewValue);

		std::string GetName() const;
		void SetName(std::string NewValue);
		int GetNameHash() const;

		void SetIDOfUnTyped(std::string NewValue);
	private:
		std::string ID;
		FE_OBJECT_TYPE Type = FE_NULL;
		bool bDirtyFlag = false;

		std::string Name;
		int NameHash = 0;
		void SetID(std::string NewValue);
		void SetType(FE_OBJECT_TYPE NewValue);
	protected:
		std::vector<std::string> CallListOnDeleteFEObject;
		virtual void ProcessOnDeleteCallbacks(std::string DeletingFEObject);
	};

	#define OBJECT_MANAGER FEObjectManager::getInstance()
}

#endif FEOBJECT_H