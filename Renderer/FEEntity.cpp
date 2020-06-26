#include "FEEntity.h"
using namespace FocalEngine;

FEEntity::FEEntity(FEMesh* Mesh, FEMaterial* Material, std::string Name)
{
	mesh = Mesh;
	material = Material;
	name = Name;
}

FEEntity::~FEEntity()
{
}

void FEEntity::render()
{
	if (!visible)
		return;

	FE_GL_ERROR(glBindVertexArray(mesh->getVaoID()));
	if ((mesh->vertexBuffers & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	if ((mesh->vertexBuffers & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	if ((mesh->vertexBuffers & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	if ((mesh->vertexBuffers & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	if ((mesh->vertexBuffers & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));

	if ((mesh->vertexBuffers & FE_INDEX) == FE_INDEX)
		FE_GL_ERROR(glDrawElements(GL_TRIANGLES, mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	if ((mesh->vertexBuffers & FE_INDEX) != FE_INDEX)
		FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, mesh->getVertexCount()));

	if ((mesh->vertexBuffers & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glDisableVertexAttribArray(0));
	if ((mesh->vertexBuffers & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glDisableVertexAttribArray(1));
	if ((mesh->vertexBuffers & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glDisableVertexAttribArray(2));
	if ((mesh->vertexBuffers & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glDisableVertexAttribArray(3));
	if ((mesh->vertexBuffers & FE_UV) == FE_UV) FE_GL_ERROR(glDisableVertexAttribArray(4));
	FE_GL_ERROR(glBindVertexArray(0));
}

bool FEEntity::isVisible()
{
	return visible;
}

void FEEntity::setVisibility(bool isVisible)
{
	visible = isVisible;
}

std::string FEEntity::getName()
{
	return name;
}

void FEEntity::setName(std::string newName)
{
	name = newName;
}

FEAABB FEEntity::getAABB()
{
	FEAABB correctedAABB;
	correctedAABB.min = (transform.getTransformMatrix() * glm::vec4(mesh->AABB.min.x, mesh->AABB.min.y, mesh->AABB.min.z, 1.0f));
	correctedAABB.max = (transform.getTransformMatrix() * glm::vec4(mesh->AABB.max.x, mesh->AABB.max.y, mesh->AABB.max.z, 1.0f));

	return correctedAABB;
}

bool FEEntity::isCastShadows()
{
	return castShadows;
}

void FEEntity::setCastShadows(bool isCastShadows)
{
	castShadows = isCastShadows;
}

bool FEEntity::isReceivingShadows()
{
	return receiveShadows;
}

void FEEntity::setReceivingShadows(bool isReceivingShadows)
{
	receiveShadows = isReceivingShadows;
}