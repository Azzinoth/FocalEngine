#include "FENaiveSceneGraph.h"
using namespace FocalEngine;

FENaiveSceneGraph::FENaiveSceneGraph()
{
	Root = new FENaiveSceneEntity();
}

void FENaiveSceneGraph::Clear()
{
	RemoveEntity(Root);
	delete Root;
	Root = new FENaiveSceneEntity();
}

FENaiveSceneGraph::~FENaiveSceneGraph()
{
	delete Root;
}

FENaiveSceneEntity* FENaiveSceneGraph::GetRoot() const
{
	return Root;
}

FENaiveSceneEntity* FENaiveSceneGraph::GetEntity(std::string ID)
{
	if (ID == Root->GetObjectID())
		return Root;

	return Root->GetChild(ID);
}

FENaiveSceneEntity* FENaiveSceneGraph::GetEntityByOldEntityID(std::string OldEntityID)
{
	// Root can'n have OldStyleEntity
	return Root->GetChildByOldEntityID(OldEntityID);
}

std::string FENaiveSceneGraph::AddEntity(FEObject* OldStyleEntity)
{
	FENaiveSceneEntity* NewEntity = new FENaiveSceneEntity(OldStyleEntity->GetName());
	NewEntity->OldStyleEntity = OldStyleEntity;
	Root->AddChild(NewEntity);
	return NewEntity->GetObjectID();
}

bool FENaiveSceneGraph::MoveEntity(std::string EntityID, std::string NewParentID)
{
	FENaiveSceneEntity* Entity = GetEntity(EntityID);
	FENaiveSceneEntity* NewParent = GetEntity(NewParentID);

	if (Entity == nullptr || NewParent == nullptr)
		return false;

	RemoveEntity(Entity);
	AddEntity(NewParent, Entity);

	return true;
}

void FENaiveSceneGraph::AddEntity(FENaiveSceneEntity* Entity)
{
	Root->AddChild(Entity);
}

void FENaiveSceneGraph::AddEntity(FENaiveSceneEntity* Parent, FENaiveSceneEntity* Entity)
{
	Parent->AddChild(Entity);
}

void FENaiveSceneGraph::RemoveEntity(FENaiveSceneEntity* Entity)
{
	Root->RemoveChild(Entity);
}

std::vector<FENaiveSceneEntity*> FENaiveSceneGraph::GetEntityByName(std::string Name)
{
	std::vector<FENaiveSceneEntity*> Entities;
	std::vector<FENaiveSceneEntity*> Children = Root->GetChildren();

	for (size_t i = 0; i < Children.size(); i++)
	{
		if (Children[i]->GetName() == Name)
			Entities.push_back(Children[i]);
	}

	return Entities;
}