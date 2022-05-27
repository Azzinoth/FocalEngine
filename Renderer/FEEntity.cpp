#include "FEEntity.h"
using namespace FocalEngine;

FEEntity::FEEntity() : FEObject(FE_ENTITY, "")
{
}

FEEntity::FEEntity(FEPrefab* Prefab, std::string Name) : FEObject(FE_ENTITY, Name)
{
	prefab = Prefab;
	setName(Name);
	if (prefab != nullptr)
		entityAABB = prefab->getAABB();
}

FEEntity::~FEEntity()
{
}

void FEEntity::render()
{
	//FE_GL_ERROR(glBindVertexArray(gameModel->mesh->getVaoID()));
	//if ((gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glEnableVertexAttribArray(0));
	//if ((gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glEnableVertexAttribArray(1));
	//if ((gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glEnableVertexAttribArray(2));
	//if ((gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glEnableVertexAttribArray(3));
	//if ((gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glEnableVertexAttribArray(4));
	//if ((gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glEnableVertexAttribArray(5));

	//if ((gameModel->mesh->vertexAttributes & FE_INDEX) == FE_INDEX)
	//	FE_GL_ERROR(glDrawElements(GL_TRIANGLES, gameModel->mesh->getVertexCount(), GL_UNSIGNED_INT, 0));
	//if ((gameModel->mesh->vertexAttributes & FE_INDEX) != FE_INDEX)
	//	FE_GL_ERROR(glDrawArrays(GL_TRIANGLES, 0, gameModel->mesh->getVertexCount()));

	//// could it be that this disabled part is only slowing engine down without any profit ?
	///*if ((gameModel->mesh->vertexAttributes & FE_POSITION) == FE_POSITION) FE_GL_ERROR(glDisableVertexAttribArray(0));
	//if ((gameModel->mesh->vertexAttributes & FE_COLOR) == FE_COLOR) FE_GL_ERROR(glDisableVertexAttribArray(1));
	//if ((gameModel->mesh->vertexAttributes & FE_NORMAL) == FE_NORMAL) FE_GL_ERROR(glDisableVertexAttribArray(2));
	//if ((gameModel->mesh->vertexAttributes & FE_TANGENTS) == FE_TANGENTS) FE_GL_ERROR(glDisableVertexAttribArray(3));
	//if ((gameModel->mesh->vertexAttributes & FE_UV) == FE_UV) FE_GL_ERROR(glDisableVertexAttribArray(4));
	//if ((gameModel->mesh->vertexAttributes & FE_MATINDEX) == FE_MATINDEX) FE_GL_ERROR(glDisableVertexAttribArray(5));*/
	//FE_GL_ERROR(glBindVertexArray(0));
}

bool FEEntity::isVisible()
{
	return visible;
}

void FEEntity::setVisibility(bool isVisible)
{
	visible = isVisible;
}

FEAABB FEEntity::getAABB()
{
	if (transform.dirtyFlag)
	{
		entityAABB = prefab->getAABB().transform(transform.getTransformMatrix());
		transform.dirtyFlag = false;
	}

	return entityAABB;
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

bool FEEntity::isPostprocessApplied()
{
	return applyPostprocess;
}

void FEEntity::setIsPostprocessApplied(bool isPostprocessApplied)
{
	applyPostprocess = isPostprocessApplied;
}