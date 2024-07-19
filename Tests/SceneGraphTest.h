#pragma once

#include "../FEngine.h"
#include "gtest/gtest.h"
using namespace FocalEngine;

class SceneGraphTest : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    std::vector<FENaiveSceneGraphNode*> PopulateSceneGraph(FEScene* SceneToWorkWith, size_t NodeCount);
    std::vector<FENaiveSceneGraphNode*> PopulateSceneGraphTinySize(FEScene* SceneToWorkWith);
    std::vector<FENaiveSceneGraphNode*> PopulateSceneGraphSmallSize(FEScene* SceneToWorkWith);
    std::vector<FENaiveSceneGraphNode*> PopulateSceneGraphMediumSize(FEScene* SceneToWorkWith);

    bool ValidateTransformConsistency(const FETransformComponent& Transform);

    void TestTransformationComponentAfterChildAdded(FEScene* SceneToWorkWith, const std::string& ComponentType, const glm::vec3& InitialParentTransform, const glm::vec3& InitialChildTransform);
    void TestTransformationAfterChildAdded(FEScene* SceneToWorkWith, const FETransformComponent& InitialParentTransform, const FETransformComponent& InitialChildTransform);

    void TestTransformationComponentAfterChildChangedParent(FEScene* SceneToWorkWith, const std::string& ComponentType, const glm::vec3& InitialParentTransform, const glm::vec3& InitialChildTransform);
    void TestTransformationAfterChildChangedParent(FEScene* SceneToWorkWith, const FETransformComponent& InitialParentTransform, const FETransformComponent& InitialChildTransform);
};