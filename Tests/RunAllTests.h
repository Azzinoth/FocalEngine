#pragma once

#include "../FEngine.h"
#include "gtest/gtest.h"
using namespace FocalEngine;


class SceneGraphTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        
    }

    void TearDown() override
    {
        SCENE.Clear();
    }

    bool ValidateTransformConsistency(const FETransformComponent& Transform);

    void TestTransformationComponentAfterChildAdded(const std::string& ComponentType, const glm::vec3& InitialParentTransform, const glm::vec3& InitialChildTransform)
    {
        FEEntity* Entity_A = SCENE.AddEmptyEntity("A");
        FEEntity* Entity_B = SCENE.AddEmptyEntity("B");

        // Set transformations based on type
        if (ComponentType == "POSITION")
        {
            Entity_A->Transform.SetPosition(InitialParentTransform);
            Entity_B->Transform.SetPosition(InitialChildTransform);
        }
        else if (ComponentType == "ROTATION")
        {
            Entity_A->Transform.SetQuaternion(glm::quat(InitialParentTransform));
            Entity_B->Transform.SetQuaternion(glm::quat(InitialChildTransform));
        }
        else if (ComponentType == "SCALE")
        {
            Entity_A->Transform.SetScale(InitialParentTransform);
            Entity_B->Transform.SetScale(InitialChildTransform);
        }

        // Temporary using old style entities.
        std::string Node_A_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_A->GetObjectID())->GetObjectID();
        std::string Node_B_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_B->GetObjectID())->GetObjectID();

        SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);

        ASSERT_TRUE(ValidateTransformConsistency(Entity_B->Transform));

        // Check transformation
        if (ComponentType == "POSITION")
        {
            glm::vec3 ExpectedPosition = InitialChildTransform - InitialParentTransform;
            glm::vec3 ActualPosition = Entity_B->Transform.GetPosition();
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));
        }
        else if (ComponentType == "ROTATION")
        {
            glm::quat ExpectedRotation = -(glm::inverse(glm::quat(InitialParentTransform)) * glm::quat(InitialChildTransform));
            glm::quat ActualRotation = Entity_B->Transform.GetQuaternion();
            // Temporary code to check with UE5 values.
            glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));
        }
        else if (ComponentType == "SCALE")
        {
            glm::vec3 ExpectedScale = InitialChildTransform / InitialParentTransform;
            glm::vec3 ActualScale = Entity_B->Transform.GetScale();
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));
        }
    }

    void TestTransformationAfterChildAdded(const FETransformComponent& InitialParentTransform, const FETransformComponent& InitialChildTransform)
    {
        FEEntity* Entity_A = SCENE.AddEmptyEntity("A");
        FEEntity* Entity_B = SCENE.AddEmptyEntity("B");

        Entity_A->Transform.SetPosition(InitialParentTransform.GetPosition());
        Entity_B->Transform.SetPosition(InitialChildTransform.GetPosition());
        
        Entity_A->Transform.SetQuaternion(InitialParentTransform.GetQuaternion());
        Entity_B->Transform.SetQuaternion(InitialChildTransform.GetQuaternion());
       
        Entity_A->Transform.SetScale(InitialParentTransform.GetScale());
        Entity_B->Transform.SetScale(InitialChildTransform.GetScale());

        // Temporary using old style entities.
        std::string Node_A_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_A->GetObjectID())->GetObjectID();
        std::string Node_B_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_B->GetObjectID())->GetObjectID();

        SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);

        ASSERT_TRUE(ValidateTransformConsistency(Entity_B->Transform));

        // Get the world matrices
        glm::mat4 ParentWorldMatrix = InitialParentTransform.GetTransformMatrix();
        glm::mat4 ChildWorldMatrix = InitialChildTransform.GetTransformMatrix();

        // Calculate the inverse of the parent's world matrix
        glm::mat4 ParentWorldInverseMatrix = glm::inverse(ParentWorldMatrix);

        // Calculate the new local matrix for the child
        glm::mat4 ChildLocalMatrix = ParentWorldInverseMatrix * ChildWorldMatrix;

        glm::dvec3 ExpectedScale;
        glm::dquat ExpectedRotation;
        glm::dvec3 ExpectedPosition;
        if (GEOMETRY.DecomposeMatrixToTranslationRotationScale(ChildLocalMatrix, ExpectedPosition, ExpectedRotation, ExpectedScale))
        {
            glm::dvec3 ActualPosition = Entity_B->Transform.GetPosition();
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));

            glm::dquat ActualRotation = Entity_B->Transform.GetQuaternion();
            // Temporary code to check with UE5 values.
            glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));

            glm::dvec3 ActualScale = Entity_B->Transform.GetScale();
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));
        }
    }

    void TestTransformationComponentAfterChildChangedParent(const std::string& ComponentType, const glm::vec3& InitialParentTransform, const glm::vec3& InitialChildTransform)
    {
        FEEntity* Entity_A = SCENE.AddEmptyEntity("A");
		FEEntity* Entity_B = SCENE.AddEmptyEntity("B");

		// Set transformations based on type
		if (ComponentType == "POSITION")
		{
			Entity_A->Transform.SetPosition(InitialParentTransform);
			Entity_B->Transform.SetPosition(InitialChildTransform);
		}
		else if (ComponentType == "ROTATION")
		{
			Entity_A->Transform.SetQuaternion(glm::quat(InitialParentTransform));
			Entity_B->Transform.SetQuaternion(glm::quat(InitialChildTransform));
		}
		else if (ComponentType == "SCALE")
		{
			Entity_A->Transform.SetScale(InitialParentTransform);
			Entity_B->Transform.SetScale(InitialChildTransform);
		}

        // Temporary using old style entities.
        std::string Node_A_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_A->GetObjectID())->GetObjectID();
        std::string Node_B_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_B->GetObjectID())->GetObjectID();

		SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);
        SCENE.SceneGraph.MoveNode(Node_B_ID, SCENE.SceneGraph.GetRoot()->GetObjectID());

		ASSERT_TRUE(ValidateTransformConsistency(Entity_B->Transform));

		// Check transformation
		if (ComponentType == "POSITION")
		{
			glm::vec3 ExpectedPosition = InitialChildTransform;
			glm::vec3 ActualPosition = Entity_B->Transform.GetPosition();
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));
		}
		else if (ComponentType == "ROTATION")
		{
			glm::quat ExpectedRotation = glm::quat(InitialChildTransform);
			glm::quat ActualRotation = Entity_B->Transform.GetQuaternion();
			// Temporary code to check with UE5 values.
			glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));
		}
		else if (ComponentType == "SCALE")
		{
			glm::vec3 ExpectedScale = InitialChildTransform;
			glm::vec3 ActualScale = Entity_B->Transform.GetScale();
            ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));
		}
    }

    void TestTransformationAfterChildChangedParent(const FETransformComponent& InitialParentTransform, const FETransformComponent& InitialChildTransform)
    {
        FEEntity* Entity_A = SCENE.AddEmptyEntity("A");
        FEEntity* Entity_B = SCENE.AddEmptyEntity("B");

        Entity_A->Transform.SetPosition(InitialParentTransform.GetPosition());
        Entity_B->Transform.SetPosition(InitialChildTransform.GetPosition());

        Entity_A->Transform.SetQuaternion(InitialParentTransform.GetQuaternion());
        Entity_B->Transform.SetQuaternion(InitialChildTransform.GetQuaternion());

        Entity_A->Transform.SetScale(InitialParentTransform.GetScale());
        Entity_B->Transform.SetScale(InitialChildTransform.GetScale());

        // Temporary using old style entities.
        std::string Node_A_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_A->GetObjectID())->GetObjectID();
        std::string Node_B_ID = SCENE.SceneGraph.GetNodeByOldEntityID(Entity_B->GetObjectID())->GetObjectID();

        SCENE.SceneGraph.MoveNode(Node_B_ID, Node_A_ID);
        SCENE.SceneGraph.MoveNode(Node_B_ID, SCENE.SceneGraph.GetRoot()->GetObjectID());

        ASSERT_TRUE(ValidateTransformConsistency(Entity_B->Transform));

        glm::vec3 ExpectedPosition = InitialChildTransform.GetPosition();
        glm::vec3 ActualPosition = Entity_B->Transform.GetPosition();
        ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualPosition, ExpectedPosition));

        glm::quat ExpectedRotation = InitialChildTransform.GetQuaternion();
        glm::quat ActualRotation = Entity_B->Transform.GetQuaternion();
        // Temporary code to check with UE5 values.
        glm::vec3 ExpectedEuler = glm::degrees(glm::eulerAngles(ExpectedRotation));
        ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualRotation, ExpectedRotation));

        glm::vec3 ExpectedScale = InitialChildTransform.GetScale();
        glm::vec3 ActualScale = Entity_B->Transform.GetScale();
        ASSERT_TRUE(GEOMETRY.IsEpsilonEqual(ActualScale, ExpectedScale));
    }
};