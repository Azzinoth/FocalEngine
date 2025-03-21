#pragma once

#include "..\Core\FEGeometricTools.h"
#include "..\Core\FEObject.h"

namespace FocalEngine
{
	struct FEPointCloudVertex
	{
		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;

		unsigned char R = 0;
		unsigned char G = 0;
		unsigned char B = 0;
		unsigned char A = 255;
	};

	class FEPointCloud : public FEObject
	{
		friend class FEEntity;
		friend class FERenderer;
		friend class FEResourceManager;
	public:
		FEPointCloud() : FEObject(FE_POINT_CLOUD, "Unnamed point cloud") {};
		~FEPointCloud();

		GLuint GetVboID() const;
		GLuint GetVaoID() const;

		size_t GetPointCount() const;

		FEAABB GetAABB() const;

		std::vector<FEPointCloudVertex> GetRawData() const;
	private:
		GLuint VboID = -1;
		GLuint VaoID = -1;

		size_t PointCount = 0;
		
		FEAABB AABB;
	};
}