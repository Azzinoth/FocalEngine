#pragma once
#include "../../Scene/FESceneManager.h"

namespace FocalEngine
{
	class FEInstancedSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEInstancedSystem)

		bool bInternalAdd = false;

		std::vector<std::pair<std::string, std::string>> EnitityIDListToInitialize;

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		void InitializeBuffers(FEEntity* Entity);
		void InitializeBuffer(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex = 0);
		void InitializeGPUCullingBuffers(FEEntity* Entity);
		void InitializeGPUCullingBuffer(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex = 0);

		void DuplicateInstancedComponent(FEEntity* EntityWithInstancedComponent, FEEntity* NewEntity);

		bool PopulateInstanceInternal(FEEntity* Entity, FEGameModelComponent& GameModelComponent, FESpawnInfo SpawnInfo);

		void AddInstanceInternal(FEEntity* Entity, glm::mat4 InstanceMatrix);
		void AddInstanceInternal(FEEntity* Entity, FEGameModelComponent& GameModelComponent, glm::mat4 InstanceMatrix, size_t BufferIndex = 0);
		void AddInstances(FEEntity* EntityWithInstancedComponent, const glm::mat4* InstanceMatrix, size_t Count);
		void AddInstances(FEEntity* Entity, FEGameModelComponent& GameModelComponent, const glm::mat4* InstanceMatrix, size_t Count, size_t BufferIndex = 0);

		void UpdateBuffers(FEEntity* Entity);
		void UpdateBuffer(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex = 0);

		void UpdateMatrices(FEEntity* Entity);
		void UpdateMatrix(FEEntity* Entity, size_t BufferIndex = 0);

		void Update();

		void Render(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex = 0);
		void RenderGameModelComponent(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, size_t BufferIndex = 0);

		void RenderOnlyBillbords(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex);
		void RenderOnlyBillbordsInternal(FEEntity* Entity, FEGameModelComponent& GameModelComponent, size_t BufferIndex = 0);

		void CheckDirtyFlag(FEEntity* Entity);

		static Json::Value InstanceComponentToJson(FEEntity* Entity);
		static void InstanceComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FEInstancedSystem)

		void AddIndividualInstance(FEEntity* Entity, glm::mat4 InstanceMatrix);
		void DeleteIndividualInstance(FEEntity* Entity, const size_t InstanceIndex);
		void ModifyIndividualInstance(FEEntity* Entity, const size_t InstanceIndex, glm::mat4 NewMatrix);

		bool TryToSnapIndividualInstance(FEEntity* Entity, size_t InstanceIndex);

		void ClearInstance(FEEntity* Entity);

		FEAABB GetAABB(FEEntity* Entity);

		bool PopulateInstance(FEEntity* Entity, FESpawnInfo SpawnInfo);
		
		void UpdateIndividualSelectModeAABBData(FEEntity* EntityWithInstancedComponent);
		void UpdateIndividualSelectModeAABBData(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		bool IsIndividualSelectMode(FEEntity* EntityWithInstancedComponent);
		bool IsIndividualSelectMode(FEInstancedComponent& InstancedComponent);

		void SetIndividualSelectMode(FEEntity* EntityWithInstancedComponent, const bool NewValue);
		void SetIndividualSelectMode(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const bool NewValue);

		FEEntity* GetEntityWithGameModelComponent(std::string EntityID)
		{
			FEObject* Object = OBJECT_MANAGER.GetFEObject(EntityID);
			if (Object == nullptr || Object->GetType() != FE_ENTITY)
				return nullptr;

			FEEntity* Entity = reinterpret_cast<FEEntity*>(Object);
			if (Entity == nullptr || !Entity->HasComponent<FEGameModelComponent>())
				return nullptr;

			return Entity;
		}
	};

#define INSTANCED_RENDERING_SYSTEM FEInstancedSystem::getInstance()
}