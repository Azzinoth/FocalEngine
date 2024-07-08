#pragma once
#include "../../Scene/FEScene.h"

namespace FocalEngine
{
	// FIX ME!
	/*struct FEDrawElementsIndirectCommand
	{
		unsigned int Count;
		unsigned int PrimCount;
		unsigned int FirstIndex;
		unsigned int BaseVertex;
		unsigned int BaseInstance;
	};*/

	class FEInstancedSystem
	{
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;
		
		SINGLETON_PRIVATE_PART(FEInstancedSystem)

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity);
		void RegisterOnComponentCallbacks();
		void OnSceneClear();

		void InitializeBuffers(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);
		void InitializeGPUCullingBuffers(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		void AddInstanceInternal(FEEntity* EntityWithInstancedComponent, glm::mat4 InstanceMatrix);
		void AddInstanceInternal(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, glm::mat4 InstanceMatrix);
		void AddInstances(FEEntity* EntityWithInstancedComponent, const glm::mat4* InstanceMatrix, size_t Count);
		void AddInstances(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const glm::mat4* InstanceMatrix, size_t Count);

		void UpdateBuffers(FEEntity* EntityWithInstancedComponent);
		void UpdateBuffers(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		void UpdateMatrices(FEEntity* EntityWithInstancedComponent);
		void UpdateMatrices(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		void Update();

		void Render(FEEntity* EntityWithInstancedComponent);
		void Render(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		void RenderOnlyBillbords(FEEntity* EntityWithInstancedComponent);
		void RenderOnlyBillbords(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		void CheckDirtyFlag(FEEntity* EntityWithInstancedComponent);
		void CheckDirtyFlag(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);
	public:
		SINGLETON_PUBLIC_PART(FEInstancedSystem)

		void AddIndividualInstance(FEEntity* EntityWithInstancedComponent, glm::mat4 InstanceMatrix);
		void AddIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, glm::mat4 InstanceMatrix);

		void DeleteIndividualInstance(FEEntity* EntityWithInstancedComponent, const size_t InstanceIndex);
		void DeleteIndividualInstance(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const size_t InstanceIndex);

		void ModifyIndividualInstance(FEEntity* EntityWithInstancedComponent, const size_t InstanceIndex, glm::mat4 NewMatrix);
		void ModifyIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const size_t InstanceIndex, glm::mat4 NewMatrix);

		bool TryToSnapIndividualInstance(FEEntity* EntityWithInstancedComponent, size_t InstanceIndex);
		bool TryToSnapIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, size_t InstanceIndex);

		void ClearInstance(FEEntity* EntityWithInstancedComponent);
		void ClearInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		FEAABB GetAABB(FEEntity* EntityWithInstancedComponent);
		FEAABB GetAABB(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		bool PopulateInstance(FEEntity* EntityWithInstancedComponent, FESpawnInfo SpawnInfo);
		bool PopulateInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, FESpawnInfo SpawnInfo);

		void UpdateIndividualSelectModeAABBData(FEEntity* EntityWithInstancedComponent);
		void UpdateIndividualSelectModeAABBData(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent);

		bool IsIndividualSelectMode(FEEntity* EntityWithInstancedComponent);
		bool IsIndividualSelectMode(FEInstancedComponent& InstancedComponent);

		void SetIndividualSelectMode(FEEntity* EntityWithInstancedComponent, const bool NewValue);
		void SetIndividualSelectMode(FEGameModelComponent& GameModelComponent, FEInstancedComponent& InstancedComponent, const bool NewValue);
	};

#define INSTANCED_RENDERING_SYSTEM FEInstancedSystem::getInstance()
}