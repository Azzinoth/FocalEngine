#pragma once

#include "../../../Renderer/FEMaterial.h"

namespace FocalEngine
{
	class FEPhongMaterial : public FEMaterial
	{
	public:
		FEPhongMaterial(FETexture* baseColorTexture);
		~FEPhongMaterial();
	private:
	};
}