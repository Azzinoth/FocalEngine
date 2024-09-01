#pragma once

#include"FEEntity.h"
#include "../ResourceManager/FEResourceManager.h"
#include "../Renderer/FELine.h"
#include "FENaiveSceneGraph.h"
#include <typeindex>
#include "entt.hpp"

namespace FocalEngine
{
	enum class FESceneFlag : uint32_t
	{
		None = 0,
		Renderable = 1 << 0,
		Active = 1 << 1,
		EditorMode = 1 << 2,
		GameMode = 1 << 3,
	};

	// Enable bitwise operations for FESceneFlag
	inline FESceneFlag operator|(FESceneFlag First, FESceneFlag Second)
	{
		return static_cast<FESceneFlag>(
			static_cast<std::underlying_type_t<FESceneFlag>>(First) |
			static_cast<std::underlying_type_t<FESceneFlag>>(Second)
		);
	}

	inline FESceneFlag& operator|=(FESceneFlag& First, FESceneFlag Second)
	{
		First = First | Second;
		return First;
	}

	inline FESceneFlag operator&(FESceneFlag First, FESceneFlag Second)
	{
		return static_cast<FESceneFlag>(
			static_cast<std::underlying_type_t<FESceneFlag>>(First) &
			static_cast<std::underlying_type_t<FESceneFlag>>(Second)
		);
	}

	inline FESceneFlag& operator&=(FESceneFlag& First, FESceneFlag Second)
	{
		First = First & Second;
		return First;
	}

	class FEScene : public FEObject
	{
		friend class FEEntity;
		friend class FENaiveSceneGraphNode;
		friend class FENaiveSceneGraph;
		friend class FESceneManager;
		friend class FEInstancedSystem;
		friend class FERenderer;
		friend class FEngine;
		friend class FETransformSystem;
	public:
		FEScene();

		void SetFlag(FESceneFlag Flag, bool Value);
		bool HasFlag(FESceneFlag Flag) const;

		// Entity Management
		FEEntity* GetEntity(std::string ID);
		FEEntity* CreateEntity(std::string Name = "", std::string ForceObjectID = "");
		FEEntity* CreateEntityOrphan(std::string Name = "", std::string ForceObjectID = "");
		std::vector<std::string> GetEntityIDList();
		template<typename T> std::vector<std::string> GetEntityIDListWithComponent();
		template<typename T> std::vector<FEEntity*> GetEntityListWithComponent();
		std::vector<FEEntity*> GetEntityByName(std::string Name);
		FEEntity* DuplicateEntity(std::string ID, std::string NewEntityName = "");
		FEEntity* DuplicateEntity(FEEntity* SourceEntity, std::string NewEntityName = "");
		FEEntity* ImportEntity(FEEntity* EntityFromDifferentScene, FENaiveSceneGraphNode* TargetParent = nullptr, std::function<bool(FEEntity*)> Filter = nullptr);
		void DeleteEntity(std::string ID);
		void DeleteEntity(FEEntity* Entity);

		FEAABB GetEntityAABB(std::string ID);
		FEAABB GetEntityAABB(FEEntity* Entity);
		FEAABB GetSceneAABB(std::function<bool(FEEntity*)> Filter = nullptr);

		// Asset Management
		std::vector<FEObject*> ImportAsset(std::string FileName);

		// Scene Management
		void PrepareForGameModelDeletion(const FEGameModel* GameModel);
		void PrepareForPrefabDeletion(const FEPrefab* Prefab);
		void Clear();
		void Update();

		FENaiveSceneGraph SceneGraph;
	private:
		~FEScene();

		FESceneFlag Flags = FESceneFlag::None;

		// Internal Entity Management
		FEEntity* GetEntityByEnTT(entt::entity ID);
		void ClearEntityRecords(std::string EntityID, entt::entity EnttEntity);
		FEEntity* CreateEntityInternal(std::string Name = "", std::string ForceObjectID = "");
		FEEntity* DuplicateEntityInternal(FEEntity* SourceEntity, std::string NewEntityName = "");

		// Asset Loading
		std::vector<FEObject*> LoadGLTF(std::string FileName);
		std::vector<FEObject*> AddGLTFNodeToSceneGraph(const FEGLTFLoader& GLTF, const GLTFNodes& Node, const std::unordered_map<int, std::vector<FEGameModel*>>& GLTFMeshesToGameModelMap, const std::string ParentID);

		// Data Members
		entt::registry Registry;
		bool bIsSceneClearing = false;
		//bool bActive = true;
		std::unordered_map<entt::entity, FEEntity*> EnttToEntity;
		std::unordered_map<std::string, FEEntity*> EntityMap;
	};
#include "FEScene.inl"
}