#include "FEEntity.h"
using namespace FocalEngine;

FEEntity::FEEntity() : FEObject(FE_ENTITY, "")
{
}

FEEntity::FEEntity(FEPrefab* Prefab, std::string Name) : FEObject(FE_ENTITY, Name)
{
	this->Prefab = Prefab;
	SetName(Name);
	if (Prefab != nullptr)
		EntityAABB = Prefab->GetAABB();
}

FEEntity::~FEEntity()
{
}

void FEEntity::Render()
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

bool FEEntity::IsVisible() const
{
	return bVisible;
}

void FEEntity::SetVisibility(const bool NewValue)
{
	bVisible = NewValue;
}

FEAABB FEEntity::GetAABB()
{
	/*if (Transform.bDirtyFlag)
	{
		EntityAABB = Prefab->GetAABB().Transform(Transform.GetTransformMatrix());
		Transform.bDirtyFlag = false;
	}*/

	// Temporary solution
	if (Prefab == nullptr)
		return EntityAABB;

	// Temporary solution
	EntityAABB = Prefab->GetAABB().Transform(Transform.GetTransformMatrix());

	return EntityAABB;
}

bool FEEntity::IsCastShadows() const
{
	return bCastShadows;
}

void FEEntity::SetCastShadows(const bool NewValue)
{
	bCastShadows = NewValue;
}

bool FEEntity::IsReceivingShadows() const
{
	return bReceiveShadows;
}

void FEEntity::SetReceivingShadows(const bool NewValue)
{
	bReceiveShadows = NewValue;
}

bool FEEntity::IsPostprocessApplied() const
{
	return bApplyPostprocess;
}

void FEEntity::SetIsPostprocessApplied(const bool NewValue)
{
	bApplyPostprocess = NewValue;
}

void FEEntity::SetWireframeMode(const bool NewValue)
{
	bWireframeMode = NewValue;
}

bool FEEntity::IsWireframeMode() const
{
	return bWireframeMode;
}

bool FEEntity::IsUniformLighting() const
{
	return bUniformLighting;
}

void FEEntity::SetUniformLighting(bool NewValue)
{
	bUniformLighting = NewValue;
}