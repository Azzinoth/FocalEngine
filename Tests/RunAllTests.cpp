#include "RunAllTests.h"
using namespace FocalEngine;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize Google Test
	testing::InitGoogleTest();

	ENGINE.InitWindow(1280, 720);
	THREAD_POOL.SetConcurrentThreadCount(10);

	ENGINE.GetCamera()->SetIsInputActive(false);

	// Run the tests
	int TestResult = RUN_ALL_TESTS();

	while (ENGINE.IsNotTerminated())
	{
		ENGINE.BeginFrame();
		ENGINE.Render();
		ENGINE.EndFrame();
	}

	return 0;
}

TEST(SceneGraph, Check_Basic_Add_Find_Delete_Nodes)
{
	// Get root node from the scene returns valid node.
	FENaiveSceneGraphNode* RootNode = SCENE.SceneGraph.GetRoot();
	ASSERT_NE(RootNode, nullptr);

	// Root node can be found by its ID.
	FENaiveSceneGraphNode* RootNodeByID = SCENE.SceneGraph.GetNode(RootNode->GetObjectID());
	ASSERT_EQ(RootNode, RootNodeByID);
	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 0);

	// Root node can not be deleted.
	SCENE.SceneGraph.DeleteNode(RootNode);
	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 0);
	ASSERT_NE(SCENE.SceneGraph.GetNode(RootNode->GetObjectID()), nullptr);

	// Add a new node to the scene.
	FEEntity* Node_A = SCENE.AddEmptyEntity("Node_A");
	// Temporary using old style entities.
	std::string Node_A_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Node_A->GetObjectID())->GetObjectID();
	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 1);

	// New node could be found by its ID.
	FENaiveSceneGraphNode* NodeByID = SCENE.SceneGraph.GetNode(Node_A_ID);
	ASSERT_NE(NodeByID, nullptr);

	// Delete the new node.
	SCENE.SceneGraph.DeleteNode(NodeByID);
	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 0);

	// Check that deleted node can not be found.
	ASSERT_EQ(SCENE.SceneGraph.GetNode(Node_A_ID), nullptr);

	SCENE.Clear();
}

TEST(SceneGraph, Check_For_Cycles)
{
	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 0);

	// Create a few nodes.
	FEEntity* Node_A = SCENE.AddEmptyEntity("Node_A");
	FEEntity* Node_B = SCENE.AddEmptyEntity("Node_B");
	FEEntity* Node_C = SCENE.AddEmptyEntity("Node_C");

	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 3);

	// Temporary using old style entities.
	std::string Node_A_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Node_A->GetObjectID())->GetObjectID();
	std::string Node_B_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Node_B->GetObjectID())->GetObjectID();
	std::string Node_C_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Node_C->GetObjectID())->GetObjectID();

	FENaiveSceneGraphNode* NodeA = SCENE.SceneGraph.GetNode(Node_A_ID);
	FENaiveSceneGraphNode* NodeB = SCENE.SceneGraph.GetNode(Node_B_ID);
	FENaiveSceneGraphNode* NodeC = SCENE.SceneGraph.GetNode(Node_C_ID);

	// Create a valid hierarchy.
	SCENE.SceneGraph.MoveNode(NodeB->GetObjectID(), NodeA->GetObjectID());
	SCENE.SceneGraph.MoveNode(NodeC->GetObjectID(), NodeB->GetObjectID());

	// Check that there are no cycles in this valid hierarchy.
	ASSERT_FALSE(SCENE.SceneGraph.HasCycle(NodeA));
	ASSERT_FALSE(SCENE.SceneGraph.HasCycle(NodeB));
	ASSERT_FALSE(SCENE.SceneGraph.HasCycle(NodeC));

	// Try to create a cycle.
	ASSERT_FALSE(SCENE.SceneGraph.MoveNode(NodeA->GetObjectID(), NodeC->GetObjectID()));

	// Verify that the cycle was not created.
	ASSERT_FALSE(SCENE.SceneGraph.HasCycle(NodeA));
	ASSERT_FALSE(SCENE.SceneGraph.HasCycle(NodeB));
	ASSERT_FALSE(SCENE.SceneGraph.HasCycle(NodeC));

	// Try to create a self-cycle.
	ASSERT_FALSE(SCENE.SceneGraph.MoveNode(NodeA->GetObjectID(), NodeA->GetObjectID()));

	// Verify that the self-cycle was not created.
	ASSERT_FALSE(SCENE.SceneGraph.HasCycle(NodeA));

	SCENE.Clear();
}

TEST(SceneGraph, Check_GetNodeCount_AND_ChildCount_Functions)
{
	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 0);

	// Create some entities
	FEEntity* Entity_A = SCENE.AddEmptyEntity("A");
	FEEntity* Entity_B = SCENE.AddEmptyEntity("B");
	FEEntity* Entity_C = SCENE.AddEmptyEntity("C");
	FEEntity* Entity_D = SCENE.AddEmptyEntity("D");
	FEEntity* Entity_E = SCENE.AddEmptyEntity("E");
	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 5);

	// Temporary using old style entities.
	std::string Node_A_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_A->GetObjectID())->GetObjectID();
	std::string Node_B_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_B->GetObjectID())->GetObjectID();
	std::string Node_C_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_C->GetObjectID())->GetObjectID();
	std::string Node_D_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_D->GetObjectID())->GetObjectID();
	std::string Node_E_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_E->GetObjectID())->GetObjectID();

	FENaiveSceneGraphNode* Node_A = SCENE.SceneGraph.GetNode(Node_A_ID);
	FENaiveSceneGraphNode* Node_B = SCENE.SceneGraph.GetNode(Node_B_ID);
	FENaiveSceneGraphNode* Node_C = SCENE.SceneGraph.GetNode(Node_C_ID);
	FENaiveSceneGraphNode* Node_D = SCENE.SceneGraph.GetNode(Node_D_ID);
	FENaiveSceneGraphNode* Node_E = SCENE.SceneGraph.GetNode(Node_E_ID);

	// Create a hierarchy: A -> B -> C -> D
	//                      \-> E
	SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);
	ASSERT_EQ(Node_A->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_A->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Node_B->GetImediateChildrenCount(), 0);
	ASSERT_EQ(Node_B->GetRecursiveChildCount(), 0);

	SCENE.SceneGraph.MoveNode(Node_C_ID, Node_B_ID);
	ASSERT_EQ(Node_A->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_A->GetRecursiveChildCount(), 2);
	ASSERT_EQ(Node_B->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_B->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Node_C->GetImediateChildrenCount(), 0);

	SCENE.SceneGraph.MoveNode(Node_D_ID, Node_C_ID);
	ASSERT_EQ(Node_A->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_A->GetRecursiveChildCount(), 3);
	ASSERT_EQ(Node_B->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_B->GetRecursiveChildCount(), 2);
	ASSERT_EQ(Node_C->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_C->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Node_D->GetImediateChildrenCount(), 0);

	SCENE.SceneGraph.MoveNode(Node_E_ID, Node_D_ID);
	ASSERT_EQ(Node_A->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_A->GetRecursiveChildCount(), 4);
	ASSERT_EQ(Node_B->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_B->GetRecursiveChildCount(), 3);
	ASSERT_EQ(Node_C->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_C->GetRecursiveChildCount(), 2);
	ASSERT_EQ(Node_D->GetImediateChildrenCount(), 1);
	ASSERT_EQ(Node_D->GetRecursiveChildCount(), 1);
	ASSERT_EQ(Node_E->GetImediateChildrenCount(), 0);
	ASSERT_EQ(Node_E->GetRecursiveChildCount(), 0);

	ASSERT_EQ(SCENE.SceneGraph.GetNodeCount(), 5);

	SCENE.Clear();
}

TEST(SceneGraph, Check_IsDescendant_Function)
{
	// Create some entities
	FEEntity* Entity_A = SCENE.AddEmptyEntity("A");
	FEEntity* Entity_B = SCENE.AddEmptyEntity("B");
	FEEntity* Entity_C = SCENE.AddEmptyEntity("C");
	FEEntity* Entity_D = SCENE.AddEmptyEntity("D");
	FEEntity* Entity_E = SCENE.AddEmptyEntity("E");

	// Temporary using old style entities.
	std::string Node_A_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_A->GetObjectID())->GetObjectID();
	std::string Node_B_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_B->GetObjectID())->GetObjectID();
	std::string Node_C_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_C->GetObjectID())->GetObjectID();
	std::string Node_D_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_D->GetObjectID())->GetObjectID();
	std::string Node_E_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_E->GetObjectID())->GetObjectID();

	FENaiveSceneGraphNode* Node_A = SCENE.SceneGraph.GetNode(Node_A_ID);
	FENaiveSceneGraphNode* Node_B = SCENE.SceneGraph.GetNode(Node_B_ID);
	FENaiveSceneGraphNode* Node_C = SCENE.SceneGraph.GetNode(Node_C_ID);
	FENaiveSceneGraphNode* Node_D = SCENE.SceneGraph.GetNode(Node_D_ID);
	FENaiveSceneGraphNode* Node_E = SCENE.SceneGraph.GetNode(Node_E_ID);

	// Create a hierarchy: A -> B -> C -> D
	//                      \-> E
	SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);
	SCENE.SceneGraph.MoveNode(Node_C_ID, Node_B_ID);
	SCENE.SceneGraph.MoveNode(Node_D_ID, Node_C_ID);
	SCENE.SceneGraph.MoveNode(Node_E_ID, Node_A_ID);

	// Test direct parent-child relationship
	ASSERT_TRUE(SCENE.SceneGraph.IsDescendant(Node_A, Node_B));
	ASSERT_TRUE(SCENE.SceneGraph.IsDescendant(Node_B, Node_C));
	ASSERT_TRUE(SCENE.SceneGraph.IsDescendant(Node_C, Node_D));
	ASSERT_TRUE(SCENE.SceneGraph.IsDescendant(Node_A, Node_E));

	// Test grandparent relationship
	ASSERT_TRUE(SCENE.SceneGraph.IsDescendant(Node_A, Node_C));
	ASSERT_TRUE(SCENE.SceneGraph.IsDescendant(Node_B, Node_D));

	// Test great-grandparent relationship
	ASSERT_TRUE(SCENE.SceneGraph.IsDescendant(Node_A, Node_D));

	// Test non-descendant relationships
	ASSERT_FALSE(SCENE.SceneGraph.IsDescendant(Node_B, Node_A));
	ASSERT_FALSE(SCENE.SceneGraph.IsDescendant(Node_C, Node_A));
	ASSERT_FALSE(SCENE.SceneGraph.IsDescendant(Node_D, Node_A));
	ASSERT_FALSE(SCENE.SceneGraph.IsDescendant(Node_E, Node_B));
	ASSERT_FALSE(SCENE.SceneGraph.IsDescendant(Node_D, Node_E));

	// Test self-relationship
	ASSERT_TRUE(SCENE.SceneGraph.IsDescendant(Node_A, Node_A));

	// Test with null nodes
	ASSERT_FALSE(SCENE.SceneGraph.IsDescendant(nullptr, Node_A));
	ASSERT_FALSE(SCENE.SceneGraph.IsDescendant(Node_A, nullptr));
	ASSERT_FALSE(SCENE.SceneGraph.IsDescendant(nullptr, nullptr));

	SCENE.Clear();
}

bool SceneGraphTest::ValidateTransformConsistency(const FETransformComponent& Transform)
{
	glm::dvec3 Position = Transform.GetPosition();
	glm::dquat Rotation = Transform.GetQuaternion();
	glm::dvec3 Scale = Transform.GetScale();

	glm::mat4 TransformMatrix = Transform.GetTransformMatrix();
	glm::dvec3 PositionFromMatrix;
	glm::dquat RotationFromMatrix;
	glm::dvec3 ScaleFromMatrix;
	GEOMETRY.DecomposeMatrixToTranslationRotationScale(TransformMatrix, PositionFromMatrix, RotationFromMatrix, ScaleFromMatrix);

	// Check if GetPosition() returns the same value as the position from the matrix
	if (!GEOMETRY.IsEpsilonEqual(Position, PositionFromMatrix))
		return false;

	// Check if GetQuaternion() returns the same value as the rotation from the matrix
	if (!GEOMETRY.IsEpsilonEqual(Rotation, RotationFromMatrix))
		return false;

	// Check if GetScale() returns the same value as the scale from the matrix
	if (!GEOMETRY.IsEpsilonEqual(Scale, ScaleFromMatrix))
		return false;

	return true;
}

TEST_F(SceneGraphTest, Check_Basic_Transformations_After_Child_Added)
{
	TestTransformationComponentAfterChildAdded("POSITION", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, -1.0f, -1.0f));
	TestTransformationComponentAfterChildAdded("POSITION", glm::vec3(-34.6f, 20.4f, -23.5f), glm::vec3(4.5f, -2.7f, -13.3f));

	TestTransformationComponentAfterChildAdded("ROTATION", glm::vec3(glm::radians(5.0f), glm::radians(145.0f), glm::radians(45.0f)), glm::vec3(glm::radians(-65.0f), glm::radians(15.0f), glm::radians(90.0f)));
	TestTransformationComponentAfterChildAdded("ROTATION", glm::vec3(glm::radians(-46.23f), glm::radians(175.12f), glm::radians(90.0f)), glm::vec3(glm::radians(-162.6f), glm::radians(-27.23f), glm::radians(90.0f)));

	TestTransformationComponentAfterChildAdded("SCALE", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 2.0f, 1.5f));
	TestTransformationComponentAfterChildAdded("SCALE", glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(2.0f, 2.0f, 2.0f));

	// For more complex transformations uniform scaling is used.
	FETransformComponent ParentTransform;
	ParentTransform.SetPosition(glm::vec3(54.6f, -167.2f, -83.9f));
	ParentTransform.SetRotation(glm::vec3(-48.9f, 155.8f, -47.8f));
	ParentTransform.SetScale(glm::vec3(1.25f, 1.25f, 1.25f));

	FETransformComponent ChildTransform;
	ChildTransform.SetPosition(glm::vec3(-34.6f, 20.4f, -23.5f));
	ChildTransform.SetRotation(glm::vec3(-76.63f, -115.76f, 176.4f));
	ChildTransform.SetScale(glm::vec3(0.67f, 0.67f, 0.67f));

	TestTransformationAfterChildAdded(ParentTransform, ChildTransform);

	TestTransformationComponentAfterChildChangedParent("POSITION", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-1.0f, -1.0f, -1.0f));
	TestTransformationComponentAfterChildChangedParent("POSITION", glm::vec3(-34.6f, 20.4f, -23.5f), glm::vec3(4.5f, -2.7f, -13.3f));

	TestTransformationComponentAfterChildChangedParent("ROTATION", glm::vec3(glm::radians(5.0f), glm::radians(145.0f), glm::radians(45.0f)), glm::vec3(glm::radians(-65.0f), glm::radians(15.0f), glm::radians(90.0f)));
	TestTransformationComponentAfterChildChangedParent("ROTATION", glm::vec3(glm::radians(-46.23f), glm::radians(175.12f), glm::radians(90.0f)), glm::vec3(glm::radians(-162.6f), glm::radians(-27.23f), glm::radians(90.0f)));

	TestTransformationComponentAfterChildChangedParent("SCALE", glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 2.0f, 1.5f));
	TestTransformationComponentAfterChildChangedParent("SCALE", glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(2.0f, 2.0f, 2.0f));

	// For more complex transformations uniform scaling is used.
	ParentTransform.SetPosition(glm::vec3(54.6f, -167.2f, -83.9f));
	ParentTransform.SetRotation(glm::vec3(-48.9f, 155.8f, -47.8f));
	ParentTransform.SetScale(glm::vec3(1.25f, 1.25f, 1.25f));

	ChildTransform.SetPosition(glm::vec3(-34.6f, 20.4f, -23.5f));
	ChildTransform.SetRotation(glm::vec3(-76.63f, -115.76f, 176.4f));
	ChildTransform.SetScale(glm::vec3(0.67f, 0.67f, 0.67f));

	TestTransformationAfterChildChangedParent(ParentTransform, ChildTransform);
}