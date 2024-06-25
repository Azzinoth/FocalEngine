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

	// Root node can not be deleted.
	SCENE.SceneGraph.RemoveNode(RootNode);
	ASSERT_NE(SCENE.SceneGraph.GetNode(RootNode->GetObjectID()), nullptr);

	// Add a new node to the scene.
	FEEntity* Node_A = SCENE.AddEmptyEntity("Node_A");
	std::string Node_A_ID = SCENE.SceneGraph.AddNode(Node_A);

	// New node could be found by its ID.
	FENaiveSceneGraphNode* NodeByID = SCENE.SceneGraph.GetNode(Node_A_ID);
	ASSERT_NE(NodeByID, nullptr);

	// Delete the new node.
	SCENE.SceneGraph.RemoveNode(NodeByID);

	// Check that deleted node can not be found.
	ASSERT_EQ(SCENE.SceneGraph.GetNode(Node_A_ID), nullptr);

	SCENE.Clear();
}

TEST(SceneGraph, Check_For_Cycles)
{
	// Create a few nodes.
	FEEntity* Node_A = SCENE.AddEmptyEntity("Node_A");
	FEEntity* Node_B = SCENE.AddEmptyEntity("Node_B");
	FEEntity* Node_C = SCENE.AddEmptyEntity("Node_C");

	std::string Node_A_ID = SCENE.SceneGraph.AddNode(Node_A);
	std::string Node_B_ID = SCENE.SceneGraph.AddNode(Node_B);
	std::string Node_C_ID = SCENE.SceneGraph.AddNode(Node_C);

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

TEST(SceneGraph, Check_IsDescendant_Function)
{
	// Create some entities
	FEEntity* Entity_A = SCENE.AddEmptyEntity("A");
	FEEntity* Entity_B = SCENE.AddEmptyEntity("B");
	FEEntity* Entity_C = SCENE.AddEmptyEntity("C");
	FEEntity* Entity_D = SCENE.AddEmptyEntity("D");
	FEEntity* Entity_E = SCENE.AddEmptyEntity("E");

	// Add nodes to the scene graph
	std::string Node_A_ID = SCENE.SceneGraph.AddNode(Entity_A);
	std::string Node_B_ID = SCENE.SceneGraph.AddNode(Entity_B);
	std::string Node_C_ID = SCENE.SceneGraph.AddNode(Entity_C);
	std::string Node_D_ID = SCENE.SceneGraph.AddNode(Entity_D);
	std::string Node_E_ID = SCENE.SceneGraph.AddNode(Entity_E);

	// Create a hierarchy: A -> B -> C -> D
	//                      \-> E
	SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);
	SCENE.SceneGraph.MoveNode(Node_C_ID, Node_B_ID);
	SCENE.SceneGraph.MoveNode(Node_D_ID, Node_C_ID);
	SCENE.SceneGraph.MoveNode(Node_E_ID, Node_A_ID);

	FENaiveSceneGraphNode* Node_A = SCENE.SceneGraph.GetNode(Node_A_ID);
	FENaiveSceneGraphNode* Node_B = SCENE.SceneGraph.GetNode(Node_B_ID);
	FENaiveSceneGraphNode* Node_C = SCENE.SceneGraph.GetNode(Node_C_ID);
	FENaiveSceneGraphNode* Node_D = SCENE.SceneGraph.GetNode(Node_D_ID);
	FENaiveSceneGraphNode* Node_E = SCENE.SceneGraph.GetNode(Node_E_ID);

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

bool IsEqual(const glm::dvec3& FirstVector, const glm::dvec3& SecondVector, double Epsilon = 1e-5)
{
	return std::abs(FirstVector.x - SecondVector.x) < Epsilon &&
		   std::abs(FirstVector.y - SecondVector.y) < Epsilon &&
		   std::abs(FirstVector.z - SecondVector.z) < Epsilon;
}

bool IsEqual(const glm::dquat& FirstQuaternion, const glm::dquat& SecondQuaternion, double Epsilon = 1e-5)
{
	return std::abs(FirstQuaternion.x - SecondQuaternion.x) < Epsilon &&
		   std::abs(FirstQuaternion.y - SecondQuaternion.y) < Epsilon &&
		   std::abs(FirstQuaternion.z - SecondQuaternion.z) < Epsilon &&
		   std::abs(FirstQuaternion.w - SecondQuaternion.w) < Epsilon;
}

bool ValidateTransformConsistency(const FETransformComponent& Transform)
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
	if (!IsEqual(Position, PositionFromMatrix))
		return false;

	// Check if GetQuaternion() returns the same value as the rotation from the matrix
	if (!IsEqual(Rotation, RotationFromMatrix))
		return false;

	// Check if GetScale() returns the same value as the scale from the matrix
	if (!IsEqual(Scale, ScaleFromMatrix))
		return false;

	return true;
}

TEST(SceneGraph, Check_Basic_Transformations_After_Child_Added)
{
	// Check positions.
	glm::vec3 InitialParentPosition = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 InitialChildPosition = glm::vec3(-1.0f, -1.0f, -1.0f);

	FEEntity* Entity_A = SCENE.AddEmptyEntity("A");
	Entity_A->Transform.SetPosition(InitialParentPosition);

	FEEntity* Entity_B = SCENE.AddEmptyEntity("B");
	Entity_B->Transform.SetPosition(InitialChildPosition);

	// Add nodes to the scene graph
	std::string Node_A_ID = SCENE.SceneGraph.AddNode(Entity_A);
	std::string Node_B_ID = SCENE.SceneGraph.AddNode(Entity_B);

	// Create a hierarchy: A -> B
	SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);

	ASSERT_TRUE(ValidateTransformConsistency(Entity_B->Transform));

	// Check that child's position was set in a way that it is relative to parent.
	// and to preserve the world position of the child.
	glm::vec3 ExpectedPosition = InitialChildPosition - InitialParentPosition;
	glm::vec3 ActualPosition = Entity_B->Transform.GetPosition();
	ASSERT_EQ(ActualPosition, ExpectedPosition);

	SCENE.Clear();

	// Test with different initial positions
	InitialParentPosition = glm::vec3(-34.6f, 20.4f, -23.5f);
	InitialChildPosition = glm::vec3(4.5f, -2.7f, -13.3f);

	Entity_A = SCENE.AddEmptyEntity("A");
	Entity_A->Transform.SetPosition(InitialParentPosition);

	Entity_B = SCENE.AddEmptyEntity("B");
	Entity_B->Transform.SetPosition(InitialChildPosition);

	// Add nodes to the scene graph
	Node_A_ID = SCENE.SceneGraph.AddNode(Entity_A);
	Node_B_ID = SCENE.SceneGraph.AddNode(Entity_B);

	// Create a hierarchy: A -> B
	SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);

	ASSERT_TRUE(ValidateTransformConsistency(Entity_B->Transform));

	// Check that child's position was set in a way that it is relative to parent.
	// and to preserve the world position of the child.
	ExpectedPosition = InitialChildPosition - InitialParentPosition;
	ActualPosition = Entity_B->Transform.GetPosition();
	ASSERT_EQ(ActualPosition, ExpectedPosition);

	SCENE.Clear();

	// Check rotations.
	glm::quat InitialParentRotation = glm::quat(glm::vec3(glm::radians(5.0f), glm::radians(145.0f), glm::radians(45.0f)));
	glm::quat InitialChildRotation = glm::quat(glm::vec3(glm::radians(-65.0f), glm::radians(15.0f), glm::radians(90.0f)));

	Entity_A = SCENE.AddEmptyEntity("A");
	Entity_A->Transform.SetQuaternion(InitialParentRotation);

	Entity_B = SCENE.AddEmptyEntity("B");
	Entity_B->Transform.SetQuaternion(InitialChildRotation);

	// Add nodes to the scene graph
	Node_A_ID = SCENE.SceneGraph.AddNode(Entity_A);
	Node_B_ID = SCENE.SceneGraph.AddNode(Entity_B);

	// Create a hierarchy: A -> B
	SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);

	ASSERT_TRUE(ValidateTransformConsistency(Entity_B->Transform));

	// Check that child's rotation was set in a way that it is relative to parent.
	// and to preserve the world rotation of the child.
	glm::quat ExpectedRotation = -(glm::inverse(InitialParentRotation) * InitialChildRotation);
	glm::quat ActualRotation = Entity_B->Transform.GetQuaternion();
	// Temporary code to check with UE5 values.
	glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
	ASSERT_TRUE(IsEqual(ActualRotation, ExpectedRotation));

	SCENE.Clear();

	// Test with different initial rotations
	InitialParentRotation = glm::quat(glm::vec3(glm::radians(-46.23f), glm::radians(175.12f), glm::radians(90.0f)));
	InitialChildRotation = glm::quat(glm::vec3(glm::radians(-162.6f), glm::radians(-27.23f), glm::radians(90.0f)));

	Entity_A = SCENE.AddEmptyEntity("A");
	Entity_A->Transform.SetQuaternion(InitialParentRotation);

	Entity_B = SCENE.AddEmptyEntity("B");
	Entity_B->Transform.SetQuaternion(InitialChildRotation);

	// Add nodes to the scene graph
	Node_A_ID = SCENE.SceneGraph.AddNode(Entity_A);
	Node_B_ID = SCENE.SceneGraph.AddNode(Entity_B);

	// Create a hierarchy: A -> B
	SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);

	ASSERT_TRUE(ValidateTransformConsistency(Entity_B->Transform));

	// Check that child's rotation was set in a way that it is relative to parent.
	// and to preserve the world rotation of the child.
	ExpectedRotation = -(glm::inverse(InitialParentRotation) * InitialChildRotation);
	ActualRotation = Entity_B->Transform.GetQuaternion();
	// Temporary code to check with UE5 values.
	glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
	ASSERT_TRUE(IsEqual(ActualRotation, ExpectedRotation));

	SCENE.Clear();
}