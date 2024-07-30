#include "SceneGraphTest.h"
using namespace FocalEngine;

// Using a fixed seed to make it easier to debug.
#define RANDOM_SEED 42
#define RANDOM_ACTIONS_ITERATIONS 1000

TEST(SceneGraph, Check_Basic_Add_Find_Delete_Nodes)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	// Get root node from the scene returns valid node.
	FENaiveSceneGraphNode* RootNode = CurrentScene->SceneGraph.GetRoot();
	ASSERT_NE(RootNode, nullptr);

	// Root node can be found by its ID.
	FENaiveSceneGraphNode* RootNodeByID = CurrentScene->SceneGraph.GetNode(RootNode->GetObjectID());
	ASSERT_EQ(RootNode, RootNodeByID);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);

	// Root node can not be deleted.
	CurrentScene->SceneGraph.DeleteNode(RootNode);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	ASSERT_NE(CurrentScene->SceneGraph.GetNode(RootNode->GetObjectID()), nullptr);

	// Add a new node to the CurrentScene->
	FEEntity* Node_A = CurrentScene->CreateEntity("Node_A");
	// Temporary using old style entities.
	std::string Node_A_ID = CurrentScene->SceneGraph.GetNodeByEntityID(Node_A->GetObjectID())->GetObjectID();
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 1);

	// New node could be found by its ID.
	FENaiveSceneGraphNode* NodeByID = CurrentScene->SceneGraph.GetNode(Node_A_ID);
	ASSERT_NE(NodeByID, nullptr);

	// Delete the new node.
	CurrentScene->SceneGraph.DeleteNode(NodeByID);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);

	// Check that deleted node can not be found.
	ASSERT_EQ(CurrentScene->SceneGraph.GetNode(Node_A_ID), nullptr);

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST(SceneGraph, Check_For_Cycles)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);

	// Create a few nodes.
	FEEntity* Node_A = CurrentScene->CreateEntity("Node_A");
	FEEntity* Node_B = CurrentScene->CreateEntity("Node_B");
	FEEntity* Node_C = CurrentScene->CreateEntity("Node_C");

	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 3);

	// Temporary using old style entities.
	std::string Node_A_ID = CurrentScene->SceneGraph.GetNodeByEntityID(Node_A->GetObjectID())->GetObjectID();
	std::string Node_B_ID = CurrentScene->SceneGraph.GetNodeByEntityID(Node_B->GetObjectID())->GetObjectID();
	std::string Node_C_ID = CurrentScene->SceneGraph.GetNodeByEntityID(Node_C->GetObjectID())->GetObjectID();

	FENaiveSceneGraphNode* NodeA = CurrentScene->SceneGraph.GetNode(Node_A_ID);
	FENaiveSceneGraphNode* NodeB = CurrentScene->SceneGraph.GetNode(Node_B_ID);
	FENaiveSceneGraphNode* NodeC = CurrentScene->SceneGraph.GetNode(Node_C_ID);

	// Create a valid hierarchy.
	CurrentScene->SceneGraph.MoveNode(NodeB->GetObjectID(), NodeA->GetObjectID());
	CurrentScene->SceneGraph.MoveNode(NodeC->GetObjectID(), NodeB->GetObjectID());

	// Check that there are no cycles in this valid hierarchy.
	ASSERT_FALSE(CurrentScene->SceneGraph.HasCycle(NodeA));
	ASSERT_FALSE(CurrentScene->SceneGraph.HasCycle(NodeB));
	ASSERT_FALSE(CurrentScene->SceneGraph.HasCycle(NodeC));

	// Try to create a cycle.
	ASSERT_FALSE(CurrentScene->SceneGraph.MoveNode(NodeA->GetObjectID(), NodeC->GetObjectID()));

	// Verify that the cycle was not created.
	ASSERT_FALSE(CurrentScene->SceneGraph.HasCycle(NodeA));
	ASSERT_FALSE(CurrentScene->SceneGraph.HasCycle(NodeB));
	ASSERT_FALSE(CurrentScene->SceneGraph.HasCycle(NodeC));

	// Try to create a self-cycle.
	ASSERT_FALSE(CurrentScene->SceneGraph.MoveNode(NodeA->GetObjectID(), NodeA->GetObjectID()));

	// Verify that the self-cycle was not created.
	ASSERT_FALSE(CurrentScene->SceneGraph.HasCycle(NodeA));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_GetNodeCount_AND_ChildCount_Functions)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// Check root node (0)
	ASSERT_EQ(Nodes[0]->GetImediateChildrenCount(), 3);
	ASSERT_EQ(Nodes[0]->GetRecursiveChildCount(), 29);

	// Check level 1 nodes (1, 2, 3)
	ASSERT_EQ(Nodes[1]->GetImediateChildrenCount(), 3);
	ASSERT_EQ(Nodes[1]->GetRecursiveChildCount(), 8);
	ASSERT_EQ(Nodes[2]->GetImediateChildrenCount(), 3);
	ASSERT_EQ(Nodes[2]->GetRecursiveChildCount(), 8);
	ASSERT_EQ(Nodes[3]->GetImediateChildrenCount(), 3);
	ASSERT_EQ(Nodes[3]->GetRecursiveChildCount(), 10);

	// Check some level 2 nodes (4, 5, 6, 7, 8, 9, 10, 11)
	ASSERT_EQ(Nodes[4]->GetImediateChildrenCount(), 2);
	ASSERT_EQ(Nodes[4]->GetRecursiveChildCount(), 3);
	ASSERT_EQ(Nodes[5]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[5]->GetRecursiveChildCount(), 2);
	ASSERT_EQ(Nodes[6]->GetImediateChildrenCount(), 0);
	ASSERT_EQ(Nodes[6]->GetRecursiveChildCount(), 0);
	ASSERT_EQ(Nodes[7]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[7]->GetRecursiveChildCount(), 2);
	ASSERT_EQ(Nodes[8]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[8]->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Nodes[9]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[9]->GetRecursiveChildCount(), 2);
	ASSERT_EQ(Nodes[10]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[10]->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Nodes[11]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[11]->GetRecursiveChildCount(), 2);
	ASSERT_EQ(Nodes[12]->GetImediateChildrenCount(), 2);
	ASSERT_EQ(Nodes[12]->GetRecursiveChildCount(), 4);

	// Check some level 3 nodes (13, 14, 15, 16, 17, 18, 19, 20, 21 22)
	ASSERT_EQ(Nodes[13]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[13]->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Nodes[14]->GetImediateChildrenCount(), 0);
	ASSERT_EQ(Nodes[14]->GetRecursiveChildCount(), 0);
	ASSERT_EQ(Nodes[15]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[15]->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Nodes[16]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[16]->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Nodes[17]->GetImediateChildrenCount(), 0);
	ASSERT_EQ(Nodes[17]->GetRecursiveChildCount(), 0);
	ASSERT_EQ(Nodes[18]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[18]->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Nodes[19]->GetImediateChildrenCount(), 0);
	ASSERT_EQ(Nodes[19]->GetRecursiveChildCount(), 0);
	ASSERT_EQ(Nodes[20]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[20]->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Nodes[21]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[21]->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Nodes[22]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Nodes[22]->GetRecursiveChildCount(), 1);

	// Check some level 4 nodes (23, 24, 25, 26, 27, 28, 29) (leaf nodes)
	for (size_t i = 23; i <= 29; i++)
	{
		ASSERT_EQ(Nodes[i]->GetImediateChildrenCount(), 0);
		ASSERT_EQ(Nodes[i]->GetRecursiveChildCount(), 0);
	}

	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_MoveNode_Function)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// First simple move
	ASSERT_TRUE(CurrentScene->SceneGraph.MoveNode(Nodes[15]->GetObjectID(), Nodes[10]->GetObjectID()));
	ASSERT_EQ(Nodes[5]->GetImediateChildrenCount(), 0);
	ASSERT_EQ(Nodes[5]->GetRecursiveChildCount(), 0);
	ASSERT_EQ(Nodes[10]->GetImediateChildrenCount(), 2);
	ASSERT_EQ(Nodes[10]->GetRecursiveChildCount(), 3);
	ASSERT_EQ(Nodes[3]->GetRecursiveChildCount(), 12);
	ASSERT_EQ(Nodes[1]->GetRecursiveChildCount(), 6);

	srand(RANDOM_SEED);
	for (size_t i = 0; i < RANDOM_ACTIONS_ITERATIONS; i++)
	{
		size_t RandomIndexOfNodeToMove = rand() % CurrentScene->SceneGraph.GetNodeCount();
		size_t RandomIndexOfNewParent = rand() % CurrentScene->SceneGraph.GetNodeCount();

		size_t NodeToMoveChildrenCount = Nodes[RandomIndexOfNodeToMove]->GetImediateChildrenCount();
		size_t NodeToMoveRecursiveChildCount = Nodes[RandomIndexOfNodeToMove]->GetRecursiveChildCount();

		size_t NewParentChildrenCount = Nodes[RandomIndexOfNewParent]->GetImediateChildrenCount();
		size_t NewParentRecursiveChildCount = Nodes[RandomIndexOfNewParent]->GetRecursiveChildCount();

		bool bParentWasDescendant = CurrentScene->SceneGraph.IsDescendant(Nodes[RandomIndexOfNewParent], Nodes[RandomIndexOfNodeToMove]);
		if (CurrentScene->SceneGraph.MoveNode(Nodes[RandomIndexOfNodeToMove]->GetObjectID(), Nodes[RandomIndexOfNewParent]->GetObjectID()))
		{
			// If node was moved, check if the counts are correct.
			ASSERT_EQ(Nodes[RandomIndexOfNodeToMove]->GetImediateChildrenCount(), NodeToMoveChildrenCount);
			ASSERT_EQ(Nodes[RandomIndexOfNodeToMove]->GetRecursiveChildCount(), NodeToMoveRecursiveChildCount);

			ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetImediateChildrenCount(), NewParentChildrenCount + 1);
			if (bParentWasDescendant)
			{
				ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetRecursiveChildCount(), NewParentRecursiveChildCount);
			}
			else
			{
				ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetRecursiveChildCount(), NewParentRecursiveChildCount + NodeToMoveRecursiveChildCount + 1);
			}
		}
		else
		{
			// If node was not moved, check if the counts are the same.
			ASSERT_EQ(Nodes[RandomIndexOfNodeToMove]->GetImediateChildrenCount(), NodeToMoveChildrenCount);
			ASSERT_EQ(Nodes[RandomIndexOfNodeToMove]->GetRecursiveChildCount(), NodeToMoveRecursiveChildCount);

			ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetImediateChildrenCount(), NewParentChildrenCount);
			ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetRecursiveChildCount(), NewParentRecursiveChildCount);
		}

		// Check if the scene graph still have same number of nodes.
		ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);
	}

	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);
	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Node_AddChild_Function)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// Try to add a child that is nullptr
	Nodes[0]->AddChild(nullptr);
	ASSERT_EQ(Nodes[0]->GetImediateChildrenCount(), 3);

	// Try to add a child to it self
	Nodes[0]->AddChild(Nodes[0]);
	ASSERT_EQ(Nodes[0]->GetImediateChildrenCount(), 3);

	// Try to add a child that is already a child
	Nodes[0]->AddChild(Nodes[1]);
	ASSERT_EQ(Nodes[0]->GetImediateChildrenCount(), 3);

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Extensive_Node_Manipulation)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	const size_t MaxNodes = 10000;

	srand(RANDOM_SEED);
	for (size_t i = 0; i < RANDOM_ACTIONS_ITERATIONS; i++)
	{
		size_t RandomAction = rand() % 3;
		while ((CurrentScene->SceneGraph.GetNodeCount() >= MaxNodes && RandomAction == 0) ||
			(CurrentScene->SceneGraph.GetNodeCount() == 0 && RandomAction != 0))
		{
			RandomAction = rand() % 2;
		}
		size_t OldNodeCount = CurrentScene->SceneGraph.GetNodeCount();

		// Add a new node
		if (RandomAction == 0)
		{
			size_t AmountOfNodesToAdd = rand() % 5 + 1;
			for (size_t i = 0; i < AmountOfNodesToAdd; i++)
			{
				FEEntity* Entity = CurrentScene->CreateEntity("Node_" + std::to_string(CurrentScene->SceneGraph.GetNodeCount()));
				Nodes.push_back(CurrentScene->SceneGraph.GetNodeByEntityID(Entity->GetObjectID()));
			}

			ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), OldNodeCount + AmountOfNodesToAdd);
		}
		// Move a node
		else if (RandomAction == 1)
		{
			size_t RandomIndexOfNodeToMove = rand() % CurrentScene->SceneGraph.GetNodeCount();
			size_t RandomIndexOfNewParent = rand() % CurrentScene->SceneGraph.GetNodeCount();

			size_t NodeToMoveChildrenCount = Nodes[RandomIndexOfNodeToMove]->GetImediateChildrenCount();
			size_t NodeToMoveRecursiveChildCount = Nodes[RandomIndexOfNodeToMove]->GetRecursiveChildCount();

			size_t NewParentChildrenCount = Nodes[RandomIndexOfNewParent]->GetImediateChildrenCount();
			size_t NewParentRecursiveChildCount = Nodes[RandomIndexOfNewParent]->GetRecursiveChildCount();

			bool bParentWasDescendant = CurrentScene->SceneGraph.IsDescendant(Nodes[RandomIndexOfNewParent], Nodes[RandomIndexOfNodeToMove]);
			if (CurrentScene->SceneGraph.MoveNode(Nodes[RandomIndexOfNodeToMove]->GetObjectID(), Nodes[RandomIndexOfNewParent]->GetObjectID()))
			{
				// If node was moved, check if the counts are correct.
				ASSERT_EQ(Nodes[RandomIndexOfNodeToMove]->GetImediateChildrenCount(), NodeToMoveChildrenCount);
				ASSERT_EQ(Nodes[RandomIndexOfNodeToMove]->GetRecursiveChildCount(), NodeToMoveRecursiveChildCount);

				ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetImediateChildrenCount(), NewParentChildrenCount + 1);
				if (bParentWasDescendant)
				{
					ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetRecursiveChildCount(), NewParentRecursiveChildCount);
				}
				else
				{
					ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetRecursiveChildCount(), NewParentRecursiveChildCount + NodeToMoveRecursiveChildCount + 1);
				}
			}
			else
			{
				// If node was not moved, check if the counts are the same.
				ASSERT_EQ(Nodes[RandomIndexOfNodeToMove]->GetImediateChildrenCount(), NodeToMoveChildrenCount);
				ASSERT_EQ(Nodes[RandomIndexOfNodeToMove]->GetRecursiveChildCount(), NodeToMoveRecursiveChildCount);

				ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetImediateChildrenCount(), NewParentChildrenCount);
				ASSERT_EQ(Nodes[RandomIndexOfNewParent]->GetRecursiveChildCount(), NewParentRecursiveChildCount);
			}

			// Check if the scene graph still have same number of nodes.
			ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), OldNodeCount);
		}
		// Delete a node
		else if (RandomAction == 2)
		{
			size_t RandomIndexOfNodeToDelete = rand() % CurrentScene->SceneGraph.GetNodeCount();
			FENaiveSceneGraphNode* Parent = Nodes[RandomIndexOfNodeToDelete]->GetParent();
			size_t ParentChildrenCount = Parent->GetImediateChildrenCount();
			size_t ParentRecursiveChildCount = Parent->GetRecursiveChildCount();

			FENaiveSceneGraphNode* NodeToDelete = Nodes[RandomIndexOfNodeToDelete];
			size_t NodeToDeleteChildrenCount = NodeToDelete->GetImediateChildrenCount();
			size_t NodeToDeleteRecursiveChildCount = NodeToDelete->GetRecursiveChildCount();

			std::vector<FENaiveSceneGraphNode*> AllChildren = NodeToDelete->GetRecursiveChildren();
			Nodes.erase(Nodes.begin() + RandomIndexOfNodeToDelete);
			for (size_t j = 0; j < AllChildren.size(); j++)
			{
				for (size_t k = 0; k < Nodes.size(); k++)
				{
					if (Nodes[k] == AllChildren[j])
					{
						Nodes.erase(Nodes.begin() + k);
						break;
					}
				}
			}
			ASSERT_EQ(Nodes.size(), OldNodeCount - NodeToDeleteRecursiveChildCount - 1);

			CurrentScene->SceneGraph.DeleteNode(NodeToDelete);

			// Check if the counts are correct.
			ASSERT_EQ(Parent->GetImediateChildrenCount(), ParentChildrenCount - 1);
			ASSERT_EQ(Parent->GetRecursiveChildCount(), ParentRecursiveChildCount - NodeToDeleteRecursiveChildCount - 1);

			ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), OldNodeCount - NodeToDeleteRecursiveChildCount - 1);
		}
	}

	//test = CurrentScene->SceneGraph.ToJson().toStyledString();

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_IsDescendant_Function)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// Test direct parent-child relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[1]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[2]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[3]));

	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[1], Nodes[4]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[1], Nodes[5]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[1], Nodes[6]));

	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[15], Nodes[24]));

	// Test grandparent relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[5]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[8]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[12]));

	// Test great-grandparent relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[2], Nodes[26]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[3], Nodes[29]));

	// Test long-distance relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[23]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[24]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[25]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[26]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[27]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[28]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[29]));

	// Test non-descendant relationships
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[1], Nodes[25]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[1], Nodes[7]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[1], Nodes[3]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[2], Nodes[24]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[2], Nodes[10]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[2], Nodes[23]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[1], Nodes[2]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[2], Nodes[1]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[23], Nodes[24]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[24], Nodes[23]));

	// Test self-relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], Nodes[0]));

	// Test with null nodes
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(nullptr, Nodes[0]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(Nodes[0], nullptr));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(nullptr, nullptr));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Position_Inheritance_Propagation)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphTinySize(CurrentScene);
	FEEntity* Entity_0 = reinterpret_cast<FEEntity*>(Nodes[0]->GetEntity());
	FEEntity* Entity_1 = reinterpret_cast<FEEntity*>(Nodes[1]->GetEntity());

	// Update only the parent node's transformation.
	Entity_0->GetComponent<FETransformComponent>().SetPosition(glm::vec3(14.0f, 18.0f, -5.0f));

	// Force update
	ENGINE.BeginFrame();
	ENGINE.Render();
	ENGINE.EndFrame();

	// Check if the position is inherited by the child nodes.
	glm::dvec3 ExpectedPosition = glm::dvec3(14.000000, 18.000000, -5.000000);
	glm::mat4 ChildTransformMatrix = Entity_1->GetComponent<FETransformComponent>().GetWorldMatrix();
	glm::dvec3 ActualPosition;
	glm::dquat ActualRotation;
	glm::dvec3 ActualScale;
	GEOMETRY.DecomposeMatrixToTranslationRotationScale(ChildTransformMatrix, ActualPosition, ActualRotation, ActualScale);
	// Code to generate the expected values
	//std::string Output = "glm::dvec3 ExpectedPosition = glm::dvec3(" + std::to_string(ActualPosition.x) + ", " + std::to_string(ActualPosition.y) + ", " + std::to_string(ActualPosition.z) + ");\n";
	//Output += "glm::dquat ExpectedRotation = glm::dquat(" + std::to_string(ActualRotation.w) + ", " + std::to_string(ActualRotation.x) + ", " + std::to_string(ActualRotation.y) + ", " + std::to_string(ActualRotation.z) + ");\n";
	//Output += "glm::dvec3 ExpectedScale = glm::dvec3(" + std::to_string(ActualScale.x) + ", " + std::to_string(ActualScale.y) + ", " + std::to_string(ActualScale.z) + ");\n";
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Rotation_Inheritance_Propagation)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphTinySize(CurrentScene);
	FEEntity* Entity_0 = reinterpret_cast<FEEntity*>(Nodes[0]->GetEntity());
	FEEntity* Entity_1 = reinterpret_cast<FEEntity*>(Nodes[1]->GetEntity());

	// Update parent node's rotation
	glm::quat Rotation = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0));
	Entity_0->GetComponent<FETransformComponent>().SetQuaternion(Rotation);

	// Force update
	ENGINE.BeginFrame();
	ENGINE.Render();
	ENGINE.EndFrame();

	// Check if rotation is inherited by child nodes
	glm::dquat ExpectedRotation = glm::dquat(0.923880, 0.000000, 0.382683, 0.000000);
	glm::mat4 ChildTransformMatrix = Entity_1->GetComponent<FETransformComponent>().GetWorldMatrix();
	glm::dvec3 ActualPosition;
	glm::dquat ActualRotation;
	glm::dvec3 ActualScale;
	GEOMETRY.DecomposeMatrixToTranslationRotationScale(ChildTransformMatrix, ActualPosition, ActualRotation, ActualScale);
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Scale_Inheritance_Propagation)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphTinySize(CurrentScene);
	FEEntity* Entity_0 = reinterpret_cast<FEEntity*>(Nodes[0]->GetEntity());
	FEEntity* Entity_1 = reinterpret_cast<FEEntity*>(Nodes[1]->GetEntity());

	// Update parent node's scale
	Entity_0->GetComponent<FETransformComponent>().SetScale(glm::vec3(2.0f, 2.0f, 2.0f));

	// Force update
	ENGINE.BeginFrame();
	ENGINE.Render();
	ENGINE.EndFrame();

	// Check if scale is inherited by child nodes
	glm::dvec3 ExpectedScale = glm::dvec3(2.000000, 2.000000, 2.000000);
	glm::mat4 ChildTransformMatrix = Entity_1->GetComponent<FETransformComponent>().GetWorldMatrix();
	glm::dvec3 ActualPosition;
	glm::dquat ActualRotation;
	glm::dvec3 ActualScale;
	GEOMETRY.DecomposeMatrixToTranslationRotationScale(ChildTransformMatrix, ActualPosition, ActualRotation, ActualScale);
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Multi_Level_Inheritance_Propagation_Tiny)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphTinySize(CurrentScene);

	FEEntity* Entity_0 = reinterpret_cast<FEEntity*>(Nodes[0]->GetEntity());
	FEEntity* Entity_1 = reinterpret_cast<FEEntity*>(Nodes[1]->GetEntity());
	FEEntity* Entity_2 = reinterpret_cast<FEEntity*>(Nodes[2]->GetEntity());
	FEEntity* Entity_3 = reinterpret_cast<FEEntity*>(Nodes[3]->GetEntity());

	// Apply transformations to various nodes
	Entity_0->GetComponent<FETransformComponent>().SetPosition(glm::vec3(10.0f, 10.0f, 10.0f));
	Entity_1->GetComponent<FETransformComponent>().SetQuaternion(glm::angleAxis(glm::radians(30.0f), glm::vec3(0, 1, 0)));
	Entity_2->GetComponent<FETransformComponent>().SetScale(glm::vec3(1.5f, 1.5f, 1.5f));

	// Force update
	ENGINE.BeginFrame();
	ENGINE.Render();
	ENGINE.EndFrame();

	// Check if transformations are correctly propagated to the deepest child
	glm::mat4 ExpectedMatrix = glm::mat4(1.299038, 0.000000, -0.750000, 0.000000,
										 0.000000, 1.500000, 0.000000, 0.000000,
										 0.750000, 0.000000, 1.299038, 0.000000,
										 10.000000, 10.000000, 10.000000, 1.000000);

	glm::mat4 ActualMatrix = Entity_3->GetComponent<FETransformComponent>().GetWorldMatrix();

	// Code to generate the expected matrix
	/*std::string Output = "glm::mat4 ExpectedMatrix = glm::mat4(";
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			Output += std::to_string(ActualMatrix[i][j]);
			if (i < 3 || j < 3)
			{
				Output += ", ";
			}
		}
	}
	Output += ")";*/
	
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ExpectedMatrix, ActualMatrix));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Multi_Level_Inheritance_Propagation_Small)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphSmallSize(CurrentScene);

	FEEntity* Entity_0 = reinterpret_cast<FEEntity*>(Nodes[0]->GetEntity());
	FEEntity* Entity_1 = reinterpret_cast<FEEntity*>(Nodes[1]->GetEntity());
	FEEntity* Entity_2 = reinterpret_cast<FEEntity*>(Nodes[2]->GetEntity());
	FEEntity* Entity_6 = reinterpret_cast<FEEntity*>(Nodes[6]->GetEntity());
	FEEntity* Entity_13 = reinterpret_cast<FEEntity*>(Nodes[13]->GetEntity());

	// Apply transformations to various nodes
	Entity_0->GetComponent<FETransformComponent>().SetPosition(glm::vec3(10.0f, 10.0f, 10.0f));
	Entity_1->GetComponent<FETransformComponent>().SetQuaternion(glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0)));
	Entity_2->GetComponent<FETransformComponent>().SetScale(glm::vec3(2.0f, 2.0f, 2.0f));
	Entity_6->GetComponent<FETransformComponent>().SetPosition(glm::vec3(5.0f, 0.0f, 0.0f));

	// Force update
	ENGINE.BeginFrame();
	ENGINE.Render();
	ENGINE.EndFrame();

	// Check if transformations are correctly propagated to the deepest child
	glm::mat4 ExpectedMatrix = glm::mat4(2.000000, 0.000000, 0.000000, 0.000000,
										 0.000000, 2.000000, 0.000000, 0.000000,
										 0.000000, 0.000000, 2.000000, 0.000000,
										 20.000000, 10.000000, 10.000000, 1.000000);

	glm::mat4 ActualMatrix = Entity_13->GetComponent<FETransformComponent>().GetWorldMatrix();
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ExpectedMatrix, ActualMatrix));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Multi_Level_Inheritance_Propagation_Medium)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);

	FEEntity* Entity_0 = reinterpret_cast<FEEntity*>(Nodes[0]->GetEntity());
	FEEntity* Entity_1 = reinterpret_cast<FEEntity*>(Nodes[1]->GetEntity());
	FEEntity* Entity_2 = reinterpret_cast<FEEntity*>(Nodes[2]->GetEntity());
	FEEntity* Entity_7 = reinterpret_cast<FEEntity*>(Nodes[7]->GetEntity());
	FEEntity* Entity_16 = reinterpret_cast<FEEntity*>(Nodes[16]->GetEntity());
	FEEntity* Entity_25 = reinterpret_cast<FEEntity*>(Nodes[25]->GetEntity());

	// Apply transformations to various nodes
	Entity_0->GetComponent<FETransformComponent>().SetPosition(glm::vec3(10.2f, -10.0f, 0.124f));
	Entity_1->GetComponent<FETransformComponent>().SetQuaternion(glm::angleAxis(glm::radians(47.8f), glm::vec3(0, 1, 0)));
	Entity_2->GetComponent<FETransformComponent>().SetScale(glm::vec3(2.2f, 2.2f, 2.2f));
	Entity_7->GetComponent<FETransformComponent>().SetPosition(glm::vec3(5.0f, 0.5f, -4.3f));
	Entity_16->GetComponent<FETransformComponent>().SetQuaternion(glm::angleAxis(glm::radians(67.0f), glm::vec3(1, 0, 0)));

	// Force update
	ENGINE.BeginFrame();
	ENGINE.Render();
	ENGINE.EndFrame();

	// Check if transformations are correctly propagated to the deepest child
	glm::mat4 ExpectedMatrix = glm::mat4(2.200000, 0.000000, 0.000000, 0.000000,
										 0.000000, 0.859609, 2.025111, 0.000000,
										 0.000000, -2.025111, 0.859609, 0.000000,
										 21.200001, -8.900000, -9.336001, 1.000000);

	glm::mat4 ActualMatrix = Entity_25->GetComponent<FETransformComponent>().GetWorldMatrix();
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ExpectedMatrix, ActualMatrix));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

void SceneGraphTest::SetUp() {}

void SceneGraphTest::TearDown()
{
	//SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

std::vector<FENaiveSceneGraphNode*> SceneGraphTest::PopulateSceneGraph(FEScene* SceneToWorkWith, size_t NodeCount)
{
	std::vector<FENaiveSceneGraphNode*> Nodes;
	for (size_t i = 0; i < NodeCount; i++)
	{
		FEEntity* Entity = SceneToWorkWith->CreateEntity("Node_" + std::to_string(i));
		Nodes.push_back(SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity->GetObjectID()));
	}

	return Nodes;
}

std::vector<FENaiveSceneGraphNode*> SceneGraphTest::PopulateSceneGraphTinySize(FEScene* SceneToWorkWith)
{
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraph(SceneToWorkWith, 6);


	// Create a hierarchy:
	//
	//           0  
	//           | 
    //           1   
	//          / \   
    //         4   2
	//              \
	//               3
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[1]->GetObjectID(), Nodes[0]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[2]->GetObjectID(), Nodes[1]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[3]->GetObjectID(), Nodes[2]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[4]->GetObjectID(), Nodes[1]->GetObjectID());

	return Nodes;
}

std::vector<FENaiveSceneGraphNode*> SceneGraphTest::PopulateSceneGraphSmallSize(FEScene* SceneToWorkWith)
{
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraph(SceneToWorkWith, 15);

	// Create a hierarchy:
	//
	//             0
	//            / \
    //           1   2
	//          /|\   \
    //         3 4 5   6
	//        /  |     |\
    //       7   8     9 10
	//      / \       / \
    //     11 12     13 14

	SceneToWorkWith->SceneGraph.MoveNode(Nodes[1]->GetObjectID(), Nodes[0]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[2]->GetObjectID(), Nodes[0]->GetObjectID());

	SceneToWorkWith->SceneGraph.MoveNode(Nodes[3]->GetObjectID(), Nodes[1]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[4]->GetObjectID(), Nodes[1]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[5]->GetObjectID(), Nodes[1]->GetObjectID());

	SceneToWorkWith->SceneGraph.MoveNode(Nodes[6]->GetObjectID(), Nodes[2]->GetObjectID());

	SceneToWorkWith->SceneGraph.MoveNode(Nodes[7]->GetObjectID(), Nodes[3]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[8]->GetObjectID(), Nodes[4]->GetObjectID());

	SceneToWorkWith->SceneGraph.MoveNode(Nodes[9]->GetObjectID(), Nodes[6]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[10]->GetObjectID(), Nodes[6]->GetObjectID());

	SceneToWorkWith->SceneGraph.MoveNode(Nodes[11]->GetObjectID(), Nodes[7]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[12]->GetObjectID(), Nodes[7]->GetObjectID());

	SceneToWorkWith->SceneGraph.MoveNode(Nodes[13]->GetObjectID(), Nodes[9]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[14]->GetObjectID(), Nodes[9]->GetObjectID());

	return Nodes;
}

std::vector<FENaiveSceneGraphNode*> SceneGraphTest::PopulateSceneGraphMediumSize(FEScene* SceneToWorkWith)
{
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraph(SceneToWorkWith, 30);

	// Create a hierarchy:
	//
	//                  0
	//          /       |       \
    //         1        2        3
	//       / | \    / | \    / | \
    //      4  5  6  7  8  9  10 11 12
	//     /\  |     |  |  |   |  |  |\
    //   13 14 15   16 17 18  19 20 21 22
	//   |     |     |     |      |  |  \
    //  23    24    25    26      27 28  29

	// Level 1
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[1]->GetObjectID(), Nodes[0]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[2]->GetObjectID(), Nodes[0]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[3]->GetObjectID(), Nodes[0]->GetObjectID());

	// Level 2
	for (int i = 1; i <= 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			SceneToWorkWith->SceneGraph.MoveNode(Nodes[3 * i + j + 1]->GetObjectID(), Nodes[i]->GetObjectID());
		}
	}

	// Level 3
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[13]->GetObjectID(), Nodes[4]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[14]->GetObjectID(), Nodes[4]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[15]->GetObjectID(), Nodes[5]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[16]->GetObjectID(), Nodes[7]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[17]->GetObjectID(), Nodes[8]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[18]->GetObjectID(), Nodes[9]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[19]->GetObjectID(), Nodes[10]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[20]->GetObjectID(), Nodes[11]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[21]->GetObjectID(), Nodes[12]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[22]->GetObjectID(), Nodes[12]->GetObjectID());

	// Level 4
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[23]->GetObjectID(), Nodes[13]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[24]->GetObjectID(), Nodes[15]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[25]->GetObjectID(), Nodes[16]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[26]->GetObjectID(), Nodes[18]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[27]->GetObjectID(), Nodes[20]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[28]->GetObjectID(), Nodes[21]->GetObjectID());
	SceneToWorkWith->SceneGraph.MoveNode(Nodes[29]->GetObjectID(), Nodes[22]->GetObjectID());

	return Nodes;
}

void SceneGraphTest::TestTransformationComponentAfterChildAdded(FEScene* SceneToWorkWith, const std::string& ComponentType, const glm::vec3& InitialParentTransform, const glm::vec3& InitialChildTransform)
{
	FEEntity* Entity_A = SceneToWorkWith->CreateEntity("A");
	FEEntity* Entity_B = SceneToWorkWith->CreateEntity("B");

	// Set transformations based on type
	if (ComponentType == "POSITION")
	{
		Entity_A->GetComponent<FETransformComponent>().SetPosition(InitialParentTransform);
		Entity_B->GetComponent<FETransformComponent>().SetPosition(InitialChildTransform);
	}
	else if (ComponentType == "ROTATION")
	{
		Entity_A->GetComponent<FETransformComponent>().SetQuaternion(glm::quat(InitialParentTransform));
		Entity_B->GetComponent<FETransformComponent>().SetQuaternion(glm::quat(InitialChildTransform));
	}
	else if (ComponentType == "SCALE")
	{
		Entity_A->GetComponent<FETransformComponent>().SetScale(InitialParentTransform);
		Entity_B->GetComponent<FETransformComponent>().SetScale(InitialChildTransform);
	}

	// Temporary using old style entities.
	std::string Node_A_ID = SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity_A->GetObjectID())->GetObjectID();
	std::string Node_B_ID = SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity_B->GetObjectID())->GetObjectID();

	SceneToWorkWith->SceneGraph.MoveNode(Node_B_ID, Node_A_ID);

	ASSERT_TRUE(ValidateTransformConsistency(Entity_B->GetComponent<FETransformComponent>()));

	// Check transformation
	if (ComponentType == "POSITION")
	{
		glm::vec3 ExpectedPosition = InitialChildTransform - InitialParentTransform;
		glm::vec3 ActualPosition = Entity_B->GetComponent<FETransformComponent>().GetPosition();
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));
	}
	else if (ComponentType == "ROTATION")
	{
		glm::quat ExpectedRotation = -(glm::inverse(glm::quat(InitialParentTransform)) * glm::quat(InitialChildTransform));
		glm::quat ActualRotation = Entity_B->GetComponent<FETransformComponent>().GetQuaternion();
		// Temporary code to check with UE5 values.
		glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));
	}
	else if (ComponentType == "SCALE")
	{
		glm::vec3 ExpectedScale = InitialChildTransform / InitialParentTransform;
		glm::vec3 ActualScale = Entity_B->GetComponent<FETransformComponent>().GetScale();
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));
	}
}

bool SceneGraphTest::ValidateTransformConsistency(FETransformComponent& Transform)
{
	glm::dvec3 WorldPosition = Transform.GetPosition(FE_WORLD_SPACE);
	glm::dquat WorldRotation = Transform.GetQuaternion(FE_WORLD_SPACE);
	glm::dvec3 WorldScale = Transform.GetScale(FE_WORLD_SPACE);

	glm::mat4 TransformMatrix = Transform.GetWorldMatrix();
	glm::dvec3 PositionFromMatrix;
	glm::dquat RotationFromMatrix;
	glm::dvec3 ScaleFromMatrix;
	GEOMETRY.DecomposeMatrixToTranslationRotationScale(TransformMatrix, PositionFromMatrix, RotationFromMatrix, ScaleFromMatrix);

	// Check if GetPosition() returns the same value as the position from the matrix
	if (!GEOMETRY.IsEpsilonEqual(WorldPosition, PositionFromMatrix))
		return false;

	// Check if GetQuaternion() returns the same value as the rotation from the matrix
	if (!GEOMETRY.IsEpsilonEqual(WorldRotation, RotationFromMatrix))
		return false;

	// Check if GetScale() returns the same value as the scale from the matrix
	if (!GEOMETRY.IsEpsilonEqual(WorldScale, ScaleFromMatrix))
		return false;

	glm::dvec3 LocalPosition = Transform.GetPosition();
	glm::dquat LocalRotation = Transform.GetQuaternion();
	glm::dvec3 LocalScale = Transform.GetScale();

	TransformMatrix = Transform.GetLocalMatrix();
	GEOMETRY.DecomposeMatrixToTranslationRotationScale(TransformMatrix, PositionFromMatrix, RotationFromMatrix, ScaleFromMatrix);

	// Check if GetPosition() returns the same value as the position from the matrix
	if (!GEOMETRY.IsEpsilonEqual(LocalPosition, PositionFromMatrix))
		return false;

	// Check if GetQuaternion() returns the same value as the rotation from the matrix
	if (!GEOMETRY.IsEpsilonEqual(LocalRotation, RotationFromMatrix))
		return false;

	// Check if GetScale() returns the same value as the scale from the matrix
	if (!GEOMETRY.IsEpsilonEqual(LocalScale, ScaleFromMatrix))
		return false;

	return true;
}

void SceneGraphTest::TestTransformationAfterChildAdded(FEScene* SceneToWorkWith, FETransformComponent& InitialParentTransform, FETransformComponent& InitialChildTransform)
{
	FEEntity* Entity_A = SceneToWorkWith->CreateEntity("A");
	FEEntity* Entity_B = SceneToWorkWith->CreateEntity("B");

	Entity_A->GetComponent<FETransformComponent>().SetPosition(InitialParentTransform.GetPosition());
	Entity_B->GetComponent<FETransformComponent>().SetPosition(InitialChildTransform.GetPosition());

	Entity_A->GetComponent<FETransformComponent>().SetQuaternion(InitialParentTransform.GetQuaternion());
	Entity_B->GetComponent<FETransformComponent>().SetQuaternion(InitialChildTransform.GetQuaternion());

	Entity_A->GetComponent<FETransformComponent>().SetScale(InitialParentTransform.GetScale());
	Entity_B->GetComponent<FETransformComponent>().SetScale(InitialChildTransform.GetScale());

	// Temporary using old style entities.
	std::string Node_A_ID = SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity_A->GetObjectID())->GetObjectID();
	std::string Node_B_ID = SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity_B->GetObjectID())->GetObjectID();

	SceneToWorkWith->SceneGraph.MoveNode(Node_B_ID, Node_A_ID);

	ASSERT_TRUE(ValidateTransformConsistency(Entity_B->GetComponent<FETransformComponent>()));

	// Get the local matrices
	glm::mat4 ParentLocalMatrix = InitialParentTransform.GetLocalMatrix();
	glm::mat4 ChildLocalMatrix = InitialChildTransform.GetLocalMatrix();

	// Calculate the inverse of the parent's local matrix
	glm::mat4 ParentLocalInverseMatrix = glm::inverse(ParentLocalMatrix);

	// Calculate the new final matrix for the child
	glm::mat4 ChildFinalMatrix = ParentLocalInverseMatrix * ChildLocalMatrix;

	glm::dvec3 ExpectedScale;
	glm::dquat ExpectedRotation;
	glm::dvec3 ExpectedPosition;
	if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(ChildFinalMatrix, ExpectedPosition, ExpectedRotation, ExpectedScale))
	{
		glm::dvec3 ActualPosition = Entity_B->GetComponent<FETransformComponent>().GetPosition();
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));

		glm::dquat ActualRotation = Entity_B->GetComponent<FETransformComponent>().GetQuaternion();
		// Temporary code to check with UE5 values.
		glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));

		glm::dvec3 ActualScale = Entity_B->GetComponent<FETransformComponent>().GetScale();
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));
	}
}

void SceneGraphTest::TestTransformationComponentAfterChildChangedParent(FEScene* SceneToWorkWith, const std::string& ComponentType, const glm::vec3& InitialParentTransform, const glm::vec3& InitialChildTransform)
{
	FEEntity* Entity_A = SceneToWorkWith->CreateEntity("A");
	FEEntity* Entity_B = SceneToWorkWith->CreateEntity("B");

	// Set transformations based on type
	if (ComponentType == "POSITION")
	{
		Entity_A->GetComponent<FETransformComponent>().SetPosition(InitialParentTransform);
		Entity_B->GetComponent<FETransformComponent>().SetPosition(InitialChildTransform);
	}
	else if (ComponentType == "ROTATION")
	{
		Entity_A->GetComponent<FETransformComponent>().SetQuaternion(glm::quat(InitialParentTransform));
		Entity_B->GetComponent<FETransformComponent>().SetQuaternion(glm::quat(InitialChildTransform));
	}
	else if (ComponentType == "SCALE")
	{
		Entity_A->GetComponent<FETransformComponent>().SetScale(InitialParentTransform);
		Entity_B->GetComponent<FETransformComponent>().SetScale(InitialChildTransform);
	}

	// Temporary using old style entities.
	std::string Node_A_ID = SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity_A->GetObjectID())->GetObjectID();
	std::string Node_B_ID = SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity_B->GetObjectID())->GetObjectID();

	SceneToWorkWith->SceneGraph.MoveNode(Node_B_ID, Node_A_ID);
	SceneToWorkWith->SceneGraph.MoveNode(Node_B_ID, SceneToWorkWith->SceneGraph.GetRoot()->GetObjectID());

	ASSERT_TRUE(ValidateTransformConsistency(Entity_B->GetComponent<FETransformComponent>()));

	// Check transformation
	if (ComponentType == "POSITION")
	{
		glm::vec3 ExpectedPosition = InitialChildTransform;
		glm::vec3 ActualPosition = Entity_B->GetComponent<FETransformComponent>().GetPosition();
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));
	}
	else if (ComponentType == "ROTATION")
	{
		glm::quat ExpectedRotation = glm::quat(InitialChildTransform);
		glm::quat ActualRotation = Entity_B->GetComponent<FETransformComponent>().GetQuaternion();
		// Temporary code to check with UE5 values.
		glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));
	}
	else if (ComponentType == "SCALE")
	{
		glm::vec3 ExpectedScale = InitialChildTransform;
		glm::vec3 ActualScale = Entity_B->GetComponent<FETransformComponent>().GetScale();
		ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));
	}
}

void SceneGraphTest::TestTransformationAfterChildChangedParent(FEScene* SceneToWorkWith, FETransformComponent& InitialParentTransform, FETransformComponent& InitialChildTransform)
{
	FEEntity* Entity_A = SceneToWorkWith->CreateEntity("A");
	FEEntity* Entity_B = SceneToWorkWith->CreateEntity("B");

	Entity_A->GetComponent<FETransformComponent>().SetPosition(InitialParentTransform.GetPosition());
	Entity_B->GetComponent<FETransformComponent>().SetPosition(InitialChildTransform.GetPosition());

	Entity_A->GetComponent<FETransformComponent>().SetQuaternion(InitialParentTransform.GetQuaternion());
	Entity_B->GetComponent<FETransformComponent>().SetQuaternion(InitialChildTransform.GetQuaternion());

	Entity_A->GetComponent<FETransformComponent>().SetScale(InitialParentTransform.GetScale());
	Entity_B->GetComponent<FETransformComponent>().SetScale(InitialChildTransform.GetScale());

	// Temporary using old style entities.
	std::string Node_A_ID = SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity_A->GetObjectID())->GetObjectID();
	std::string Node_B_ID = SceneToWorkWith->SceneGraph.GetNodeByEntityID(Entity_B->GetObjectID())->GetObjectID();

	SceneToWorkWith->SceneGraph.MoveNode(Node_B_ID, Node_A_ID);
	SceneToWorkWith->SceneGraph.MoveNode(Node_B_ID, SceneToWorkWith->SceneGraph.GetRoot()->GetObjectID());

	ASSERT_TRUE(ValidateTransformConsistency(Entity_B->GetComponent<FETransformComponent>()));

	glm::vec3 ExpectedPosition = InitialChildTransform.GetPosition();
	glm::vec3 ActualPosition = Entity_B->GetComponent<FETransformComponent>().GetPosition();
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));

	glm::quat ExpectedRotation = InitialChildTransform.GetQuaternion();
	glm::quat ActualRotation = Entity_B->GetComponent<FETransformComponent>().GetQuaternion();
	// Temporary code to check with UE5 values.
	glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));

	glm::vec3 ExpectedScale = InitialChildTransform.GetScale();
	glm::vec3 ActualScale = Entity_B->GetComponent<FETransformComponent>().GetScale();
	ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));
}

TEST_F(SceneGraphTest, Check_Basic_Transformations_After_Child_Added)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");

	TestTransformationComponentAfterChildAdded(CurrentScene, "POSITION", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, -1.0f, -1.0f));
	TestTransformationComponentAfterChildAdded(CurrentScene, "POSITION", glm::vec3(-34.6f, 20.4f, -23.5f), glm::vec3(4.5f, -2.7f, -13.3f));

	TestTransformationComponentAfterChildAdded(CurrentScene, "ROTATION", glm::vec3(glm::radians(5.0f), glm::radians(145.0f), glm::radians(45.0f)), glm::vec3(glm::radians(-65.0f), glm::radians(15.0f), glm::radians(90.0f)));
	TestTransformationComponentAfterChildAdded(CurrentScene, "ROTATION", glm::vec3(glm::radians(-46.23f), glm::radians(175.12f), glm::radians(90.0f)), glm::vec3(glm::radians(-162.6f), glm::radians(-27.23f), glm::radians(90.0f)));

	TestTransformationComponentAfterChildAdded(CurrentScene, "SCALE", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 2.0f, 1.5f));
	TestTransformationComponentAfterChildAdded(CurrentScene, "SCALE", glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(2.0f, 2.0f, 2.0f));

	// For more complex transformations uniform scaling is used.
	FETransformComponent ParentTransform;
	ParentTransform.SetPosition(glm::vec3(54.6f, -167.2f, -83.9f));
	ParentTransform.SetRotation(glm::vec3(-48.9f, 155.8f, -47.8f));
	ParentTransform.SetScale(glm::vec3(1.25f, 1.25f, 1.25f));

	FETransformComponent ChildTransform;
	ChildTransform.SetPosition(glm::vec3(-34.6f, 20.4f, -23.5f));
	ChildTransform.SetRotation(glm::vec3(-76.63f, -115.76f, 176.4f));
	ChildTransform.SetScale(glm::vec3(0.67f, 0.67f, 0.67f));

	TestTransformationAfterChildAdded(CurrentScene, ParentTransform, ChildTransform);

	TestTransformationComponentAfterChildChangedParent(CurrentScene, "POSITION", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, -1.0f, -1.0f));
	TestTransformationComponentAfterChildChangedParent(CurrentScene, "POSITION", glm::vec3(-34.6f, 20.4f, -23.5f), glm::vec3(4.5f, -2.7f, -13.3f));

	TestTransformationComponentAfterChildChangedParent(CurrentScene, "ROTATION", glm::vec3(glm::radians(5.0f), glm::radians(145.0f), glm::radians(45.0f)), glm::vec3(glm::radians(-65.0f), glm::radians(15.0f), glm::radians(90.0f)));
	TestTransformationComponentAfterChildChangedParent(CurrentScene, "ROTATION", glm::vec3(glm::radians(-46.23f), glm::radians(175.12f), glm::radians(90.0f)), glm::vec3(glm::radians(-162.6f), glm::radians(-27.23f), glm::radians(90.0f)));

	TestTransformationComponentAfterChildChangedParent(CurrentScene, "SCALE", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 2.0f, 1.5f));
	TestTransformationComponentAfterChildChangedParent(CurrentScene, "SCALE", glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(2.0f, 2.0f, 2.0f));

	// For more complex transformations uniform scaling is used.
	ParentTransform.SetPosition(glm::vec3(54.6f, -167.2f, -83.9f));
	ParentTransform.SetRotation(glm::vec3(-48.9f, 155.8f, -47.8f));
	ParentTransform.SetScale(glm::vec3(1.25f, 1.25f, 1.25f));

	ChildTransform.SetPosition(glm::vec3(-34.6f, 20.4f, -23.5f));
	ChildTransform.SetRotation(glm::vec3(-76.63f, -115.76f, 176.4f));
	ChildTransform.SetScale(glm::vec3(0.67f, 0.67f, 0.67f));

	TestTransformationAfterChildChangedParent(CurrentScene, ParentTransform, ChildTransform);

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Save_Load_Simple)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// Save IDs of the entities
	std::vector<std::string> EntityIDs;
	for (FENaiveSceneGraphNode* Node : Nodes)
	{
		EntityIDs.push_back(reinterpret_cast<FEEntity*>(Node->GetEntity())->GetObjectID());
	}

	// Save IDs of the nodes
	std::vector<std::string> NodeIDs;
	for (FENaiveSceneGraphNode* Node : Nodes)
	{
		NodeIDs.push_back(Node->GetObjectID());
	}

	// Save the scene
	std::string FilePath = "SceneGraphTest_Check_Save_Load_Simple.txt";
	
	Json::Value SceneHierarchy = CurrentScene->SceneGraph.ToJson();
	Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, SceneHierarchy);

	std::ofstream SceneFile;
	SceneFile.open("SceneGraphTest_Check_Save_Load_Simple.txt");
	SceneFile << JsonFile;
	SceneFile.close();

	CurrentScene->Clear();
	Nodes.clear();
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);

	// ****************************** Load the scene ******************************

	// Before we load the scene, we need to create the entities.
	for (size_t i = 0; i < 30; i++)
	{
		FEEntity* Entity = CurrentScene->CreateEntityOrphan("Node_" + std::to_string(i), EntityIDs[i]);
	}

	std::ifstream LoadSceneFile;
	LoadSceneFile.open("SceneGraphTest_Check_Save_Load_Simple.txt");
	std::string FileData((std::istreambuf_iterator<char>(LoadSceneFile)), std::istreambuf_iterator<char>());
	LoadSceneFile.close();

	Json::Value Root;
	JSONCPP_STRING Err;
	Json::CharReaderBuilder ReadBuilder;

	const std::unique_ptr<Json::CharReader> Reader(ReadBuilder.newCharReader());
	ASSERT_TRUE(Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &Root, &Err));
		
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	CurrentScene->SceneGraph.FromJson(Root);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// Retrieve the nodes with IDs of original nodes.
	std::vector<FENaiveSceneGraphNode*> LoadedNodes;
	for (const std::string& NodeID : NodeIDs)
	{
		LoadedNodes.push_back(CurrentScene->SceneGraph.GetNode(NodeID));
		ASSERT_NE(LoadedNodes.back(), nullptr);
	}

	// Check that scene nodes bound to correct entities.
	for (size_t i = 0; i < 30; i++)
	{
		ASSERT_EQ(LoadedNodes[i]->GetEntity()->GetObjectID(), EntityIDs[i]);
	}

	// Check if the hierarchy is correct.
	// Test direct parent-child relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[1]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[2]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[3]));

	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[4]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[5]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[6]));

	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[15], LoadedNodes[24]));

	// Test grandparent relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[5]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[8]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[12]));

	// Test great-grandparent relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[26]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[3], LoadedNodes[29]));

	// Test long-distance relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[23]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[24]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[25]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[26]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[27]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[28]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[29]));

	// Test non-descendant relationships
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[25]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[7]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[3]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[24]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[10]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[23]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[2]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[1]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[23], LoadedNodes[24]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[24], LoadedNodes[23]));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Save_Load_Simple_2)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// Save IDs of the entities
	std::vector<std::string> EntityIDs;
	for (FENaiveSceneGraphNode* Node : Nodes)
	{
		EntityIDs.push_back(reinterpret_cast<FEEntity*>(Node->GetEntity())->GetObjectID());
	}

	// Save IDs of the nodes
	std::vector<std::string> NodeIDs;
	for (FENaiveSceneGraphNode* Node : Nodes)
	{
		NodeIDs.push_back(Node->GetObjectID());
	}

	// Save the scene
	std::string FilePath = "SceneGraphTest_Check_Save_Load_Simple.txt";

	Json::Value SceneHierarchy = CurrentScene->SceneGraph.ToJson();
	Json::StreamWriterBuilder Builder;
	const std::string JsonFile = Json::writeString(Builder, SceneHierarchy);

	std::ofstream SceneFile;
	SceneFile.open("SceneGraphTest_Check_Save_Load_Simple.txt");
	SceneFile << JsonFile;
	SceneFile.close();

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
	Nodes.clear();
	CurrentScene = SCENE_MANAGER.CreateScene("TestScene_2");
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	// ****************************** Load the scene ******************************

	// Before we load the scene, we need to create the entities.
	for (size_t i = 0; i < 30; i++)
	{
		FEEntity* Entity = CurrentScene->CreateEntityOrphan("Node_" + std::to_string(i), EntityIDs[i]);
	}

	std::ifstream LoadSceneFile;
	LoadSceneFile.open("SceneGraphTest_Check_Save_Load_Simple.txt");
	std::string FileData((std::istreambuf_iterator<char>(LoadSceneFile)), std::istreambuf_iterator<char>());
	LoadSceneFile.close();

	Json::Value Root;
	JSONCPP_STRING Err;
	Json::CharReaderBuilder ReadBuilder;

	const std::unique_ptr<Json::CharReader> Reader(ReadBuilder.newCharReader());
	ASSERT_TRUE(Reader->parse(FileData.c_str(), FileData.c_str() + FileData.size(), &Root, &Err));

	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 0);
	CurrentScene->SceneGraph.FromJson(Root);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// Retrieve the nodes with IDs of original nodes.
	std::vector<FENaiveSceneGraphNode*> LoadedNodes;
	for (const std::string& NodeID : NodeIDs)
	{
		LoadedNodes.push_back(CurrentScene->SceneGraph.GetNode(NodeID));
		ASSERT_NE(LoadedNodes.back(), nullptr);
	}

	// Check that scene nodes bound to correct entities.
	for (size_t i = 0; i < 30; i++)
	{
		ASSERT_EQ(LoadedNodes[i]->GetEntity()->GetObjectID(), EntityIDs[i]);
	}

	// Check if the hierarchy is correct.
	// Test direct parent-child relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[1]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[2]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[3]));

	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[4]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[5]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[6]));

	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[15], LoadedNodes[24]));

	// Test grandparent relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[5]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[8]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[12]));

	// Test great-grandparent relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[26]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[3], LoadedNodes[29]));

	// Test long-distance relationship
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[23]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[24]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[25]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[26]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[27]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[28]));
	ASSERT_TRUE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[0], LoadedNodes[29]));

	// Test non-descendant relationships
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[25]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[7]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[3]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[24]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[10]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[23]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[1], LoadedNodes[2]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[2], LoadedNodes[1]));

	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[23], LoadedNodes[24]));
	ASSERT_FALSE(CurrentScene->SceneGraph.IsDescendant(LoadedNodes[24], LoadedNodes[23]));

	SCENE_MANAGER.DeleteScene(CurrentScene->GetObjectID());
}

void AddNodeAndChildsToVector(std::vector<FENaiveSceneGraphNode*>& VectorToAdd, FENaiveSceneGraphNode* Node)
{
	VectorToAdd.push_back(Node);
	std::vector<FENaiveSceneGraphNode*> Childrens = Node->GetChildren();
	for (size_t i = 0; i < Node->GetImediateChildrenCount(); i++)
	{
		AddNodeAndChildsToVector(VectorToAdd, Childrens[i]);
	}
}

bool CheckEntityParentScene(FEScene* Scene, FENaiveSceneGraphNode* Node)
{
	if (Node == nullptr)
		return false;

	if (Node != Scene->SceneGraph.GetRoot())
	{
		if (Node->GetEntity() == nullptr)
			return false;

		if (Node->GetEntity()->GetParentScene() != Scene)
			return false;
	}

	std::vector<FENaiveSceneGraphNode*> Childrens = Node->GetChildren();
	for (size_t i = 0; i < Node->GetImediateChildrenCount(); i++)
	{
		if (!CheckEntityParentScene(Scene, Childrens[i]))
			return false;
	}

	return true;
}

TEST_F(SceneGraphTest, Check_SceneNodes_Import)
{
	FEScene* FirstScene = SCENE_MANAGER.CreateScene("TestScene_1");
	ASSERT_EQ(FirstScene->SceneGraph.GetNodeCount(), 0);
	std::vector<FENaiveSceneGraphNode*> FirstNodes = PopulateSceneGraphMediumSize(FirstScene);
	for (size_t i = 0; i < FirstNodes.size(); i++)
		FirstNodes[i]->SetName("S1_" + FirstNodes[i]->GetName());
	size_t FirstNodeCount = 30;
	ASSERT_EQ(FirstScene->SceneGraph.GetNodeCount(), FirstNodeCount);

	FEScene* SecondScene = SCENE_MANAGER.CreateScene("TestScene_2");
	ASSERT_EQ(SecondScene->SceneGraph.GetNodeCount(), 0);
	std::vector<FENaiveSceneGraphNode*> SecondNodes = PopulateSceneGraphSmallSize(SecondScene);
	for (size_t i = 0; i < SecondNodes.size(); i++)
		SecondNodes[i]->SetName("S2_" + SecondNodes[i]->GetName());
	size_t SecondNodeCount = 15;
	ASSERT_EQ(SecondScene->SceneGraph.GetNodeCount(), SecondNodeCount);

	srand(RANDOM_SEED);
	for (size_t i = 0; i < RANDOM_ACTIONS_ITERATIONS / 20; i++)
	{
		bool bImportFromSecondToFirst = rand() % 2;
		FEScene* SourceScene = bImportFromSecondToFirst ? SecondScene : FirstScene;
		FEScene* DestinationScene = bImportFromSecondToFirst ? FirstScene : SecondScene;
		std::vector<FENaiveSceneGraphNode*>* SourceNodes = bImportFromSecondToFirst ? &SecondNodes : &FirstNodes;
		std::vector<FENaiveSceneGraphNode*>* DestinationNodes = bImportFromSecondToFirst ? &FirstNodes : &SecondNodes;

		size_t BeforeImportNodeCountInSource = SourceScene->SceneGraph.GetNodeCount();
		size_t BeforeImportNodeCountInDestination = DestinationScene->SceneGraph.GetNodeCount();

		size_t RandomIndexOfNodeToMove = rand() % SourceScene->SceneGraph.GetNodeCount();
		size_t RandomIndexOfNewParent = rand() % DestinationScene->SceneGraph.GetNodeCount();

		size_t NodeToMoveChildrenCount = (*SourceNodes)[RandomIndexOfNodeToMove]->GetImediateChildrenCount();
		size_t NodeToMoveRecursiveChildCount = (*SourceNodes)[RandomIndexOfNodeToMove]->GetRecursiveChildCount();

		size_t NewParentChildrenCount = (*DestinationNodes)[RandomIndexOfNewParent]->GetImediateChildrenCount();
		size_t NewParentRecursiveChildCount = (*DestinationNodes)[RandomIndexOfNewParent]->GetRecursiveChildCount();

		size_t NodesAdded = NodeToMoveRecursiveChildCount + 1;

		FEEntity* ImportedEntity = DestinationScene->ImportEntity((*SourceNodes)[RandomIndexOfNodeToMove]->GetEntity(), (*DestinationNodes)[RandomIndexOfNewParent]);
		FENaiveSceneGraphNode* ImportedNode = nullptr;
		if (ImportedEntity)
			ImportedNode = DestinationScene->SceneGraph.GetNodeByEntityID(ImportedEntity->GetObjectID());

		if (ImportedNode)
		{
			ImportedNode->SetName("Imported_" + ImportedNode->GetName());
			// If node was imported, check if the counts are correct.
			ASSERT_EQ((*SourceNodes)[RandomIndexOfNodeToMove]->GetImediateChildrenCount(), NodeToMoveChildrenCount);
			ASSERT_EQ((*SourceNodes)[RandomIndexOfNodeToMove]->GetRecursiveChildCount(), NodeToMoveRecursiveChildCount);
			ASSERT_EQ(SourceScene->SceneGraph.GetNodeCount(), BeforeImportNodeCountInSource);

			ASSERT_EQ(DestinationScene->SceneGraph.GetNodeCount(), BeforeImportNodeCountInDestination + (NodeToMoveRecursiveChildCount + 1));

			FirstNodeCount += bImportFromSecondToFirst ? NodesAdded : 0;
			SecondNodeCount += bImportFromSecondToFirst ? 0 : NodesAdded;

			// Check that all entities are bound to the correct scene.
			ASSERT_TRUE(CheckEntityParentScene(FirstScene, FirstScene->SceneGraph.GetRoot()));
			ASSERT_TRUE(CheckEntityParentScene(SecondScene, SecondScene->SceneGraph.GetRoot()));

			// Then adjust DestinationNodes
			AddNodeAndChildsToVector((*DestinationNodes), ImportedNode);

			ASSERT_EQ((*DestinationNodes)[RandomIndexOfNewParent]->GetImediateChildrenCount(), NewParentChildrenCount + 1);
			ASSERT_EQ((*DestinationNodes)[RandomIndexOfNewParent]->GetRecursiveChildCount(), NewParentRecursiveChildCount + NodeToMoveRecursiveChildCount + 1);
		}
		else
		{
			// If node was not imported, check if the counts are the same.
			ASSERT_EQ((*SourceNodes)[RandomIndexOfNodeToMove]->GetImediateChildrenCount(), NodeToMoveChildrenCount);
			ASSERT_EQ((*SourceNodes)[RandomIndexOfNodeToMove]->GetRecursiveChildCount(), NodeToMoveRecursiveChildCount);

			ASSERT_EQ((*DestinationNodes)[RandomIndexOfNewParent]->GetImediateChildrenCount(), NewParentChildrenCount);
			ASSERT_EQ((*DestinationNodes)[RandomIndexOfNewParent]->GetRecursiveChildCount(), NewParentRecursiveChildCount);
		}

		ASSERT_EQ(FirstScene->SceneGraph.GetNodeCount(), FirstNodeCount);
		ASSERT_EQ(SecondScene->SceneGraph.GetNodeCount(), SecondNodeCount);
	}

	ASSERT_EQ(FirstScene->SceneGraph.GetNodeCount(), FirstNodeCount);
	ASSERT_EQ(SecondScene->SceneGraph.GetNodeCount(), SecondNodeCount);
	SCENE_MANAGER.DeleteScene(FirstScene->GetObjectID());
	SCENE_MANAGER.DeleteScene(SecondScene->GetObjectID());
}

TEST_F(SceneGraphTest, Check_Delete_Nodes_and_Entities)
{
	FEScene* CurrentScene = SCENE_MANAGER.CreateScene("TestScene");
	std::vector<FENaiveSceneGraphNode*> Nodes = PopulateSceneGraphMediumSize(CurrentScene);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), 30);

	// Save IDs of the entities
	std::vector<std::string> EntityIDs;
	for (FENaiveSceneGraphNode* Node : Nodes)
	{
		EntityIDs.push_back(reinterpret_cast<FEEntity*>(Node->GetEntity())->GetObjectID());
	}

	// Save IDs of the nodes
	std::vector<std::string> NodeIDs;
	for (FENaiveSceneGraphNode* Node : Nodes)
	{
		NodeIDs.push_back(Node->GetObjectID());
	}

	int NodeCount = 30;
	// ************ Delete entity without children ************
	std::string EntityToDeleteID = Nodes[27]->GetEntity()->GetObjectID();
	std::string NodeToDeleteID = Nodes[27]->GetObjectID();

	ASSERT_EQ(Nodes[20]->GetImediateChildrenCount(), 1);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), NodeCount);
	ASSERT_EQ(Nodes[0]->GetRecursiveChildCount(), NodeCount - 1);

	CurrentScene->DeleteEntity(Nodes[27]->GetEntity());

	// Check if the entity and the node are deleted.
	ASSERT_EQ(CurrentScene->GetEntity(EntityToDeleteID), nullptr);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNode(NodeToDeleteID), nullptr);

	// Basic check if the hierarchy is correct.
	NodeCount--;
	ASSERT_EQ(Nodes[20]->GetImediateChildrenCount(), 0);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), NodeCount);
	ASSERT_EQ(Nodes[0]->GetRecursiveChildCount(), NodeCount - 1);

	// ************ Delete entity with one children ************
	EntityToDeleteID = Nodes[16]->GetEntity()->GetObjectID();
	NodeToDeleteID = Nodes[16]->GetObjectID();

	std::string ChildEntityToDeleteID = Nodes[25]->GetEntity()->GetObjectID();
	std::string ChildNodeToDeleteID = Nodes[25]->GetObjectID();

	CurrentScene->DeleteEntity(Nodes[16]->GetEntity());
	
	ASSERT_EQ(CurrentScene->GetEntity(EntityToDeleteID), nullptr);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNode(NodeToDeleteID), nullptr);

	ASSERT_EQ(CurrentScene->GetEntity(ChildEntityToDeleteID), nullptr);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNode(ChildNodeToDeleteID), nullptr);

	// Basic check if the hierarchy is correct.
	NodeCount -= 2;
	ASSERT_EQ(Nodes[7]->GetImediateChildrenCount(), 0);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), NodeCount);
	ASSERT_EQ(Nodes[0]->GetRecursiveChildCount(), NodeCount - 1);

	// ************ Delete entity that have subtree as children ************
	EntityToDeleteID = Nodes[1]->GetEntity()->GetObjectID();
	NodeToDeleteID = Nodes[1]->GetObjectID();

	std::vector<std::string> ChildEntitiesToDelete;
	std::vector<std::string> ChildNodesToDelete;

	std::vector<FENaiveSceneGraphNode*> AllChildrens = Nodes[1]->GetRecursiveChildren();
	for (FENaiveSceneGraphNode* Child : AllChildrens)
	{
		ChildEntitiesToDelete.push_back(Child->GetEntity()->GetObjectID());
		ChildNodesToDelete.push_back(Child->GetObjectID());
	}

	CurrentScene->DeleteEntity(Nodes[1]->GetEntity());

	ASSERT_EQ(CurrentScene->GetEntity(EntityToDeleteID), nullptr);
	ASSERT_EQ(CurrentScene->SceneGraph.GetNode(NodeToDeleteID), nullptr);

	for (size_t i = 0; i < ChildEntitiesToDelete.size(); i++)
	{
		ASSERT_EQ(CurrentScene->GetEntity(ChildEntitiesToDelete[i]), nullptr);
		ASSERT_EQ(CurrentScene->SceneGraph.GetNode(ChildNodesToDelete[i]), nullptr);
	}

	// Basic check if the hierarchy is correct.
	NodeCount -= 1 + AllChildrens.size();
	ASSERT_EQ(CurrentScene->SceneGraph.GetNodeCount(), NodeCount);
	ASSERT_EQ(Nodes[0]->GetRecursiveChildCount(), NodeCount - 1);
}