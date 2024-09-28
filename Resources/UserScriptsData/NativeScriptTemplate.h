#include "SubSystems/FocalEngine/FENativeScriptConnector.h"
using namespace FocalEngine;

class PLACE_HOLDER : public FENativeScriptCore
{
public:
	void Awake() override;
	void OnUpdate(double DeltaTime) override;
	void OnDestroy() override;

	int ExampleVariable = 0;
};

REGISTER_SCRIPT(PLACE_HOLDER)
// Way to expose variable to an editor.
REGISTER_SCRIPT_FIELD(PLACE_HOLDER, int, ExampleVariable)
