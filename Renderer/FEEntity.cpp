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
	delete mesh;
	delete material;
}

glm::vec3 FEEntity::getPosition()
{
	return position;
}

glm::vec3 FEEntity::getRotation()
{
	return rotation;
}

glm::vec3 FEEntity::getScale()
{
	return scale;
}

void FEEntity::setPosition(glm::vec3 newPosition)
{
	position = newPosition;
	updateWorldMatrix();
}

void FEEntity::setRotation(glm::vec3 newRotation)
{
	rotation = newRotation;
	updateWorldMatrix();
}

void FEEntity::setScale(glm::vec3 newScale)
{
	scale = newScale;
	updateWorldMatrix();
}

void FEEntity::updateWorldMatrix()
{
	worldMatrix = glm::mat4(1.0);
	worldMatrix = glm::translate(worldMatrix, position);

	worldMatrix = glm::rotate(worldMatrix, (float)rotation.x * ANGLE_TORADIANS_COF, glm::vec3(1, 0, 0));
	worldMatrix = glm::rotate(worldMatrix, (float)rotation.y * ANGLE_TORADIANS_COF, glm::vec3(0, 1, 0));
	worldMatrix = glm::rotate(worldMatrix, (float)rotation.z * ANGLE_TORADIANS_COF, glm::vec3(0, 0, 1));

	worldMatrix = glm::scale(worldMatrix, glm::vec3(scale[0], scale[1], scale[2]));
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