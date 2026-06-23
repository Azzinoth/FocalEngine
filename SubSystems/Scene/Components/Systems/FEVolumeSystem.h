#pragma once
#include "../../Scene/FESceneManager.h"
#include "../../../../ResourceManager/BaseResources/FENewMaterial.h"

namespace FocalEngine
{
	struct FETransferFunctionColorPoint
	{
		float Position = 0.0f;
		glm::vec3 Color = glm::vec3(0.0f);
	};

	struct FETransferFunctionOpacityPoint
	{
		float Position = 0.0f;
		float Opacity = 0.0f;
	};

#define TRANSFER_FUNCTION_RESOLUTION 256

	struct FEPerEntityTransferFunctionData
	{
		std::vector<FETransferFunctionColorPoint> ColorPoints;
		std::vector<FETransferFunctionOpacityPoint> OpacityPoints;
	};

	class FOCAL_ENGINE_API FEVolumeSystem
	{
		friend struct FEVolumeComponent;
		friend class FEScene;
		friend class FERenderer;
		friend class FEngine;

		SINGLETON_PRIVATE_PART(FEVolumeSystem)

		bool bInternalAdd = false;
		std::vector<FEShader*> VolumetricShaders;
	private:

		std::unordered_map<std::string, FEPerEntityTransferFunctionData> EntityTransferFunctionData;

		std::vector<FETransferFunctionColorPoint> DefaultTransferFunctionColorPoints;
		std::vector<FETransferFunctionOpacityPoint> DefaultTransferFunctionOpacityPoints;

		void InitializeTransferFunctionTexture(FEEntity* Entity);

		static void OnMyComponentAdded(FEEntity* Entity);
		static void OnMyComponentDestroy(FEEntity* Entity, bool bIsSceneClearing);
		void RegisterOnComponentCallbacks();

		static void DuplicateVolumeComponent(FEEntity* SourceEntity, FEEntity* TargetEntity);

		static Json::Value VolumeComponentToJson(FEEntity* Entity);
		static void VolumeComponentFromJson(FEEntity* Entity, Json::Value Root);
	public:
		SINGLETON_PUBLIC_PART(FEVolumeSystem)

		void Render(FEEntity* Entity, FEEntity* Camera);
		bool RenderVolumeComponent(FETransformComponent& TransformComponent, FEVolumeComponent& VolumeComponent, FEEntity* Camera);

		std::vector<FEShader*> GetVolumetricShaders();

		bool DoesVolumeComponentHaveTransferFunction(FEVolumeComponent& VolumeComponent);
		std::vector<FETransferFunctionColorPoint>& GetTransferFunctionColorPoints(FEEntity* Entity);
		std::vector<FETransferFunctionOpacityPoint>& GetTransferFunctionOpacityPoints(FEEntity* Entity);
		glm::vec3 EvaluateTransferFunctionColor(FEEntity* Entity, float Position);
		float EvaluateTransferFunctionOpacity(FEEntity* Entity, float Position);
		void BakeTransferFunction(FEEntity* Entity);
		FETexture* GetTransferFunctionLookupTexture(FEEntity* Entity);
	};

#ifdef FOCAL_ENGINE_SHARED
	extern "C" __declspec(dllexport) void* GetVolumeSystem();
	#define VOLUME_SYSTEM (*static_cast<FEVolumeSystem*>(GetVolumeSystem()))
#else
	#define VOLUME_SYSTEM FEVolumeSystem::GetInstance()
#endif
}