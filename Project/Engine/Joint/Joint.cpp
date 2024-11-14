#include "Joint.h"

#include "../Node/Node.h"
#include "MatrixFunction.h"
#include "LineDrawer.h"
#include "VectorFunction.h"

void Joint::Initialize()
{
}

void Joint::Update(std::vector<Joint>& _joints)
{
    localMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotation, transform_.translate);
    if (parentIndex_)
    {
        SkeletonSpcaceMatrix_ = localMatrix_ * _joints[*parentIndex_].SkeletonSpcaceMatrix_;
    }
    else
    {
        SkeletonSpcaceMatrix_ = localMatrix_;
    }
}


void Joint::Draw(const Matrix4x4& _wMat, std::vector <Joint>& _joints)
{
    Matrix4x4 wMat = SkeletonSpcaceMatrix_ * _wMat;

    LineDrawer::GetInstance()->DrawOBB(wMat);

    Vector3 pos = Transform({ 0,0,0 }, wMat);
    for (int32_t childIndex : children_)
    {
        Vector3 childPos = Transform({ 0,0,0 }, _joints[childIndex].SkeletonSpcaceMatrix_ * _wMat);
        LineDrawer::GetInstance()->RegisterPoint(pos, childPos);
    }
}

int32_t Joint::CreateJoint(const Node& _node, const std::optional<int32_t>& _parent, std::vector<Joint>& _joints)
{
    Joint joint = {};
    joint.name_ = _node.name_;
    joint.localMatrix_ = _node.GetLocalMatrix();
    joint.SkeletonSpcaceMatrix_ = MakeIdentity4x4();
    joint.transform_ = _node.transform_;
    joint.index_ = static_cast<int32_t>(_joints.size());
    joint.parentIndex_ = _parent;

    _joints.push_back(joint);
    for (const Node& child : _node.children_)
    {
        int32_t childIndex = CreateJoint(child, joint.index_, _joints);
        _joints[joint.index_].children_.push_back(childIndex);
    }


    return joint.index_;
}
