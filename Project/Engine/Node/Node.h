#pragma once

#include "Matrix4x4.h"
#include "QuaternionTransform.h"
#include <string>
#include <vector>

struct aiNode;
class Node
{
public:

    Node() = default;
    ~Node() = default;

    void ReadNode(const aiNode* _node);

    Matrix4x4 GetLocalMatrix() const { return localMatrix_; }

    std::string name_ = {};
    QuaternionTransform transform_ = {};
    std::vector<Node> children_ = {};
private:
    Matrix4x4 localMatrix_;

};