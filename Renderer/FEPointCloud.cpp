#include "FEPointCloud.h"
using namespace FocalEngine;

//FEPointCloud::FEPointCloud(std::vector<PointCloudVertex>& RawPointCloudData, std::string Name, std::string ForceObjectID) : FEObject(FE_POINT_CLOUD, Name)
//{
//	if (!ForceObjectID.empty())
//	{
//		SetID
//		SetObjectID(ForceObjectID);
//	}
//
//	glGenBuffers(1, &VboID);
//
//	// Bind and upload vertex data to the VBO.
//	glBindBuffer(GL_ARRAY_BUFFER, PointVBO);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(PointDataWithColor) * PointsWithColor.size(), PointsWithColor.data(), GL_STATIC_DRAW);
//
//	// Create VAO.
//	glGenVertexArrays(1, &PointVAO);
//
//	// Bind and link VAO and VBO.
//	glBindVertexArray(PointVAO);
//	glBindBuffer(GL_ARRAY_BUFFER, PointVBO);
//
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PointDataWithColor), (void*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(PointDataWithColor), (void*)(3 * sizeof(float)));
//	glEnableVertexAttribArray(1);
//}

FEPointCloud::~FEPointCloud()
{
	//FE_GL_ERROR(glDeleteVertexArrays(1, &VaoID));
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
