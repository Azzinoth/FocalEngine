#include "SubSystems/FocalEngine/SubSystems/PluginSystem/FEPluginConnector.h"
using namespace FocalEngine;

class PLACE_HOLDER : public FEPluginCore
{
public:
	bool OnLoad() override;
	void OnUnload() override;
	void OnUpdate(double DeltaTime) override;
};