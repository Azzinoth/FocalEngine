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

	// Mainly used as temporary storage for the point cloud data
	// when using 64 bit precision, later converted to 32 bit
	struct FEPointCloudVertexDouble
	{
		double X = 0.0;
		double Y = 0.0;
		double Z = 0.0;

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
		friend class FEPointCloudSystem;
	public:
		FEPointCloud();
		~FEPointCloud();

		static const size_t MaxBytesPerBuffer = UINT32_MAX - 1;
		static const size_t MaxPointsPerBuffer = MaxBytesPerBuffer / (sizeof(FEPointCloudVertex));

		GLuint GetVboID() const;
		GLuint GetVaoID() const;

		size_t GetPointCount() const;

		FEAABB GetAABB() const;

		std::vector<FEPointCloudVertex> GetRawData() const;

		bool IsAdvancedRenderingEnabled() const;
		void SetAdvancedRenderingEnabled(const bool bUseAdvancedRendering);

		bool GetComputeShaderBuffer(GLuint& OutBufferID) const;
		bool GetComputeShaderBuffers(std::vector<GLuint>& OutBufferIDs) const;
	private:
		GLuint VboID = -1;
		GLuint VaoID = -1;
		GLuint ComputeShaderBuffer = -1;
		std::vector<GLuint> ComputeShaderBuffers;

		bool bUseAdvancedRendering = false;

		size_t PointCount = 0;
		
		FEAABB AABB;
	};
}