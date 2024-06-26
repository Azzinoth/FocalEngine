#pragma once

#include "../FEngine.h"
#include "gtest/gtest.h"
using namespace FocalEngine;

class SceneGraphTest : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    std::vector<FENaiveSceneGraphNode*> PopulateSceneGraph(size_t NodeCount);
    std::vector<FENaiveSceneGraphNode*> PopulateSceneGraphTinySize();
    std::vector<FENaiveSceneGraphNode*> PopulateSceneGraphSmallSize();
    std::vector<FENaiveSceneGraphNode*> PopulateSceneGraphMediumSize();

    bool ValidateTransformConsistency(const FETransformComponent& Transform);

    void TestTransformationComponentAfterChildAdded(const std::string& ComponentType, const glm::vec3& InitialParentTransform, const glm::vec3& InitialChildTransform);
    void TestTransformationAfterChildAdded(const FETransformComponent& InitialParentTransform, const FETransformComponent& InitialChildTransform);

    void TestTransformationComponentAfterChildChangedParent(const std::string& ComponentType, const glm::vec3& InitialParentTransform, const glm::vec3& InitialChildTransform);
    void TestTransformationAfterChildChangedParent(const FETransformComponent& InitialParentTransform, const FETransformComponent& InitialChildTransform);
};