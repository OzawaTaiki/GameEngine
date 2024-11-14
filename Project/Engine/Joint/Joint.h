#pragma once

#include "QuaternionTransform.h"
#include "Matrix4x4.h"
#include <string>
#include <vector>
#include <optional>

class Node;
class Joint
{
public:

    Joint() = default;
    ~Joint() = default;

    void Initialize();
    void Update(std::vector<Joint>& _joints);
    void Draw(const Matrix4x4& _wMat, std::vector <Joint>& _joints);
    static int32_t CreateJoint(const Node& _node, const std::optional<int32_t>& _parent, std::vector<Joint>& _joints);

    void SetTransform(const QuaternionTransform& _transform) { transform_ = _transform; }
    Matrix4x4 GetSkeletonSpaceMatrix() const { return SkeletonSpcaceMatrix_; }

    std::string name_ = {};
    int32_t index_ = 0;
private:
    QuaternionTransform transform_ = {};
    Matrix4x4 localMatrix_ = {};
    Matrix4x4 SkeletonSpcaceMatrix_ = {};
    std::vector<int32_t> children_ = {};
    std::optional<int32_t> parentIndex_ = {};


};