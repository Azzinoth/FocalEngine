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
		FE_FRAME_BUFFER = 7,
		FE_POST_PROCESS = 8,
		FE_TERRAIN_LAYER = 9,
		FE_PREFAB = 10,
		FE_SCENE_GRAPH_NODE = 11,
		FE_SCENE = 12
	};

	struct FEObjectLoadedData
	{
		std::string ID;
		FE_OBJECT_TYPE Type = FE_NULL;
		std::string Tag;
		std::string Name;
	};

	class FOCAL_ENGINE_API FEObjectManager
	{
		friend class FEObject;
		friend class FEngine;
		friend class FERenderer;
		friend class FEResourceManager;
		friend class FEScene;
		friend class FESceneManager;
	public:
		SINGLETON_PUBLIC_PART(FEObjectManager)
		FEObject* GetFEObject(std::string ID);
		// Takes open file stream and saves the object part of the file.
		void SaveFEObjectPart(std::fstream& OpenedFile, FEObject* Object);
		// Takes open file stream and loads the object part of the file, also returns bytes read.
		FEObjectLoadedData LoadFEObjectPart(std::fstream& OpenedFile);
		// Takes file data as char* and loads the object part, also returns bytes read.
		FEObjectLoadedData LoadFEObjectPart(char* FileData, int& CurrentShift);
	private:
		SINGLETON_PRIVATE_PART(FEObjectManager)
		std::unordered_map<std::string, FEObject*> AllObjects;
		std::vector<std::unordered_map<std::string, FEObject*>> ObjectsByType;
	};

#define OBJECT_MANAGER FEObjectManager::GetInstance()

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
			case FocalEngine::FE_SCENE_GRAPH_NODE:
			{
				return "FE_SCENE_GRAPH_NODE";
			}
			case FocalEngine::FE_SCENE:
			{
				return "FE_SCENE";
			}
			default:
				break;
		}

		return "FE_NULL";
	}

	class FOCAL_ENGINE_API FEObject
	{
		friend class FEObjectManager;
		friend class FEngine;
		friend class FERenderer;
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
		friend class FESceneManager;
		friend class FETerrainSystem;
		friend class FESkyDomeSystem;
	public:
		FEObject(FE_OBJECT_TYPE ObjectType, std::string ObjectName);
		~FEObject();

		std::string GetObjectID() const;
		FE_OBJECT_TYPE GetType() const;

		std::string GetTag() const;

		bool IsDirty() const;
		void SetDirtyFlag(bool NewValue);

		std::string GetName() const;
		void SetName(std::string NewValue);
		int GetNameHash() const;

		void SetIDOfUnTyped(std::string NewValue);
	private:
		std::string ID;
		FE_OBJECT_TYPE Type = FE_NULL;
		std::string Tag = "";
		bool bDirtyFlag = false;

		std::string Name;
		int NameHash = 0;
		void SetID(std::string NewValue);
		void SetTag(std::string NewValue);
		void SetType(FE_OBJECT_TYPE NewValue);

	protected:
		std::vector<std::string> CallListOnDeleteFEObject;
		virtual void ProcessOnDeleteCallbacks(std::string DeletingFEObject);
	};
}

#endif FEOBJECT_H