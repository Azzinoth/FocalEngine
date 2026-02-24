#pragma once
#include "../Core/FEObject.h"

namespace FocalEngine
{
    enum ComponentVisibilityType : uint32_t
    {
        GAME_MODEL = 1 << 0,
        TERRAIN = 1 << 1,
        POINT_CLOUD = 1 << 2,
        LINES = 1 << 3,
        VOLUME = 1 << 4,
        VIRTUAL_UI = 1 << 5,

        ALL = 0xFFFFFFFF
    };

	struct FEVisibilityComponent
	{
		friend class FEComponentsTools;
       
		FEVisibilityComponent() {};

        bool IsVisible(ComponentVisibilityType Type = ALL) const;
        void SetVisible(ComponentVisibilityType Type, bool bVisible);
        void SetVisible(bool bVisible);
	private:
        uint32_t VisibleTypes = ALL;
	};
}