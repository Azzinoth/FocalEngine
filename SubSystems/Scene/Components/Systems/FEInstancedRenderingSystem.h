#pragma once
#include "../../Scene/FEScene.h"
#include "../../Scene/FENewEntity.h"

namespace FocalEngine
{
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
		entt::observer* Observer = nullptr;

		// This can be used later with transfrom component.
		// DELETE ME!
		//entt::observer observer{registry, entt::collector.update<TransformComponent>()};

		void InitializeObserver();

		void InitializeBuffers(FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);
		void InitializeGPUCullingBuffers(FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);

		void AddInstanceInternal(FENewEntity* EntityWithInstancedComponent, glm::mat4 InstanceMatrix);
		void AddInstanceInternal(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent, glm::mat4 InstanceMatrix);
		void AddInstances(FENewEntity* EntityWithInstancedComponent, const glm::mat4* InstanceMatrix, size_t Count);
		void AddInstances(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent, const glm::mat4* InstanceMatrix, size_t Count);

		void UpdateBuffers(FENewEntity* EntityWithInstancedComponent);
		void UpdateBuffers(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);

		void UpdateMatrices(FENewEntity* EntityWithInstancedComponent);
		void UpdateMatrices(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);

		void CheckObserver();
		void Update();

		void Render(FENewEntity* EntityWithInstancedComponent);
		void Render(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);

		void RenderOnlyBillbords(FENewEntity* EntityWithInstancedComponent);
		void RenderOnlyBillbords(FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);

		void CheckDirtyFlag(FENewEntity* EntityWithInstancedComponent);
		void CheckDirtyFlag(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);
	public:
		SINGLETON_PUBLIC_PART(FEInstancedSystem)

		void AddIndividualInstance(FENewEntity* EntityWithInstancedComponent, glm::mat4 InstanceMatrix);
		void AddIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent, glm::mat4 InstanceMatrix);

		void DeleteIndividualInstance(FENewEntity* EntityWithInstancedComponent, const size_t InstanceIndex);
		void DeleteIndividualInstance(FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent, const size_t InstanceIndex);

		void ModifyIndividualInstance(FENewEntity* EntityWithInstancedComponent, const size_t InstanceIndex, glm::mat4 NewMatrix);
		void ModifyIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent, const size_t InstanceIndex, glm::mat4 NewMatrix);

		bool TryToSnapIndividualInstance(FENewEntity* EntityWithInstancedComponent, size_t InstanceIndex);
		bool TryToSnapIndividualInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent, size_t InstanceIndex);

		void ClearInstance(FENewEntity* EntityWithInstancedComponent);
		void ClearInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);

		FEAABB GetAABB(FENewEntity* EntityWithInstancedComponent);
		FEAABB GetAABB(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);

		bool PopulateInstance(FENewEntity* EntityWithInstancedComponent, FESpawnInfo SpawnInfo);
		bool PopulateInstance(FETransformComponent& TransformComponent, FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent, FESpawnInfo SpawnInfo);

		void UpdateIndividualSelectModeAABBData(FENewEntity* EntityWithInstancedComponent);
		void UpdateIndividualSelectModeAABBData(FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent);

		bool IsIndividualSelectMode(FENewEntity* EntityWithInstancedComponent);
		bool IsIndividualSelectMode(FEInstancedRenderingComponent& InstancedComponent);

		void SetIndividualSelectMode(FENewEntity* EntityWithInstancedComponent, const bool NewValue);
		void SetIndividualSelectMode(FEGameModelComponent& GameModelComponent, FEInstancedRenderingComponent& InstancedComponent, const bool NewValue);
	};

#define INSTANCED_RENDERING_SYSTEM FEInstancedSystem::getInstance()
}