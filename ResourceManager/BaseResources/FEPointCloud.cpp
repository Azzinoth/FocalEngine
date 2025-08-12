#include "FEPointCloud.h"
using namespace FocalEngine;

FEPointCloud::FEPointCloud() : FEObject(FE_POINT_CLOUD, "Unnamed Point Cloud") {};

FEPointCloud::~FEPointCloud()
{
	FE_GL_ERROR(glDeleteVertexArrays(1, &VaoID));
	FE_GL_ERROR(glDeleteBuffers(1, &VboID));
}

GLuint FEPointCloud::GetVboID() const
{
	return VboID;
}

GLuint FEPointCloud::GetVaoID() const
{
	return VaoID;
}

FEAABB FEPointCloud::GetAABB() const
{
	return AABB;
}

size_t FEPointCloud::GetPointCount() const
{
	return PointCount;
}

std::vector<FEPointCloudVertex> FEPointCloud::GetRawData() const
{
	std::vector<FEPointCloudVertex> RawData;

	if (PointCount == 0)
	{
		LOG.Add("FEPointCloud::GetRawData() called on object with 0 points", "FE_POINT_CLOUD", FE_LOG_WARNING);
		return RawData;
	}

	if (IsAdvancedRenderingEnabled())
	{
		if (ComputeShaderBuffer == GLuint(-1))
		{
			LOG.Add("FEPointCloud::GetRawData() called on object with invalid ComputeShaderBuffer", "FE_POINT_CLOUD", FE_LOG_WARNING);
			return RawData;
		}

		RawData.resize(PointCount);
		FE_GL_ERROR(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ComputeShaderBuffer));
		FE_GL_ERROR(glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(FEPointCloudVertex) * PointCount, RawData.data()));

		return RawData;
	}
	else
	{
		if (VaoID == GLuint(-1))
		{
			LOG.Add("FEPointCloud::GetRawData() called on object with invalid VAO", "FE_POINT_CLOUD", FE_LOG_WARNING);
			return RawData;
		}

		if (VboID == GLuint(-1))
		{
			LOG.Add("FEPointCloud::GetRawData() called on object with invalid VBO", "FE_POINT_CLOUD", FE_LOG_WARNING);
			return RawData;
		}

		RawData.resize(PointCount);
		FE_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, VboID));
		FE_GL_ERROR(glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(FEPointCloudVertex) * PointCount, RawData.data()));
	}

	return RawData;
}

bool FEPointCloud::IsAdvancedRenderingEnabled() const
{
	return bUseAdvancedRendering;
}

#include "../SubSystems/Scene/Components/Systems/FEPointCloudSystem.h"
void FEPointCloud::SetAdvancedRenderingEnabled(const bool bUseAdvancedRendering)
{
	if (POINT_CLOUD_SYSTEM.SetAdvancedRendering(this, bUseAdvancedRendering))
		this->bUseAdvancedRendering = bUseAdvancedRendering;
}

bool FEPointCloud::GetComputeShaderBuffer(GLuint& BufferIDToFill) const
{
	if (ComputeShaderBuffer == GLuint(-1))
	{
		LOG.Add("FEPointCloud::GetComputeShaderBuffer() called on object with invalid ComputeShaderBuffer", "FE_POINT_CLOUD", FE_LOG_WARNING);
		return false;
	}

	if (!bUseAdvancedRendering)
	{
		LOG.Add("FEPointCloud::GetComputeShaderBuffer() called on object with advanced rendering disabled", "FE_POINT_CLOUD", FE_LOG_WARNING);
		return false;
	}

	if (!ComputeShaderBuffers.empty())
	{
		LOG.Add("FEPointCloud::GetComputeShaderBuffer() called on object with multiple ComputeShaderBuffers", "FE_POINT_CLOUD", FE_LOG_WARNING);
		return false;
	}

	BufferIDToFill = ComputeShaderBuffer;
	return true;
}

bool FEPointCloud::GetComputeShaderBuffers(std::vector<GLuint>& OutBufferIDs) const
{
	if (ComputeShaderBuffers.empty())
	{
		LOG.Add("FEPointCloud::GetComputeShaderBuffers() called on object with empty ComputeShaderBuffers", "FE_POINT_CLOUD", FE_LOG_WARNING);
		return false;
	}

	if (!bUseAdvancedRendering)
	{
		LOG.Add("FEPointCloud::GetComputeShaderBuffers() called on object with advanced rendering disabled", "FE_POINT_CLOUD", FE_LOG_WARNING);
		return false;
	}

	OutBufferIDs = ComputeShaderBuffers;
	return true;
}